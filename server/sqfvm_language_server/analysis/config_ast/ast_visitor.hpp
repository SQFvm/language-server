#ifndef SQFVM_LANGUAGE_SERVER_ANALYSIS_config_ast_AST_VISITOR_HPP
#define SQFVM_LANGUAGE_SERVER_ANALYSIS_config_ast_AST_VISITOR_HPP

#include "config_ast_analyzer.hpp"
#include "../analyzer.hpp"
#include <string>
#include <algorithm>
#include <filesystem>

namespace sqf::parser::config::bison
{
    struct astnode;
}
namespace sqfvm::language_server::analysis::config_ast {

    class ast_visitor {
    protected:
        struct code_action_tuple {
            database::tables::t_code_action code_action;
            std::vector<database::tables::t_code_action_change> changes;
        };
        std::vector<database::tables::t_reference> m_references;
        std::vector<database::tables::t_variable> m_variables;
        std::vector<database::tables::t_diagnostic> m_diagnostics;
        std::vector<database::tables::t_hover> m_hovers;
        std::vector<code_action_tuple> m_code_actions;

        friend class config_ast_analyzer;

        std::filesystem::path ls_folder_of(config_ast_analyzer &a) const {
            return a.m_ls_path;
        }

        auto is_offset_in_macro(config_ast_analyzer &a, size_t offset) const {
            return a.is_offset_in_macro(offset);
        }
        auto decode_preprocessed_offset(config_ast_analyzer &a, size_t offset) const {
            return a.decode_preprocessed_offset(offset);
        }

        std::shared_ptr<sqf::runtime::runtime> runtime_of(config_ast_analyzer &a) {
            return a.m_runtime;
        }

        const database::tables::t_file &file_of(config_ast_analyzer &a) const {
            return a.m_file;
        }

        const database::context &context_of(config_ast_analyzer &a) const {
            return a.m_context;
        }

        database::context &context_of(config_ast_analyzer &a) {
            return a.m_context;
        }

        const std::string_view text_of(config_ast_analyzer &a) const {
            return a.m_preprocessed_text;
        }

        std::string scope_name_of(config_ast_analyzer &a) const {
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

        virtual void start(config_ast_analyzer &a) = 0;

        virtual void enter(
                config_ast_analyzer &a,
                const sqf::parser::config::bison::astnode &node,
                const std::vector<const ::sqf::parser::config::bison::astnode *> &parent_nodes) = 0;

        virtual void exit(
                config_ast_analyzer &a,
                const sqf::parser::config::bison::astnode &node,
                const std::vector<const ::sqf::parser::config::bison::astnode *> &parent_nodes) = 0;

        virtual void end(config_ast_analyzer &a) = 0;

        virtual void analyze(
                sqfvm::language_server::analysis::config_ast::config_ast_analyzer &config_ast_analyzer,
                const database::context &context) {}
    };
}
#endif // SQFVM_LANGUAGE_SERVER_ANALYSIS_config_ast_AST_VISITOR_HPP