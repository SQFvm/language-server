#ifndef SQFVM_LANGUAGE_SERVER_ANALYSIS_config_ast_VISITORS_VARIABLES_VISITOR_HPP
#define SQFVM_LANGUAGE_SERVER_ANALYSIS_config_ast_VISITORS_VARIABLES_VISITOR_HPP

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

namespace sqfvm::language_server::analysis::config_ast::visitors {
    class general_visitor : public ast_visitor {
    public:
        ~general_visitor() override = default;

        void start(config_ast_analyzer &a) override;
        void enter(
                config_ast_analyzer &a,
                const ::sqf::parser::config::bison::astnode &node,
                const std::vector<const ::sqf::parser::config::bison::astnode *> &parent_nodes
        ) override;

        void exit(
                config_ast_analyzer &a,
                const ::sqf::parser::config::bison::astnode &node,
                const std::vector<const ::sqf::parser::config::bison::astnode *> &parent_nodes) override;

        void end(
                config_ast_analyzer &a) override;

        void analyze(
                sqfvm::language_server::analysis::config_ast::config_ast_analyzer &config_ast_analyzer,
                const database::context &context) override;
    };
}
#endif // SQFVM_LANGUAGE_SERVER_ANALYSIS_config_ast_VISITORS_VARIABLES_VISITOR_HPP