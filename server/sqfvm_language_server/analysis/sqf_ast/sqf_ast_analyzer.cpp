#include "sqf_ast_analyzer.hpp"
#include "ast_visitor.hpp"
#include "../../util.hpp"
#include "visitors/variables_visitor.hpp"
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
    auto path_info = m_runtime->fileio().get_info(m_file.path, {});
    if (!path_info.has_value()) {
        m_diagnostics.push_back({
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
            visitor->analyze(m_context);
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
                                .visitor_index = static_cast<size_t>(visitor_diff),
                                .id = visitor_variable.id_pk
                        }] = visitor_variable.id_pk;
                        file_variables_mapped.push_back(visitor_variable);
                    } else {
                        auto copy = visitor_variable;
                        copy.id_pk = 0;
                        // Variable does not exist
                        variable_map[visitor_id_pair{
                                .visitor_index = static_cast<size_t>(visitor_diff),
                                .id = visitor_variable.id_pk
                        }] = storage.insert(copy);
                    }
                } else {
                    // This variable is not in this file
                    auto db_variable = storage.get_optional<database::tables::t_variable>(
                            where(c(&database::tables::t_variable::scope) == visitor_variable.scope
                                  && c(&database::tables::t_variable::name) == visitor_variable.name));
                    if (db_variable.has_value()) {
                        // Variable already exists
                        variable_map[visitor_id_pair{
                                .visitor_index = static_cast<size_t>(visitor_diff),
                                .id = visitor_variable.id_pk
                        }] = db_variable->id_pk;
                        file_variables_mapped.push_back(*db_variable);
                    } else {
                        // Variable does not exist
                        auto copy = visitor_variable;
                        copy.id_pk = 0;
                        variable_map[visitor_id_pair{
                                .visitor_index = static_cast<size_t>(visitor_diff),
                                .id = visitor_variable.id_pk
                        }] = storage.insert(copy);
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
                storage.remove<database::tables::t_variable>(db_variable.id_pk);
            }
        }
#pragma endregion

#pragma region References
        // Remove all references related to this file
        storage.remove_all<database::tables::t_reference>(
                where(c(&database::tables::t_reference::file_fk) == m_file.id_pk));

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
                storage.insert(copy);
            }
        }
#pragma endregion


#pragma region Diagnostics
        // Remove old diagnostics
        storage.remove_all<database::tables::t_diagnostic>(
                where(c(&database::tables::t_diagnostic::file_fk) == m_file.id_pk));

        // Add new diagnostics
        storage.insert_range(m_diagnostics.begin(), m_diagnostics.end());
        for (auto &visitor: m_visitors) {
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

#pragma clang diagnostic push
#pragma ide diagnostic ignored "misc-no-recursion"

void
sqfvm::language_server::analysis::sqf_ast::sqf_ast_analyzer::recurse(const sqf::parser::sqf::bison::astnode &parent) {
    m_descend_ast_nodes.push_back(&parent);
    for (auto &visitor: m_visitors) {
        visitor->enter(*this, parent, m_descend_ast_nodes);
    }
    for (auto &child: parent.children) {
        recurse(child);
    }
    for (auto &visitor: m_visitors) {
        visitor->exit(*this, parent, m_descend_ast_nodes);
    }
    m_descend_ast_nodes.pop_back();
}

#pragma clang diagnostic pop

void sqfvm::language_server::analysis::sqf_ast::sqf_ast_analyzer::analyze_ast(
        sqf::runtime::runtime &runtime) {
    for (auto &visitor: m_visitors) {
        visitor->start(*this);
    }
    auto logger = StdOutLogger();
    auto parser = sqf::parser::sqf::parser(logger);
    auto tokenizer = sqf::parser::sqf::tokenizer(m_text.begin(), m_text.end(), m_file.path);
    sqf::parser::sqf::bison::astnode root;
    parser.get_tree(runtime, tokenizer, &root);
    recurse(root);
    for (auto &visitor: m_visitors) {
        visitor->end(*this);
    }
}

sqfvm::language_server::analysis::sqf_ast::sqf_ast_analyzer::sqf_ast_analyzer(
        std::filesystem::path db_path,
        sqfvm_factory &factory,
        sqfvm::language_server::database::tables::t_file file,
        std::string &text)
        : m_file(std::move(file)),
          m_text(text),
          m_runtime(),
          m_context(db_path) {
    m_runtime = factory.create([&](auto &msg) { m_diagnostics.push_back(msg); }, m_context);
    m_visitors.push_back(new visitors::variables_visitor());
}

sqfvm::language_server::analysis::sqf_ast::sqf_ast_analyzer::~sqf_ast_analyzer() {
    for (auto v: m_visitors) {
        delete v;
    }
}

