#include "sqf_ast_analyzer.hpp"
#include "ast_visitor.hpp"
#include "../../util.hpp"
#include "visitors/general_visitor.hpp"
#include "visitors/scripted_visitor.hpp"
#include "../../runtime_logger.hpp"
#include <functional>
#include <utility>
#include <unordered_map>
#include <operators/ops.h>
#include <fileio/default.h>
#include <parser/sqf/sqf_parser.hpp>
#include <parser/preprocessor/default.h>
#include <parser/config/config_parser.hpp>


namespace {

    struct visitor_id_pair {
        size_t visitor_index;
        uint64_t id;

        bool operator==(const visitor_id_pair &other) const {
            return visitor_index == other.visitor_index
                   && id == other.id;
        }
    };

    // from boost (functional/hash):
    // see http://www.boost.org/doc/libs/1_35_0/doc/html/hash/combine.html template
    template<class T>
    inline void hash_combine(std::size_t &seed, T const &v) {
        seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
}
namespace std {
    template<>
    struct hash<visitor_id_pair> {
        size_t operator()(visitor_id_pair const &v) const {
            size_t seed = 0;
            hash_combine(seed, v.visitor_index);
            hash_combine(seed, v.id);
            return seed;
        }
    };
}

void sqfvm::language_server::analysis::sqf_ast::sqf_ast_analyzer::commit() {
    using namespace sqlite_orm;
    std::unordered_map<visitor_id_pair, uint64_t> variable_map{};

    auto &storage = m_context.storage();
    storage.begin_transaction();

    try {

        // Call analyze on all visitors
        for (auto &visitor: m_visitors) {
            visitor->analyze(*this, m_context);
        }

#pragma region Variables
        // Get all variables related to this file
        auto file_scope_name = scope_name();
        auto file_scope_name_like_expression = file_scope_name + "%";
        std::vector<database::tables::t_variable> db_file_variables = storage.get_all<database::tables::t_variable>(
                where(like(&database::tables::t_variable::scope, file_scope_name_like_expression)));
        std::vector<database::tables::t_variable> file_variables_mapped{};


        // Map all variables to their visitor
        for (auto visitor_it = m_visitors.begin(); visitor_it != m_visitors.end(); ++visitor_it) {
            auto &visitor = *visitor_it;
            auto visitor_diff = visitor_it - m_visitors.begin();
            auto visitor_index = static_cast<size_t>(visitor_diff);
            for (auto &visitor_variable: visitor->m_variables) {
                if (visitor_variable.scope.length() > file_scope_name.length()
                    && std::string_view(visitor_variable.scope.begin(),
                                        visitor_variable.scope.begin() + file_scope_name.length()) ==
                       file_scope_name) {
                    // This variable is in this file
                    auto db_variable = std::find_if(db_file_variables.begin(), db_file_variables.end(),
                                                    [&](auto &db_variable) {
                                                        return db_variable.scope == visitor_variable.scope;
                                                    });
                    if (db_variable != db_file_variables.end()) {
                        variable_map[visitor_id_pair{
                                .visitor_index = visitor_index,
                                .id = visitor_variable.id_pk
                        }] = db_variable->id_pk;
                        file_variables_mapped.push_back(*db_variable);
                    } else {
                        auto copy = visitor_variable;
                        copy.id_pk = 0;
                        // Variable does not exist
                        auto insert_res = storage.insert(copy);
                        variable_map[visitor_id_pair{
                                .visitor_index = visitor_index,
                                .id = visitor_variable.id_pk
                        }] = insert_res;
                    }
                } else {
                    // This variable is not in this file
                    std::optional<database::tables::t_variable> db_variable = {};
                    auto result = storage.get_all<database::tables::t_variable>(
                            where((c(&database::tables::t_variable::scope) == visitor_variable.scope)
                                  and
                                  (c(&database::tables::t_variable::variable_name) == visitor_variable.variable_name)));
                    db_variable = result.empty() ? std::nullopt : std::optional(result[0]);
                    if (db_variable.has_value()) {
                        // Variable already exists
                        variable_map[visitor_id_pair{
                                .visitor_index = visitor_index,
                                .id = visitor_variable.id_pk
                        }] = db_variable->id_pk;
                        file_variables_mapped.push_back(*db_variable);
                    } else {
                        // Variable does not exist
                        auto copy = visitor_variable;
                        copy.id_pk = 0;
                        auto insert_res = storage.insert(copy);
                        variable_map[visitor_id_pair{
                                .visitor_index = visitor_index,
                                .id = visitor_variable.id_pk
                        }] = insert_res;
                    }
                }
            }
        }

        // Remove all variables that are not in the file anymore
        for (auto &db_variable: db_file_variables) {
            if (std::find_if(file_variables_mapped.begin(), file_variables_mapped.end(),
                             [&](auto &variable) {
                                 return variable.scope == db_variable.scope;
                             }) == file_variables_mapped.end()) {
                storage.remove_all<database::tables::t_reference>(
                        where(c(&database::tables::t_reference::variable_fk) == db_variable.id_pk));
                storage.remove<database::tables::t_variable>(db_variable.id_pk);
            }
        }
#pragma endregion
#pragma region References
        // Remove all references related to this file
        storage.remove_all<database::tables::t_reference>(
                where(c(&database::tables::t_reference::source_file_fk) == m_file.id_pk));

        // Add all references
        for (auto visitor_it = m_visitors.begin(); visitor_it != m_visitors.end(); ++visitor_it) {
            auto &visitor = *visitor_it;
            auto visitor_diff = visitor_it - m_visitors.begin();
            for (auto &visitor_reference: visitor->m_references) {
                auto copy = visitor_reference;
                copy.id_pk = 0;
                copy.variable_fk = variable_map[visitor_id_pair{
                        .visitor_index = static_cast<size_t>(visitor_diff),
                        .id = visitor_reference.variable_fk
                }];
                insert(storage, copy);
            }
        }
#pragma endregion
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
            it.is_suppressed = !m_slspp_context->can_report(it.code, it.line);
        }
        storage.insert_range(m_diagnostics.begin(), m_diagnostics.end());
        for (auto &visitor: m_visitors) {
            for (auto &it: visitor->m_diagnostics) {
                if (it.file_fk == 0)
                    it.file_fk = m_file.id_pk;
                if (it.source_file_fk == 0)
                    it.source_file_fk = m_file.id_pk;
                it.is_suppressed = !m_slspp_context->can_report(it.code, it.line);
            }
            storage.insert_range(visitor->m_diagnostics.begin(), visitor->m_diagnostics.end());
        }
#pragma endregion

        // Remove outdated flag
        m_file.is_outdated = false;
        storage.update(m_file);

        storage.commit();
    }
    catch (std::exception &e) {
        storage.rollback();
        throw e;
    }


}

