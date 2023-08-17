#ifndef SQFVM_LANGUAGE_SERVER_ANALYSIS_SQF_AST_SQF_AST_ANALYZER_HPP
#define SQFVM_LANGUAGE_SERVER_ANALYSIS_SQF_AST_SQF_AST_ANALYZER_HPP

#include "../slspp_context.hpp"
#include "../analyzer.hpp"
#include "../../sqfvm_factory.hpp"
#include <string_view>
#include <utility>
#include <parser/sqf/astnode.hpp>

namespace sqfvm::language_server::analysis::sqf_ast {
    class ast_visitor;
    class sqf_ast_analyzer : public analyzer {
        friend class ast_visitor;
        database::tables::t_file m_file;
        std::string m_text;
        std::string m_preprocessed_text;
        std::vector<database::tables::t_diagnostic> m_diagnostics;
        std::vector<ast_visitor*> m_visitors;
        std::vector<const ::sqf::parser::sqf::bison::astnode *> m_descend_ast_nodes;
        std::shared_ptr<sqf::runtime::runtime> m_runtime;
        std::shared_ptr<slspp_context> m_slspp_context;
        database::context m_context;

        void recurse(const sqf::parser::sqf::bison::astnode &parent);
        void analyze_ast(sqf::runtime::runtime &runtime);
        [[nodiscard]] std::string scope_name() const {
            std::string scope{};
            auto id = std::to_string(m_file.id_pk);
            scope.reserve("scope@"sv.length() + id.length() + "://"sv.length());
            scope.append("scope@"sv);
            scope.append(id);
            scope.append("://"sv);
            return scope;
        }
        void insert(
                database::context::storage_t &storage,
                const database::tables::t_reference& copy) const;

    public:
        sqf_ast_analyzer(
                const std::filesystem::path& db_path,
                sqfvm_factory &factory,
                database::tables::t_file file,
                std::string &text);
        ~sqf_ast_analyzer() override;

        // Perform an abstract analysis of the document, gathering references of variables, functions, etc.
        // to be committed to the database in the next step.
        void analyze() override;

        // Commit the analysis to the database.
        void commit() override;
    };
}


#endif //SQFVM_LANGUAGE_SERVER_ANALYSIS_SQF_AST_SQF_AST_ANALYZER_HPP
