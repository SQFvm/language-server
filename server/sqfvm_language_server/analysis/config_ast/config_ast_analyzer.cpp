//
// Created by marco.silipo on 17.08.2023.
//

#include "config_ast_analyzer.hpp"

#include "ast_visitor.hpp"
#include "visitors/general_visitor.hpp"
#include <utility>
#include <unordered_map>
#include <fileio/default.h>
#include <parser/preprocessor/default.h>
#include <parser/config/config_parser.hpp>
#include "parser/config/parser.tab.hh"


namespace {


    // from boost (functional/hash):
    // see http://www.boost.org/doc/libs/1_35_0/doc/html/hash/combine.html template
    template<class T>
    inline void hash_combine(std::size_t &seed, T const &v) {
        seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
}
namespace std {
    template<>
    struct hash<sqfvm::language_server::analysis::config_ast::config_ast_analyzer::visitor_id_pair> {
        size_t operator()(sqfvm::language_server::analysis::config_ast::config_ast_analyzer::visitor_id_pair const &v) const {
            size_t seed = 0;
            hash_combine(seed, v.visitor_index);
            hash_combine(seed, v.id);
            return seed;
        }
    };
}
using namespace sqlite_orm;

void sqfvm::language_server::analysis::config_ast::config_ast_analyzer::commit() {
    std::unordered_map<visitor_id_pair, uint64_t> variable_map{};

    auto &storage = m_context.storage();
#if !defined(_DEBUG)
    storage.begin_transaction();
#endif

    try {

        // Call analyze on all visitors
        for (auto &visitor: m_visitors) {
            visitor->analyze(*this, m_context);
        }

#pragma region Code Actions
        // Remove old code actions
        for (auto &it: storage.get_all<database::tables::t_code_action>(
                where(c(&database::tables::t_code_action::file_fk) == m_file.id_pk))) {
            storage.remove_all<database::tables::t_code_action_change>(
                    where(c(&database::tables::t_code_action_change::code_action_fk) == it.id_pk));
        }
        storage.remove_all<database::tables::t_code_action>(
                where(c(&database::tables::t_code_action::file_fk) == m_file.id_pk));

        // Add new code actions
        for (auto &visitor: m_visitors) {
            for (auto &it: visitor->m_code_actions) {
                if (it.code_action.file_fk == 0)
                    it.code_action.file_fk = m_file.id_pk;
                auto code_action_id = storage.insert(it.code_action);
                for (auto &change: it.changes) {
                    change.code_action_fk = code_action_id;
                }
                storage.insert_range(it.changes.begin(), it.changes.end());
            }
        }
#pragma endregion
#pragma region Hovers
        if (!m_preprocessed_text.empty()) {
            // Remove old hovers
            storage.remove_all<database::tables::t_hover>(
                    where(c(&database::tables::t_hover::file_fk) == m_file.id_pk));

            // Add new hovers
            std::vector<database::tables::t_hover> hovers;
            std::stringstream hover_text;
            for (auto &it: m_hover_tuples) {
                if (std::filesystem::path(it.path) != m_file.path)
                    continue;
                hover_text << "`";
                hover_text << m_text.substr(it.raw_start, it.raw_end - it.raw_start);
                hover_text << "`\n";
                hover_text << "```sqf\n";
                hover_text << m_preprocessed_text.substr(it.pp_start, it.pp_end - it.pp_start);
                hover_text << "\n```\n";

                hovers.emplace_back(
                        0,
                        m_file.id_pk,
                        it.start_line,
                        it.start_column,
                        it.end_line,
                        it.end_column,
                        hover_text.str());
                hover_text.str("");
            }
            storage.insert_range(hovers.begin(), hovers.end());
            for (auto &visitor: m_visitors) {
                for (auto &it: visitor->m_hovers) {
                    if (it.file_fk == 0)
                        it.file_fk = m_file.id_pk;
                }
                storage.insert_range(visitor->m_hovers.begin(), visitor->m_hovers.end());
            }
        }
#pragma endregion
#pragma region Includes
        if (!m_preprocessed_text.empty()) {
            // Remove old includes
            storage.remove_all<database::tables::t_file_include>(
                    where(c(&database::tables::t_file_include::source_file_fk) == m_file.id_pk));

            // Add new includes
            std::vector<database::tables::t_file_include> file_includes;
            for (auto &it: m_file_include) {
                auto included_path_file = m_context.db_get_file_from_path(it.included_path);
                if (!included_path_file.has_value())
                    continue;
                auto source_path_file = m_context.db_get_file_from_path(it.source_path);
                if (!source_path_file.has_value())
                    continue;
                file_includes.emplace_back(
                        0,
                        included_path_file->id_pk,
                        source_path_file->id_pk,
                        m_file.id_pk);
            }
            storage.insert_range(file_includes.begin(), file_includes.end());
        }
#pragma endregion
#pragma region Diagnostics
        // Remove old diagnostics
        storage.remove_all<database::tables::t_diagnostic>(
                where(c(&database::tables::t_diagnostic::source_file_fk) == m_file.id_pk));

        // Add new diagnostics
        for (auto &it: m_diagnostics) {
            if (it.file_fk == 0)
                it.file_fk = m_file.id_pk;
            if (it.source_file_fk == 0)
                it.source_file_fk = m_file.id_pk;
            std::string path = it.file_fk == m_file.id_pk
                               ? m_file.path
                               : m_context.storage().get<database::tables::t_file>(it.file_fk).path;
            it.is_suppressed = !m_slspp_context->can_report(it.code, path, it.line);
        }
        storage.insert_range(m_diagnostics.begin(), m_diagnostics.end());
        for (auto &visitor: m_visitors) {
            for (auto &it: visitor->m_diagnostics) {
                if (it.file_fk == 0)
                    it.file_fk = m_file.id_pk;
                if (it.source_file_fk == 0)
                    it.source_file_fk = m_file.id_pk;
                std::string path = it.file_fk == m_file.id_pk
                                   ? m_file.path
                                   : m_context.storage().get<database::tables::t_file>(it.file_fk).path;
                it.is_suppressed = !m_slspp_context->can_report(it.code, path, it.line);
            }
            storage.insert_range(visitor->m_diagnostics.begin(), visitor->m_diagnostics.end());
        }
#pragma endregion

        // Remove outdated flag
        m_file.is_outdated = false;
        storage.update(m_file);

#if !defined(_DEBUG)
        storage.commit();
#endif
    }
    catch (std::exception &e) {
#if !defined(_DEBUG)
        storage.rollback();
#endif
        throw e;
    }


}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "misc-no-recursion"

void sqfvm::language_server::analysis::config_ast::config_ast_analyzer::recurse(
        const sqf::parser::config::bison::astnode &parent) {
    for (auto &visitor: m_visitors) {
        visitor->enter(*this, parent, m_descend_ast_nodes);
    }
    m_descend_ast_nodes.push_back(&parent);
    for (auto &child: parent.children) {
        recurse(child);
    }
    m_descend_ast_nodes.pop_back();
    for (auto &visitor: m_visitors) {
        visitor->exit(*this, parent, m_descend_ast_nodes);
    }
}

#pragma clang diagnostic pop

void sqfvm::language_server::analysis::config_ast::config_ast_analyzer::analyze(sqf::runtime::runtime &runtime) {
    for (auto &visitor: m_visitors) {
        visitor->start(*this);
    }
    auto parser = sqf::parser::config::parser(runtime.get_logger());
    auto tokenizer = sqf::parser::config::tokenizer(m_preprocessed_text.begin(), m_preprocessed_text.end(), m_file.path);
    // sqf::parser::config::bison::astnode root;
    // auto success = parser.get_tree(runtime, tokenizer, &root);
    // if (!success) {
    //     return;
    // }
    // recurse(root);
    parser.parse(runtime.confighost(), m_preprocessed_text, {m_file.path, {}, {}});
    for (auto &visitor: m_visitors) {
        visitor->end(*this);
    }
}

sqfvm::language_server::analysis::config_ast::config_ast_analyzer::config_ast_analyzer(
        const std::filesystem::path &db_path,
        database::tables::t_file file,
        sqfvm_factory &factory,
        std::string text,
        std::filesystem::path ls_path)
        : sqfvm_analyzer(db_path, std::move(file), factory, std::move(text)),
          m_ls_path(std::move(ls_path)) {
    m_visitors.push_back(new visitors::general_visitor());
}

sqfvm::language_server::analysis::config_ast::config_ast_analyzer::~config_ast_analyzer() {
    for (auto v: m_visitors) {
        delete v;
    }
}

void sqfvm::language_server::analysis::config_ast::config_ast_analyzer::macro_resolved(
        ::sqf::parser::preprocessor::impl_default::macro_resolved_data orig_start,
        ::sqf::parser::preprocessor::impl_default::macro_resolved_data orig_end,
        size_t pp_start,
        size_t pp_end,
        sqf::runtime::runtime &runtime,
        sqf::runtime::parser::preprocessor::context &local_fileinfo,
        sqf::runtime::parser::preprocessor::context &original_fileinfo,
        const sqf::runtime::parser::macro &m,
        const std::unordered_map<std::string, std::string> &param_map) {
    m_hover_tuples.emplace_back(
            original_fileinfo.path.physical,
            orig_start.offset,
            orig_end.offset,
            pp_start,
            pp_end,
            orig_start.line,
            orig_start.column,
            orig_end.line,
            orig_end.column);
}

void sqfvm::language_server::analysis::config_ast::config_ast_analyzer::file_included(
        sqf::runtime::parser::preprocessor::context &included_fileinfo,
        sqf::runtime::parser::preprocessor::context &source_fileinfo) {
    m_file_include.emplace_back(included_fileinfo.path.physical, source_fileinfo.path.physical);
}
