#include "variables_visitor.hpp"
#include "../sqf_ast_analyzer.hpp"

#include <algorithm>

#define LINE_OFFSET -1

using namespace sqfvm::language_server::database::tables;
using namespace std::string_view_literals;

void sqfvm::language_server::analysis::sqf_ast::visitors::variables_visitor::start(sqf_ast_analyzer &a) {
    // Push initial scope
    m_scope_stack.push_back({0, scope_name_of(a)});

    // Push initial namespace
    push_namespace("missionNamespace");
}


void sqfvm::language_server::analysis::sqf_ast::visitors::variables_visitor::enter(
        sqf_ast_analyzer &a,
        const ::sqf::parser::sqf::bison::astnode &node,
        const std::vector<const ::sqf::parser::sqf::bison::astnode *> &parent_nodes
) {
    switch (node.kind) {
        case ::sqf::parser::sqf::bison::astkind::CODE: {
            if (is_right_side_of_assignment(parent_nodes, node)) {
                m_assignment_candidate->types = t_reference::type_flags::code;
                m_assignment_candidate->id_pk = m_references.size() + 1;
                m_references.push_back(*m_assignment_candidate);
                m_assignment_candidate = {};
            }

            // Push scope info
            push_scope(node, parent_nodes);
            break;
        }
        case ::sqf::parser::sqf::bison::astkind::BOOLEAN_FALSE:
        case ::sqf::parser::sqf::bison::astkind::BOOLEAN_TRUE: {
            if (m_assignment_candidate.has_value() && is_right_side_of_assignment(parent_nodes, node)) {
                m_assignment_candidate->types = t_reference::type_flags::boolean;
                m_assignment_candidate->id_pk = m_references.size() + 1;
                m_references.push_back(*m_assignment_candidate);
                m_assignment_candidate = {};
            }
            break;
        }
        case ::sqf::parser::sqf::bison::astkind::ARRAY: {
            if (m_assignment_candidate.has_value() && is_right_side_of_assignment(parent_nodes, node)) {
                m_assignment_candidate->types = t_reference::type_flags::array;
                m_assignment_candidate->id_pk = m_references.size() + 1;
                m_references.push_back(*m_assignment_candidate);
                m_assignment_candidate = {};
            }
            break;
        }
        case ::sqf::parser::sqf::bison::astkind::HEXNUMBER:
        case ::sqf::parser::sqf::bison::astkind::NUMBER: {
            if (m_assignment_candidate.has_value() && is_right_side_of_assignment(parent_nodes, node)) {
                m_assignment_candidate->types = t_reference::type_flags::scalar;
                m_assignment_candidate->id_pk = m_references.size() + 1;
                m_references.push_back(*m_assignment_candidate);
                m_assignment_candidate = {};
            }
            break;
        }
        case ::sqf::parser::sqf::bison::astkind::STRING: {
            if (m_assignment_candidate.has_value() && is_right_side_of_assignment(parent_nodes, node)) {
                m_assignment_candidate->types = t_reference::type_flags::string;
                m_assignment_candidate->id_pk = m_references.size() + 1;
                m_references.push_back(*m_assignment_candidate);
                m_assignment_candidate = {};
            }
            break;
        }
        case ::sqf::parser::sqf::bison::astkind::ASSIGNMENT_LOCAL:
        case ::sqf::parser::sqf::bison::astkind::IDENT: {
            auto reference = make_reference(node);
            reference.file_fk = file_of(a).id_pk;
            auto variable = get_or_create_variable(node.token.contents);
            reference.variable_fk = variable.id_pk;

            // Check if we are on the left side of an assignment
            if (is_left_side_of_assignment(parent_nodes, node)) {
                // Yes - We set this IDENT so put it into temp for later evaluation in parent or the code following
                reference.access = t_reference::access_flags::set;
                m_assignment_candidate = reference;
            } else { // No - We get this IDENT
                reference.access = t_reference::access_flags::get;
                reference.id_pk = m_references.size() + 1;
                m_references.push_back(reference);
            }
            break;
        }
        default:
            break;
    }
}

void sqfvm::language_server::analysis::sqf_ast::visitors::variables_visitor::exit(
        sqf_ast_analyzer &a,
        const ::sqf::parser::sqf::bison::astnode &node,
        const std::vector<const ::sqf::parser::sqf::bison::astnode *> &parent_nodes) {
    switch (node.kind) {
        case ::sqf::parser::sqf::bison::astkind::CODE: {
            pop_scope();
            break;
        }
        case ::sqf::parser::sqf::bison::astkind::ASSIGNMENT_LOCAL:
        case ::sqf::parser::sqf::bison::astkind::ASSIGNMENT: {
            if (m_assignment_candidate.has_value()) {
                m_assignment_candidate->types = t_reference::type_flags::any;
                m_assignment_candidate->id_pk = m_references.size() + 1;
                m_references.push_back(m_assignment_candidate.value());
                m_assignment_candidate = {};
            }
            break;
        }
        default:
            break;
    }
}

