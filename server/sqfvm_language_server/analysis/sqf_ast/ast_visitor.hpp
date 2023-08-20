#ifndef SQFVM_LANGUAGE_SERVER_ANALYSIS_SQF_AST_AST_VISITOR_HPP
#define SQFVM_LANGUAGE_SERVER_ANALYSIS_SQF_AST_AST_VISITOR_HPP

#include "sqf_ast_analyzer.hpp"
#include "../analyzer.hpp"
#include <string>
#include <algorithm>
#include <filesystem>
#include "parser/sqf/tokenizer.hpp"
#include "parser/sqf/astnode.hpp"

namespace sqfvm::language_server::analysis::sqf_ast {

    class ast_visitor {
    protected:
        std::vector<database::tables::t_reference> m_references;
        std::vector<database::tables::t_variable> m_variables;
        std::vector<database::tables::t_diagnostic> m_diagnostics;

        friend class sqf_ast_analyzer;

        std::filesystem::path ls_folder_of(sqf_ast_analyzer &a) const {
            return a.m_ls_path;
        }

        std::shared_ptr<sqf::runtime::runtime> runtime_of(sqf_ast_analyzer &a) {
            return a.m_runtime;
        }

        const database::tables::t_file &file_of(sqf_ast_analyzer &a) const {
            return a.m_file;
        }

        const database::context &context_of(sqf_ast_analyzer &a) const {
            return a.m_context;
        }

        database::context &context_of(sqf_ast_analyzer &a) {
            return a.m_context;
        }

        const std::string_view text_of(sqf_ast_analyzer &a) const {
            return a.m_preprocessed_text;
        }

        std::string scope_name_of(sqf_ast_analyzer &a) const {
            return a.scope_name();
        }

        [[nodiscard]] bool is_private_variable(std::string_view name) const {
            return name.empty() ? false : name[0] == '_';
        }

        [[nodiscard]] bool is_global_variable(std::string_view name) const {
            return name.empty() ? true : name[0] != '_';
        }

        [[nodiscard]] bool is_private_variable(const database::tables::t_variable &variable) const {
            return is_private_variable(variable.variable_name);
        }

        [[nodiscard]] bool is_global_variable(const database::tables::t_variable &variable) const {
            return is_global_variable(variable.variable_name);
        }


    public:
        virtual ~ast_visitor() = default;

        virtual void start(sqf_ast_analyzer &a) = 0;

        virtual void enter(
                sqf_ast_analyzer &a,
                const sqf::parser::sqf::bison::astnode &node,
                const std::vector<const ::sqf::parser::sqf::bison::astnode *> &parent_nodes) = 0;

        virtual void exit(
                sqf_ast_analyzer &a,
                const sqf::parser::sqf::bison::astnode &node,
                const std::vector<const ::sqf::parser::sqf::bison::astnode *> &parent_nodes) = 0;

        virtual void end(sqf_ast_analyzer &a) = 0;

        virtual void analyze(
                sqfvm::language_server::analysis::sqf_ast::sqf_ast_analyzer &sqf_ast_analyzer,
                const database::context &context) {}
    };
}
#endif // SQFVM_LANGUAGE_SERVER_ANALYSIS_SQF_AST_AST_VISITOR_HPP