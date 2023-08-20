//
// Created by marco.silipo on 17.08.2023.
//

#ifndef SQFVM_LANGUAGE_SERVER_SCRIPTED_VISITOR_HPP
#define SQFVM_LANGUAGE_SERVER_SCRIPTED_VISITOR_HPP


#include "../ast_visitor.hpp"

namespace sqfvm::language_server::analysis::sqf_ast::visitors {
    class scripted_visitor : public ast_visitor {
        bool m_is_disabled;
        std::optional<::sqf::runtime::instruction_set> m_start_script;
        std::optional<::sqf::runtime::instruction_set> m_enter_script;
        std::optional<::sqf::runtime::instruction_set> m_exit_script;
        std::optional<::sqf::runtime::instruction_set> m_end_script;
        std::optional<::sqf::runtime::instruction_set> m_analyze_script;


        void initialize_functions_and_documentation(
                sqfvm::language_server::analysis::sqf_ast::sqf_ast_analyzer &a,
                std::shared_ptr<sqf::runtime::runtime> runtime,
                std::filesystem::path file);

        void initialize_start_script(std::shared_ptr<sqf::runtime::runtime> runtime, std::filesystem::path file);

        void call(
                std::shared_ptr<sqf::runtime::runtime> runtime,
                std::optional<::sqf::runtime::instruction_set> &instruction_set,
                std::vector<sqf::runtime::value> this_values);

        void initialize_enter_script(std::shared_ptr<sqf::runtime::runtime> runtime, std::filesystem::path file);

        void initialize_exit_script(std::shared_ptr<sqf::runtime::runtime> runtime, std::filesystem::path file);

        void initialize_end_script(std::shared_ptr<sqf::runtime::runtime> runtime, std::filesystem::path file);

        void initialize_analyze_script(std::shared_ptr<sqf::runtime::runtime> runtime, std::filesystem::path file);

    public:
        ~scripted_visitor() override = default;

        void start(
                sqf_ast_analyzer &a
        ) override;


        void enter(
                sqf_ast_analyzer &a,
                const ::sqf::parser::sqf::bison::astnode &node,
                const std::vector<const ::sqf::parser::sqf::bison::astnode *> &parent_nodes
        ) override;

        void exit(
                sqf_ast_analyzer &a,
                const ::sqf::parser::sqf::bison::astnode &node,
                const std::vector<const ::sqf::parser::sqf::bison::astnode *> &parent_nodes
        ) override;

        void end(
                sqf_ast_analyzer &a
        ) override;

        void analyze(
                sqfvm::language_server::analysis::sqf_ast::sqf_ast_analyzer &sqf_ast_analyzer,
                const database::context &context
        ) override;
    };
}


#endif //SQFVM_LANGUAGE_SERVER_SCRIPTED_VISITOR_HPP