void sqfvm::language_server::analysis::sqf_ast::visitors::variables_visitor::end(sqf_ast_analyzer &a) {
    pop_scope();
    pop_namespace();
}

bool sqfvm::language_server::analysis::sqf_ast::visitors::variables_visitor::is_left_side_of_assignment(
        const std::vector<const ::sqf::parser::sqf::bison::astnode *> &parent_nodes,
        const ::sqf::parser::sqf::bison::astnode &node) {
    return node.kind == ::sqf::parser::sqf::bison::astkind::ASSIGNMENT_LOCAL
           || parent_nodes.size() > 2
              && (
                      (*(parent_nodes.rbegin() + 1))->kind == ::sqf::parser::sqf::bison::astkind::ASSIGNMENT
                      || (*(parent_nodes.rbegin() + 1))->kind == ::sqf::parser::sqf::bison::astkind::ASSIGNMENT_LOCAL
              )
              && (*(parent_nodes.rbegin() + 1))->children.size() >= 1
              && &(*(parent_nodes.rbegin() + 1))->children[0] == &node;
}

bool sqfvm::language_server::analysis::sqf_ast::visitors::variables_visitor::is_right_side_of_assignment(
        const std::vector<const ::sqf::parser::sqf::bison::astnode *> &parent_nodes,
        const ::sqf::parser::sqf::bison::astnode &node) const {
    return parent_nodes.size() > 2
           && (
                   (*(parent_nodes.rbegin() + 1))->kind == ::sqf::parser::sqf::bison::astkind::ASSIGNMENT
                   || (*(parent_nodes.rbegin() + 1))->kind == ::sqf::parser::sqf::bison::astkind::ASSIGNMENT_LOCAL
           )
           && (*(parent_nodes.rbegin() + 1))->children.size() >= 2
           && &(*(parent_nodes.rbegin() + 1))->children[1] == &node;
}

t_reference sqfvm::language_server::analysis::sqf_ast::visitors::variables_visitor::make_reference(
        const ::sqf::parser::sqf::bison::astnode &node) const {
    t_reference reference{};
    reference.line = node.token.line;
    reference.column = node.token.column;
    reference.offset = node.token.offset;
    reference.length = node.token.contents.length();

    return reference;
}

std::string sqfvm::language_server::analysis::sqf_ast::visitors::variables_visitor::push_scope(
        const ::sqf::parser::sqf::bison::astnode &node,
        const std::vector<const ::sqf::parser::sqf::bison::astnode *> &parent_nodes
) {
    std::string scope{};
    if (m_scope_stack.empty()) {
        scope = "scope://";
    } else {
        auto &back = m_scope_stack.back();
        auto scope_string_size = back.full_name.length() + 1;
        if (back.child_count < 10)
            scope_string_size += 1;
        else if (back.child_count < 100)
            scope_string_size += 2;
        else if (back.child_count < 1000)
            scope_string_size += 3;
        else if (back.child_count < 10000)
            scope_string_size += 4;
        else if (back.child_count < 100000)
            scope_string_size += 5;
        else if (back.child_count < 1000000)
            scope_string_size += 6;
        else if (back.child_count < 10000000)
            scope_string_size += 7;
        else if (back.child_count < 100000000)
            scope_string_size += 8;
        else if (back.child_count < 1000000000)
            scope_string_size += 9;
        else
            scope_string_size += 10;

        scope.reserve(scope_string_size);
        scope.append(back.full_name);
        scope.append("/");
        scope.append(std::to_string(back.child_count));
        m_scope_stack.back().child_count++;
    }
    variables_visitor::scope s{0, scope};
    m_scope_stack.push_back(s);
    return scope;
}

void sqfvm::language_server::analysis::sqf_ast::visitors::variables_visitor::pop_scope() {
    m_scope_stack.pop_back();
}

t_variable sqfvm::language_server::analysis::sqf_ast::visitors::variables_visitor::get_or_create_variable(
        std::string_view name) {
    if (is_private_variable(name)) {
        for (auto scope_reverse_it = m_scope_stack.rbegin();
             scope_reverse_it != m_scope_stack.rend(); ++scope_reverse_it) {
            auto &scope = *scope_reverse_it;
            auto find_res = std::find_if(
                    m_variables.begin(),
                    m_variables.end(),
                    [name, &scope](auto val) {
                        return val.variable_name == name && val.scope == scope.full_name;
                    });
            if (find_res != m_variables.end()) {
                return *find_res;
            }
        }
        t_variable variable{};
        variable.variable_name = name;
        variable.id_pk = m_variables.size() + 1;
        variable.scope = m_scope_stack.back().full_name;
        m_variables.push_back(variable);
        return variable;
    } else {
        auto find_res = std::find_if(
                m_variables.begin(),
                m_variables.end(),
                [name](auto val) {
                    return val.variable_name == name;
                });
        if (find_res != m_variables.end()) {
            return *find_res;
        } else {
            t_variable variable{};
            variable.variable_name = name;
            variable.id_pk = m_variables.size() + 1;
            variable.scope = get_namespace();
            m_variables.push_back(variable);
            return variable;
        }
    }
}

