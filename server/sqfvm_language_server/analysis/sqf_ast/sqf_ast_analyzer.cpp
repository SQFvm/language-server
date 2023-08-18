#include "sqf_ast_analyzer.hpp"
#include "ast_visitor.hpp"
#include "../../util.hpp"
#include "visitors/variables_visitor.hpp"
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

void sqfvm::language_server::analysis::sqf_ast::sqf_ast_analyzer::analyze() {
    auto path_info = m_runtime->fileio().get_info(m_file.path, {m_file.path, {}, {}});
    if (!path_info.has_value()) {
        m_diagnostics.push_back({
                                        .file_fk = m_file.id_pk,
                                        .severity = database::tables::t_diagnostic::severity_level::error,
                                        .message = "Failed to get path info for file: " + m_file.path,
                                });
        return;
    }
    analyze_ast(*m_runtime);
}

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

void sqfvm::language_server::analysis::sqf_ast::sqf_ast_analyzer::analyze_ast(
        sqf::runtime::runtime &runtime) {
    for (auto &visitor: m_visitors) {
        visitor->start(*this);
    }
    auto preprocessed_opt = runtime.parser_preprocessor().preprocess(runtime, m_text, {m_file.path, {}, {}});
    if (!preprocessed_opt.has_value()) {
        return;
    }
    m_preprocessed_text = preprocessed_opt.value();
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
        std::filesystem::path ls_path,
        const std::filesystem::path &db_path,
        sqfvm_factory &factory,
        sqfvm::language_server::database::tables::t_file file,
        std::string &text)
        : m_file(std::move(file)),
          m_text(text),
          m_preprocessed_text(text),
          m_runtime(),
          m_context(db_path),
          m_ls_path(std::move(ls_path)),
          m_slspp_context(std::make_shared<slspp_context>()) {
    m_runtime = factory.create([&](auto &msg) {
        auto copy = msg;
        copy.source_file_fk = m_file.id_pk;
        m_diagnostics.push_back(copy);
    }, m_context, m_slspp_context);
    m_visitors.push_back(new visitors::variables_visitor());
    m_visitors.push_back(new visitors::scripted_visitor());
}

sqfvm::language_server::analysis::sqf_ast::sqf_ast_analyzer::~sqf_ast_analyzer() {
    for (auto v: m_visitors) {
        delete v;
    }
}