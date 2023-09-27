#include "general_visitor.hpp"

using namespace sqfvm::language_server::database::tables;
using namespace std::string_view_literals;

void sqfvm::language_server::analysis::config_ast::visitors::general_visitor::start(config_ast_analyzer &a) {
}

void sqfvm::language_server::analysis::config_ast::visitors::general_visitor::enter(
        config_ast_analyzer &a,
        const ::sqf::parser::config::bison::astnode &node,
        const std::vector<const ::sqf::parser::config::bison::astnode *> &parent_nodes) {
}
void sqfvm::language_server::analysis::config_ast::visitors::general_visitor::exit(
        config_ast_analyzer &a,
        const ::sqf::parser::config::bison::astnode &node,
        const std::vector<const ::sqf::parser::config::bison::astnode *> &parent_nodes) {
}

void sqfvm::language_server::analysis::config_ast::visitors::general_visitor::end(config_ast_analyzer &a) {
}

void sqfvm::language_server::analysis::config_ast::visitors::general_visitor::analyze(
        sqfvm::language_server::analysis::config_ast::config_ast_analyzer &config_ast_analyzer,
        const sqfvm::language_server::database::context &context) {
}
