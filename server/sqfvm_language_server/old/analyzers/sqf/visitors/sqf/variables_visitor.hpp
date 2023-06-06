#pragma once

#include "../../analyzer_sqf.hpp"
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>
#include <optional>

namespace sqfvm::language_server::visitors::sqf {
    struct variables_visitor :
            public sqfvisitor {
        std::vector<std::string> scope_stack;
        std::optional<strmtch> assignment_temp_match;
        std::vector<std::optional<method>> method_temps;

        ~variables_visitor() override = default;

        void start(analyzer_sqf &a) override {
        }


        void enter(
                analyzer_sqf &a,
                const ::sqf::parser::sqf::bison::astnode &node,
                const std::vector<const ::sqf::parser::sqf::bison::astnode *> &parent_nodes
        ) override {
            switch (node.kind) {
                case ::sqf::parser::sqf::bison::astkind::CODE: {
                    // Check if we are on the right side of an ASSIGNMENT
                    if (is_right_side_of_assignment(parent_nodes,
                                                    node)) { // Yes - Change the temp of assignment to method_assigned and put to method_temps for later addition to methods used (with possible args)
                        method m = create_method();
                        method_temps.emplace_back(m);
                        assignment_temp_match = {};
                    } else {
                        method_temps.emplace_back();
                    }

                    // Push scope info
                    auto str_line = std::to_string(node.token.line);
                    auto str_column = std::to_string(node.token.column);
                    scope_stack.push_back(str_line + "-" + str_column);
                    break;
                }
                case ::sqf::parser::sqf::bison::astkind::ASSIGNMENT_LOCAL: {
                    strmtch match;
                    set_scope_name(match);
                    match.name = node.token.contents;
                    a.res.variables_set.push_back(match);
                    break;
                }
                case ::sqf::parser::sqf::bison::astkind::IDENT: {
                    strmtch match;
                    set_scope_name(match);
                    match.name = node.token.contents;
                    // Check if we are on the left side of an assignment
                    if (is_left_side_of_assignment(parent_nodes,
                                                   node)) { // Yes - We set this IDENT so put it into temp for later evaluation in parent or the code following
                        assignment_temp_match = match;
                    } else { // No - We get this IDENT
                        // Check if we are on the right side of a CALL
                        if (is_right_side_of_call(node, parent_nodes)) { // Yes - We set this IDENT
                            a.res.methods_used.push_back(match);
                        } else { // No - We get this IDENT
                            a.res.variables_used.push_back(match);
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        }

        [[nodiscard]] static bool is_right_side_of_call(const ::sqf::parser::sqf::bison::astnode &node,
                                                        const std::vector<const ::sqf::parser::sqf::bison::astnode *> &parent_nodes) {
            return parent_nodes.size() > 2
                   && (*(parent_nodes.rbegin() + 1))->kind == ::sqf::parser::sqf::bison::astkind::EXP4
                   && (*(parent_nodes.rbegin() + 1))->token.contents == "call"
                   && &(*(parent_nodes.rbegin() + 1))->children[1] == &node;
        }

        [[nodiscard]] static bool
        is_left_side_of_assignment(const std::vector<const ::sqf::parser::sqf::bison::astnode *> &parent_nodes,
                                   const ::sqf::parser::sqf::bison::astnode &node) {
            return parent_nodes.size() > 2
                   && (*(parent_nodes.rbegin() + 1))->kind == ::sqf::parser::sqf::bison::astkind::ASSIGNMENT
                   && (*(parent_nodes.rbegin() + 1))->children.size() >= 1
                   && &(*(parent_nodes.rbegin() + 1))->children[0] == &node;
        }

        [[nodiscard]] bool
        is_right_side_of_assignment(const std::vector<const ::sqf::parser::sqf::bison::astnode *> &parent_nodes,
                                    const ::sqf::parser::sqf::bison::astnode &node) const {
            return assignment_temp_match.has_value()
                   && parent_nodes.size() > 2
                   && (*(parent_nodes.rbegin() + 1))->kind == ::sqf::parser::sqf::bison::astkind::ASSIGNMENT
                   && (*(parent_nodes.rbegin() + 1))->children.size() >= 2
                   && &(*(parent_nodes.rbegin() + 1))->children[1] == &node;
        }

        [[nodiscard]] method create_method() const {
            method m;
            m.name = assignment_temp_match->name;
            m.position = assignment_temp_match->position;
            m.type = variable_type::method;
            bool is_private = m.name.length() > 1 && m.name[0] == '_';
            if (is_private) {
                m.varscope = variable_scope::local;
                m.scope = assignment_temp_match->scope;
            } else {
                m.varscope = variable_scope::global;
                m.scope = "";
            }
            return m;
        }

        void set_scope_name(strmtch &match) {
            size_t length = std::accumulate(
                    scope_stack.begin(), scope_stack.end(), (size_t) 0, [](auto &l, auto &r) {
                        return l + r.length() + 1;
                    });
            match.scope.reserve(length);
            for (auto &str: scope_stack) {
                match.scope.append(str);
                match.scope.append("-");
            }
        }

        void exit(analyzer_sqf &a, const ::sqf::parser::sqf::bison::astnode &node,
                  const std::vector<const ::sqf::parser::sqf::bison::astnode *> &parent_nodes) override {
            switch (node.kind) {
                case ::sqf::parser::sqf::bison::astkind::CODE: {
                    auto m = method_temps.back();
                    if (m.has_value()) {
                        a.res.methods_set.push_back(m.value());
                    }
                    method_temps.pop_back();
                    scope_stack.pop_back();
                    break;
                }
                case ::sqf::parser::sqf::bison::astkind::ASSIGNMENT: {
                    if (assignment_temp_match.has_value()) {
                        a.res.variables_set.push_back(assignment_temp_match.value());
                        assignment_temp_match = {};
                    }
                    break;
                }
                default:
                    break;
            }
        }

        void end(analyzer_sqf &a) override {
        }
    };
}