namespace {

    t_diagnostic
    diag_private_variable_value_is_never_used_001(const t_variable &variable, const t_reference &reference) {
        return {
                .id_pk = {},
                .file_fk = reference.file_fk,
                .line = reference.line + LINE_OFFSET,
                .column = reference.column,
                .offset = reference.offset,
                .length = reference.length,
                .severity = t_diagnostic::info,
                .message = "Private variable '" + variable.variable_name + "' is never used",
                .content = variable.variable_name,
                .code = "VV-001",
        };
    }

    t_diagnostic
    diag_global_variable_value_is_never_used_in_file_002(const t_variable &variable, const t_reference &reference) {
        return {
                .id_pk = {},
                .file_fk = reference.file_fk,
                .line = reference.line + LINE_OFFSET,
                .column = reference.column,
                .offset = reference.offset,
                .length = reference.length,
                .severity = t_diagnostic::info,
                .message = "Global variable '" + variable.variable_name + "' is never used in this file",
                .content = variable.variable_name,
                .code = "VV-002",
        };
    }

    t_diagnostic diag_private_variable_is_never_assigned_003(const t_variable &variable, const t_reference &reference) {
        return {
                .id_pk = {},
                .file_fk = reference.file_fk,
                .line = reference.line + LINE_OFFSET,
                .column = reference.column,
                .offset = reference.offset,
                .length = reference.length,
                .severity = t_diagnostic::warning,
                .message = "Private variable '" + variable.variable_name + "' is never assigned",
                .content = variable.variable_name,
                .code = "VV-003",
        };
    }


    t_diagnostic
    diag_global_variable_never_assigned_in_file_in_file_004(const t_variable &variable, const t_reference &reference) {
        return {
                .id_pk = {},
                .file_fk = reference.file_fk,
                .line = reference.line + LINE_OFFSET,
                .column = reference.column,
                .offset = reference.offset,
                .length = reference.length,
                .severity = t_diagnostic::verbose,
                .message = "Global variable '" + variable.variable_name + "' is never assigned in this file",
                .content = variable.variable_name,
                .code = "VV-004",
        };
    }

}

void sqfvm::language_server::analysis::sqf_ast::visitors::variables_visitor::analyze(
        const sqfvm::language_server::database::context &context) {
    using namespace sqlite_orm;

    // Find all variables which are only set once and never read
    for (auto &variable: m_variables) {
        for (auto it = m_references.begin(); it != m_references.end(); it++) {
            if (it->variable_fk != variable.id_pk || it->access != t_reference::access_flags::set)
                continue; // ToDo: Optimize the lookup as we are O(N^M) here
            auto initial_reference = it;
            // We just need to check the following references due to read order
            auto next_reference = std::find_if(initial_reference + 1, m_references.end(),
                                               [&variable](const t_reference &reference) {
                                                   return reference.variable_fk == variable.id_pk;
                                               });
            if (next_reference == m_references.end() || next_reference->access != t_reference::access_flags::get) {
                if (is_private_variable(variable)) {
                    m_diagnostics.push_back(diag_private_variable_value_is_never_used_001(
                            variable, *initial_reference));
                } else {
                    m_diagnostics.push_back(diag_global_variable_value_is_never_used_in_file_002(
                            variable, *initial_reference));
                }
            }
        }
    }

    // Find all variables which are never set
    for (auto &variable: m_variables) {
        for (auto it = m_references.begin(); it != m_references.end(); it++) {
            if (it->variable_fk != variable.id_pk || it->access != t_reference::access_flags::get)
                continue; // ToDo: Optimize the lookup as we are O(N^M) here
            auto initial_reference = it;
            // We just need to check the previous references due to read order
            auto previous_set = std::find_if(
                    m_references.begin(),
                    initial_reference,
                    [&variable](const t_reference &reference) {
                        return reference.variable_fk == variable.id_pk
                               && reference.access == t_reference::access_flags::set;
                    });
            if (previous_set == initial_reference) {
                if (is_private_variable(variable)) {
                    m_diagnostics.push_back(diag_private_variable_is_never_assigned_003(
                            variable, *initial_reference));
                } else {
                    m_diagnostics.push_back(diag_global_variable_never_assigned_in_file_in_file_004(
                            variable, *initial_reference));
                }
            }
        }
    }
}
