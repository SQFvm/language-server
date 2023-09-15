#ifndef SQFVM_LANGUAGE_SERVER_ANALYSIS_SQF_AST_SQF_AST_ANALYZER_HPP
#define SQFVM_LANGUAGE_SERVER_ANALYSIS_SQF_AST_SQF_AST_ANALYZER_HPP

#include "../slspp_context.hpp"
#include "../sqfvm_analyzer.hpp"
#include "../../sqfvm_factory.hpp"
#include <string_view>
#include <utility>
#include <parser/sqf/astnode.hpp>

namespace sqfvm::language_server::analysis::sqf_ast {
    class ast_visitor;

    class sqf_ast_analyzer : public sqfvm_analyzer {
        friend class ast_visitor;

        struct hover_tuple {
            std::string path;
            size_t raw_start;
            size_t raw_end;
            size_t pp_start;
            size_t pp_end;
            uint64_t start_line;
            uint64_t start_column;
            uint64_t end_line;
            uint64_t end_column;
        };
        std::vector<database::tables::t_diagnostic> m_diagnostics;
        std::vector<hover_tuple> m_hover_tuples;
        std::vector<ast_visitor *> m_visitors;
        std::vector<const ::sqf::parser::sqf::bison::astnode *> m_descend_ast_nodes;
        std::filesystem::path m_ls_path;

        void recurse(const sqf::parser::sqf::bison::astnode &parent);

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
                const database::tables::t_reference &copy) const;

    protected:
        void report_diagnostic(const database::tables::t_diagnostic &diagnostic) override {
            m_diagnostics.push_back(diagnostic);
        }

        void macro_resolved(
                ::sqf::parser::preprocessor::impl_default::macro_resolved_data orig_start,
                ::sqf::parser::preprocessor::impl_default::macro_resolved_data orig_end, size_t pp_start,
                size_t pp_end, ::sqf::runtime::runtime &runtime,
                ::sqf::runtime::parser::preprocessor::context &local_fileinfo,
                ::sqf::runtime::parser::preprocessor::context &original_fileinfo,
                const ::sqf::runtime::parser::macro &m,
                const std::unordered_map<std::string, std::string> &param_map) override;

    public:
        sqf_ast_analyzer(
                const std::filesystem::path &db_path,
                database::tables::t_file file,
                sqfvm_factory &factory,
                std::string text,
                std::filesystem::path ls_path);

        ~sqf_ast_analyzer() override;

        // Perform an abstract analysis of the document, gathering references of variables, functions, etc.
        // to be committed to the database in the next step.
        void analyze(sqf::runtime::runtime &runtime) override;

        // Commit the analysis to the database.
        void commit() override;
    };
}


#endif //SQFVM_LANGUAGE_SERVER_ANALYSIS_SQF_AST_SQF_AST_ANALYZER_HPP
