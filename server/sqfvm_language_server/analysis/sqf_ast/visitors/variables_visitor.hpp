#ifndef SQFVM_LANGUAGE_SERVER_ANALYSIS_SQF_AST_VISITORS_VARIABLES_VISITOR_HPP
#define SQFVM_LANGUAGE_SERVER_ANALYSIS_SQF_AST_VISITORS_VARIABLES_VISITOR_HPP

#include "../ast_visitor.hpp"
#include "../../../database/tables/t_reference.h"
#include "../../../database/tables/t_variable.h"
#include "../../../database/tables/t_variable.h"
#include <vector>
#include <string>
#include <string_view>
#include <numeric>
#include <optional>
#include <stack>

namespace sqfvm::language_server::analysis::sqf_ast::visitors {
    class variables_visitor : public ast_visitor {
        struct scope {
            size_t child_count;
            std::string full_name;
        };

        std::stack<std::string> m_namespace_stack;
        std::vector<scope> m_scope_stack;
        std::stack<database::tables::t_reference> m_method_temporary_stack;
        std::optional<database::tables::t_reference> m_assignment_candidate;

        [[nodiscard]] bool is_left_side_of_assignment(
                const std::vector<const ::sqf::parser::sqf::bison::astnode *> &parent_nodes,
                const ::sqf::parser::sqf::bison::astnode &node);

        [[nodiscard]] bool is_right_side_of_assignment(
                const std::vector<const ::sqf::parser::sqf::bison::astnode *> &parent_nodes,
                const ::sqf::parser::sqf::bison::astnode &node) const;

        [[nodiscard]] database::tables::t_reference make_reference(const ::sqf::parser::sqf::bison::astnode &node) const;
        [[nodiscard]] database::tables::t_variable get_or_create_variable(std::string_view name);
        std::string push_scope(const ::sqf::parser::sqf::bison::astnode &node,
                               const std::vector<const ::sqf::parser::sqf::bison::astnode *> &parent_nodes);
        void pop_scope();
        void push_namespace(std::string name) {
            m_namespace_stack.push(std::move(name));
        }
        void pop_namespace() {
            m_namespace_stack.pop();
        }
        [[nodiscard]] std::string get_namespace() const {
            if (m_namespace_stack.empty()) {
                return "missionNamespace";
            }
            return m_namespace_stack.top();
        }

    public:
        ~variables_visitor() override = default;

        void start(sqf_ast_analyzer &a) override;


        void enter(
                sqf_ast_analyzer &a,
                const ::sqf::parser::sqf::bison::astnode &node,
                const std::vector<const ::sqf::parser::sqf::bison::astnode *> &parent_nodes
        ) override;

        void exit(sqf_ast_analyzer &a, const ::sqf::parser::sqf::bison::astnode &node,
                  const std::vector<const ::sqf::parser::sqf::bison::astnode *> &parent_nodes) override;

        void end(sqf_ast_analyzer &a) override;
        void analyze(const database::context& context) override;
    };
}
#endif // SQFVM_LANGUAGE_SERVER_ANALYSIS_SQF_AST_VISITORS_VARIABLES_VISITOR_HPP