void sqfvm::language_server::analysis::sqf_ast::sqf_ast_analyzer::insert(
        database::context::storage_t &storage,
        const database::tables::t_reference &copy) const {
    try {
        storage.insert(copy);
    } catch (std::exception &e) {
        std::stringstream sstream;
        sstream << "Failed to insert into " << database::tables::t_reference::table_name << "." << "\n";
        sstream << "What: " << "\n";
        sstream << "    " << e.what() << "\n";
        sstream << "Data: " << "\n";
        sstream << "    file_fk: " << copy.file_fk << "\n";
        sstream << "    variable_fk: " << copy.variable_fk << "\n";
        sstream << "    access: " << static_cast<int>(copy.access) << "\n";
        sstream << "    line: " << copy.line << "\n";
        sstream << "    column: " << copy.column << "\n";
        sstream << "    offset: " << copy.offset << "\n";
        sstream << "    length: " << copy.length << "\n";
        sstream << "    types: " << static_cast<int>(copy.types);

        throw std::runtime_error(sstream.str());
    }
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "misc-no-recursion"

void sqfvm::language_server::analysis::sqf_ast::sqf_ast_analyzer::recurse(
        const sqf::parser::sqf::bison::astnode &parent) {
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

void sqfvm::language_server::analysis::sqf_ast::sqf_ast_analyzer::analyze(sqf::runtime::runtime &runtime) {
    for (auto &visitor: m_visitors) {
        visitor->start(*this);
    }
    auto parser = sqf::parser::sqf::parser(runtime.get_logger());
    auto tokenizer = sqf::parser::sqf::tokenizer(m_preprocessed_text.begin(), m_preprocessed_text.end(), m_file.path);
    sqf::parser::sqf::bison::astnode root;
    auto success = parser.get_tree(runtime, tokenizer, &root);
    if (!success) {
        return;
    }
    recurse(root);
    for (auto &visitor: m_visitors) {
        visitor->end(*this);
    }
}

sqfvm::language_server::analysis::sqf_ast::sqf_ast_analyzer::sqf_ast_analyzer(
        const std::filesystem::path &db_path,
        database::tables::t_file file,
        sqfvm_factory &factory,
        std::string text,
        std::filesystem::path ls_path)
        : sqfvm_analyzer(db_path, std::move(file), factory, std::move(text)),
          m_ls_path(std::move(ls_path)) {
    m_visitors.push_back(new visitors::general_visitor());
    m_visitors.push_back(new visitors::scripted_visitor());
}

sqfvm::language_server::analysis::sqf_ast::sqf_ast_analyzer::~sqf_ast_analyzer() {
    for (auto v: m_visitors) {
        delete v;
    }
}

void sqfvm::language_server::analysis::sqf_ast::sqf_ast_analyzer::macro_resolved(
        ::sqf::parser::preprocessor::impl_default::macro_resolved_data orig_start,
        ::sqf::parser::preprocessor::impl_default::macro_resolved_data orig_end,
        size_t pp_start,
        size_t pp_end,
        sqf::runtime::runtime &runtime, sqf::runtime::parser::preprocessor::context &local_fileinfo,
        sqf::runtime::parser::preprocessor::context &original_fileinfo, const sqf::runtime::parser::macro &m,
        const std::unordered_map<std::string, std::string> &param_map) {
    m_hover_tuples.emplace_back(orig_start.offset, orig_end.offset, pp_start, pp_end, orig_start.line, orig_start.column, orig_end.line, orig_end.column);
}
