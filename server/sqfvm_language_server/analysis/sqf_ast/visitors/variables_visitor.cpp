#include "variables_visitor.hpp"
#include "../sqf_ast_analyzer.hpp"

#include <algorithm>

#define LINE_OFFSET -1

using namespace sqfvm::language_server::database::tables;
using namespace std::string_view_literals;

namespace {

    t_diagnostic diag_private_variable_value_is_never_used_001(
            const t_variable &variable,
            const t_reference &reference) {
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

    t_diagnostic diag_global_variable_value_is_never_used_in_file_002(
            const t_variable &variable,
            const t_reference &reference) {
        return {
                .id_pk = {},
                .file_fk = reference.file_fk,
                .line = reference.line + LINE_OFFSET,
                .column = reference.column,
                .offset = reference.offset,
                .length = reference.length,
                .severity = t_diagnostic::verbose,
                .message = "Global variable '" + variable.variable_name + "' is never used in this file",
                .content = variable.variable_name,
                .code = "VV-002",
        };
    }

    t_diagnostic diag_private_variable_is_never_assigned_003(
            const t_variable &variable,
            const t_reference &reference) {
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


    t_diagnostic diag_global_variable_never_assigned_in_file_in_file_004(
            const t_variable &variable,
            const t_reference &reference) {
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

    t_diagnostic diag_variable_name_not_similar_005(
            const t_variable &variable,
            const t_reference &reference,
            std::string_view reference_content) {
        std::string content{};
        content.reserve(
                "Expected: "sv.length()
                + variable.variable_name.length()
                + ", got: "sv.length()
                + reference_content.length()
        );
        content.append("Expected: "sv);
        content.append(variable.variable_name);
        content.append(", got: "sv);
        content.append(reference_content);
        return {
                .id_pk = {},
                .file_fk = reference.file_fk,
                .line = reference.line + LINE_OFFSET,
                .column = reference.column,
                .offset = reference.offset,
                .length = reference.length,
                .severity = t_diagnostic::info,
                .message = "Variable name differs from declared name",
                .content = content,
                .code = "VV-005",
        };
    }

    t_diagnostic diag_type_missmatch_006(
            const uint64_t file_fk,
            const sqf::parser::sqf::bison::astnode &node,
            sqf::parser::sqf::bison::astkind expected_type) {
        std::string content{};
        auto got_type_str = to_string_view(node.kind);
        auto expected_type_str = to_string_view(expected_type);
        content.reserve(
                "Expected "sv.length()
                + expected_type_str.length()
                + ", got "sv.length()
                + got_type_str.length()
        );
        content.append("Expected: "sv);
        content.append(expected_type_str);
        content.append(", got: "sv);
        content.append(got_type_str);
        return {
                .id_pk = {},
                .file_fk = file_fk,
                .line = node.token.line + LINE_OFFSET,
                .column = node.token.column,
                .offset = node.token.offset,
                .length = node.token.contents.length(),
                .severity = t_diagnostic::error,
                .message = "Node type mismatch",
                .content = content,
                .code = "VV-006",
        };
    }

    t_diagnostic diag_type_missmatch_006(
            const uint64_t file_fk,
            const sqf::parser::sqf::bison::astnode &node,
            sqf::parser::sqf::bison::astkind expected_type_1,
            sqf::parser::sqf::bison::astkind expected_type_2) {
        std::string content{};
        auto got_type_str = to_string_view(node.kind);
        auto expected_type_1_str = to_string_view(expected_type_1);
        auto expected_type_2_str = to_string_view(expected_type_2);
        content.reserve(
                "Expected "sv.length()
                + expected_type_1_str.length()
                + " or "sv.length()
                + expected_type_2_str.length()
                + ", got "sv.length()
                + got_type_str.length()
        );
        content.append("Expected: "sv);
        content.append(expected_type_1_str);
        content.append(" or "sv);
        content.append(expected_type_2_str);
        content.append(", got: "sv);
        content.append(got_type_str);
        return {
                .id_pk = {},
                .file_fk = file_fk,
                .line = node.token.line + LINE_OFFSET,
                .column = node.token.column,
                .offset = node.token.offset,
                .length = node.token.contents.length(),
                .severity = t_diagnostic::error,
                .message = "Node type mismatch",
                .content = content,
                .code = "VV-006",
        };
    }

    t_diagnostic diag_cannot_determine_variable_from_expression_007(
            const uint64_t file_fk,
            const sqf::parser::sqf::bison::astnode &node,
            sqf::parser::sqf::bison::astkind expected_type) {
        std::string content{};
        auto got_type_str = to_string_view(node.kind);
        auto expected_type_str = to_string_view(expected_type);
        content.reserve(
                "Expected "sv.length()
                + expected_type_str.length()
                + ", got "sv.length()
                + got_type_str.length()
        );
        content.append("Expected: "sv);
        content.append(expected_type_str);
        content.append(", got: "sv);
        content.append(got_type_str);
        return {
                .id_pk = {},
                .file_fk = file_fk,
                .line = node.token.line + LINE_OFFSET,
                .column = node.token.column,
                .offset = node.token.offset,
                .length = node.token.contents.length(),
                .severity = t_diagnostic::verbose,
                .message = "The provided type cannot be used to determine the variable name for referral",
                .content = content,
                .code = "VV-007",
        };
    }

    t_diagnostic diag_cannot_determine_variable_from_expression_007(
            const uint64_t file_fk,
            const sqf::parser::sqf::bison::astnode &node,
            sqf::parser::sqf::bison::astkind expected_type_1,
            sqf::parser::sqf::bison::astkind expected_type_2) {
        std::string content{};
        auto got_type_str = to_string_view(node.kind);
        auto expected_type_1_str = to_string_view(expected_type_1);
        auto expected_type_2_str = to_string_view(expected_type_2);
        content.reserve(
                "Expected "sv.length()
                + expected_type_1_str.length()
                + " or "sv.length()
                + expected_type_2_str.length()
                + ", got "sv.length()
                + got_type_str.length()
        );
        content.append("Expected: "sv);
        content.append(expected_type_1_str);
        content.append(" or "sv);
        content.append(expected_type_2_str);
        content.append(", got: "sv);
        content.append(got_type_str);
        return {
                .id_pk = {},
                .file_fk = file_fk,
                .line = node.token.line + LINE_OFFSET,
                .column = node.token.column,
                .offset = node.token.offset,
                .length = node.token.contents.length(),
                .severity = t_diagnostic::verbose,
                .message = "The provided type cannot be used to determine the variable name for referral",
                .content = content,
                .code = "VV-006",
        };
    }
}


void sqfvm::language_server::analysis::sqf_ast::visitors::variables_visitor::start(sqf_ast_analyzer &a) {
    // Push initial scope
    m_scope_stack.push_back({0, scope_name_of(a)});

    // Push initial namespace
    push_namespace("missionNamespace");

    t_reference reference{};
    reference.line = 0;
    reference.column = 0;
    reference.offset = 0;
    reference.length = 0;
    reference.file_fk = file_of(a).id_pk;
    auto variable = get_or_create_variable("_this");
    reference.variable_fk = variable.id_pk;
    reference.access = t_reference::access_flags::set;
    reference.is_magic_variable = true;
    m_references.push_back(reference);
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
            push_scope(a, node, parent_nodes);
            add_magic_variables_to_current_scope(a, node, parent_nodes);
            break;
        }
        case ::sqf::parser::sqf::bison::astkind::EXP0:
        case ::sqf::parser::sqf::bison::astkind::EXP1:
        case ::sqf::parser::sqf::bison::astkind::EXP2:
        case ::sqf::parser::sqf::bison::astkind::EXP3:
        case ::sqf::parser::sqf::bison::astkind::EXP4:
        case ::sqf::parser::sqf::bison::astkind::EXP5:
        case ::sqf::parser::sqf::bison::astkind::EXP6:
        case ::sqf::parser::sqf::bison::astkind::EXP7:
        case ::sqf::parser::sqf::bison::astkind::EXP8:
        case ::sqf::parser::sqf::bison::astkind::EXP9:
        case ::sqf::parser::sqf::bison::astkind::EXPU: {
            auto contents = node.token.contents;
            if (iequal(contents, "private")) {
                expression_handling_of_private(a, node);
            } else if (iequal(contents, "params")) {
                expression_handling_of_params(a, node);
            } else if (iequal(contents, "getVariable")) {
                expression_handling_of_getvariable(a, node);
            } else if (iequal(contents, "setVariable")) {
                expression_handling_of_setvariable(a, node);
            } else if (iequal(contents, "isNil")) {
                expression_handling_of_isnil(a, node);
            } else if (iequal(contents, "for")) {
                // for being an unary operator, only the first child is relevant and always present.
                // First child also always must be a string, otherwise pushing diagnostic 006 is required.
                auto first_child = node.children.front();
                if (first_child.kind != ::sqf::parser::sqf::bison::astkind::STRING) {
                    m_diagnostics.push_back(diag_type_missmatch_006(
                            file_of(a).id_pk,
                            node,
                            ::sqf::parser::sqf::bison::astkind::STRING));
                } else {
                    auto variable_name = first_child.token.contents;
                    auto variable = get_or_create_variable(sqf_destringify(variable_name));
                    auto reference = make_reference(a, first_child, variable, t_reference::access_flags::set);
                    m_references.push_back(reference);
                }
            }
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
        case ::sqf::parser::sqf::bison::astkind::IDENT: {
            if (m_assignment_candidate.has_value()) {
                m_assignment_candidate->id_pk = m_references.size() + 1;
                m_references.push_back(*m_assignment_candidate);
                m_assignment_candidate = {};
            }
        } // fallthrough
        case ::sqf::parser::sqf::bison::astkind::ASSIGNMENT_LOCAL: {
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

sqfvm::language_server::database::tables::t_reference
sqfvm::language_server::analysis::sqf_ast::visitors::variables_visitor::make_reference(
        sqfvm::language_server::analysis::sqf_ast::sqf_ast_analyzer &a,
        const sqf::parser::sqf::bison::astnode &node,
        const t_variable &variable,
        const t_reference::access_flags &access) {
    auto reference = make_reference(node);
    reference.file_fk = file_of(a).id_pk;
    reference.variable_fk = variable.id_pk;
    reference.access = access;
    return reference;
}

void sqfvm::language_server::analysis::sqf_ast::visitors::variables_visitor::expression_handling_of_isnil(
        sqfvm::language_server::analysis::sqf_ast::sqf_ast_analyzer &a,
        const sqf::parser::sqf::bison::astnode &node) {// Call only has a right side argument
    auto right_side = node.children.back();
    // Right side always must be a string
    if (right_side.kind == sqf::parser::sqf::bison::astkind::STRING) {
        auto reference = make_reference(right_side);
        reference.file_fk = file_of(a).id_pk;
        auto variable = get_or_create_variable(sqf_destringify(right_side.token.contents));
        reference.variable_fk = variable.id_pk;
        reference.access = t_reference::access_flags::get;
        m_references.push_back(reference);
    } else if (right_side.kind == sqf::parser::sqf::bison::astkind::CODE) {
        // Handled by normal code block handling, left in for completeness
    } else if (right_side.kind == sqf::parser::sqf::bison::astkind::IDENT) {
        m_diagnostics.push_back(diag_cannot_determine_variable_from_expression_007(
                file_of(a).id_pk,
                right_side,
                ::sqf::parser::sqf::bison::astkind::STRING,
                ::sqf::parser::sqf::bison::astkind::CODE));
    } else {
        m_diagnostics.push_back(diag_type_missmatch_006(
                file_of(a).id_pk,
                right_side,
                ::sqf::parser::sqf::bison::astkind::STRING,
                ::sqf::parser::sqf::bison::astkind::CODE));
    }
}

void sqfvm::language_server::analysis::sqf_ast::visitors::variables_visitor::expression_handling_of_getvariable(
        sqfvm::language_server::analysis::sqf_ast::sqf_ast_analyzer &a,
        const sqf::parser::sqf::bison::astnode &node) {
    // Call may have a left side argument but always has a right side argument
    auto right_side = node.children.back();
    // Right side may be an identifier or a code block, or another expression
    // (namely: namespace getVariable string). We do not care for code blocks
    // but do care for identifiers and expressions
    std::optional<sqf::parser::sqf::bison::astnode> variable_node;
    if (right_side.kind == sqf::parser::sqf::bison::astkind::STRING) {
        variable_node = {right_side};
    } else if (right_side.kind == sqf::parser::sqf::bison::astkind::ARRAY) {
        auto get_variable_right_side_array = right_side.children.front();
        if (get_variable_right_side_array.kind == sqf::parser::sqf::bison::astkind::STRING) {
            variable_node = {get_variable_right_side_array};
        } else if (get_variable_right_side_array.kind == sqf::parser::sqf::bison::astkind::IDENT) {
            m_diagnostics.push_back(diag_cannot_determine_variable_from_expression_007(
                    file_of(a).id_pk,
                    get_variable_right_side_array,
                    ::sqf::parser::sqf::bison::astkind::STRING));
        } else {
            m_diagnostics.push_back(diag_type_missmatch_006(
                    file_of(a).id_pk,
                    get_variable_right_side_array,
                    ::sqf::parser::sqf::bison::astkind::STRING));
        }
    } else if (right_side.kind == sqf::parser::sqf::bison::astkind::IDENT) {
        m_diagnostics.push_back(diag_cannot_determine_variable_from_expression_007(
                file_of(a).id_pk,
                right_side,
                ::sqf::parser::sqf::bison::astkind::STRING,
                ::sqf::parser::sqf::bison::astkind::ARRAY));
    } else {
        m_diagnostics.push_back(diag_type_missmatch_006(
                file_of(a).id_pk,
                right_side,
                ::sqf::parser::sqf::bison::astkind::STRING,
                ::sqf::parser::sqf::bison::astkind::ARRAY));
    }
    if (variable_node.has_value()) {
        auto reference = make_reference(variable_node.value());
        reference.file_fk = file_of(a).id_pk;
        auto variable = get_or_create_variable(sqf_destringify(variable_node->token.contents));
        reference.variable_fk = variable.id_pk;
        reference.access = t_reference::access_flags::get;
        m_references.push_back(reference);
    }
}

void sqfvm::language_server::analysis::sqf_ast::visitors::variables_visitor::expression_handling_of_setvariable(
        sqfvm::language_server::analysis::sqf_ast::sqf_ast_analyzer &a,
        const sqf::parser::sqf::bison::astnode &node) {
    // Call may have a left side argument but always has a right side argument
    auto right_side = node.children.back();
    // Right side may be an identifier or a code block, or another expression
    // (namely: namespace getVariable string). We do not care for code blocks
    // but do care for identifiers and expressions
    std::optional<sqf::parser::sqf::bison::astnode> variable_node;
    if (right_side.kind == sqf::parser::sqf::bison::astkind::ARRAY) {
        auto set_variable_right_side_array = right_side.children.front();
        if (set_variable_right_side_array.kind == sqf::parser::sqf::bison::astkind::STRING) {
            variable_node = {set_variable_right_side_array};
        } else if (set_variable_right_side_array.kind == sqf::parser::sqf::bison::astkind::IDENT) {
            m_diagnostics.push_back(diag_cannot_determine_variable_from_expression_007(
                    file_of(a).id_pk,
                    set_variable_right_side_array,
                    ::sqf::parser::sqf::bison::astkind::STRING));
        } else {
            m_diagnostics.push_back(diag_type_missmatch_006(
                    file_of(a).id_pk,
                    set_variable_right_side_array,
                    ::sqf::parser::sqf::bison::astkind::STRING));
        }
    } else if (right_side.kind == sqf::parser::sqf::bison::astkind::IDENT) {
        m_diagnostics.push_back(diag_cannot_determine_variable_from_expression_007(
                file_of(a).id_pk,
                right_side,
                ::sqf::parser::sqf::bison::astkind::STRING,
                ::sqf::parser::sqf::bison::astkind::ARRAY));
    } else {
        m_diagnostics.push_back(diag_type_missmatch_006(
                file_of(a).id_pk,
                right_side,
                ::sqf::parser::sqf::bison::astkind::STRING));
    }
    if (variable_node.has_value()) {
        auto reference = make_reference(variable_node.value());
        reference.file_fk = file_of(a).id_pk;
        auto variable = get_or_create_variable(sqf_destringify(variable_node->token.contents));
        reference.variable_fk = variable.id_pk;
        reference.access = t_reference::access_flags::set;
        m_references.push_back(reference);
    }
}

void sqfvm::language_server::analysis::sqf_ast::visitors::variables_visitor::expression_handling_of_params(
        sqfvm::language_server::analysis::sqf_ast::sqf_ast_analyzer &a,
        const sqf::parser::sqf::bison::astnode &node) {// params may have a left value and always has a right value
    auto right_value = node.children.back();
    // Params right value is always an array but the array can contain a mix of strings and arrays, with
    // the nested arrays containing the string we are looking for at the start of the array.
    std::vector<sqf::parser::sqf::bison::astnode> variable_nodes{};
    if (right_value.kind == sqf::parser::sqf::bison::astkind::ARRAY) {
        for (auto &child: right_value.children) {
            if (child.kind == sqf::parser::sqf::bison::astkind::STRING) {
                variable_nodes.push_back(child);
            } else if (child.kind == sqf::parser::sqf::bison::astkind::ARRAY) {
                if (child.children.front().kind == sqf::parser::sqf::bison::astkind::STRING) {
                    variable_nodes.push_back(child.children.front());
                } else if (child.children.front().kind == sqf::parser::sqf::bison::astkind::IDENT) {
                    m_diagnostics.push_back(diag_cannot_determine_variable_from_expression_007(
                            file_of(a).id_pk,
                            child.children.front(),
                            ::sqf::parser::sqf::bison::astkind::STRING));
                } else {
                    m_diagnostics.push_back(diag_type_missmatch_006(
                            file_of(a).id_pk,
                            child.children.front(),
                            ::sqf::parser::sqf::bison::astkind::STRING));
                }
            } else if (child.kind == sqf::parser::sqf::bison::astkind::IDENT) {
                m_diagnostics.push_back(diag_cannot_determine_variable_from_expression_007(
                        file_of(a).id_pk,
                        child,
                        ::sqf::parser::sqf::bison::astkind::STRING,
                        ::sqf::parser::sqf::bison::astkind::ARRAY));
            } else {
                m_diagnostics.push_back(diag_type_missmatch_006(
                        file_of(a).id_pk,
                        child,
                        ::sqf::parser::sqf::bison::astkind::STRING,
                        ::sqf::parser::sqf::bison::astkind::ARRAY));
            }
        }
    }
    for (auto &variable_node: variable_nodes) {
        auto reference = make_reference(variable_node);
        reference.file_fk = file_of(a).id_pk;
        auto variable = get_or_create_variable(sqf_destringify(variable_node.token.contents));
        reference.variable_fk = variable.id_pk;
        reference.access = t_reference::access_flags::set;
        m_references.push_back(reference);
    }
}

void sqfvm::language_server::analysis::sqf_ast::visitors::variables_visitor::expression_handling_of_private(
        sqfvm::language_server::analysis::sqf_ast::sqf_ast_analyzer &a,
        const sqf::parser::sqf::bison::astnode &node) {// private always has a right value and never a left value, so we can just take the first child
    auto right_value = node.children.front();
    std::vector<sqf::parser::sqf::bison::astnode> variable_nodes{};
    if (right_value.kind == sqf::parser::sqf::bison::astkind::STRING) {
        variable_nodes.push_back(right_value);
    } else if (right_value.kind == sqf::parser::sqf::bison::astkind::ARRAY) {
        for (auto &child: right_value.children) {
            if (child.kind == sqf::parser::sqf::bison::astkind::STRING) {
                variable_nodes.push_back(child);
            } else if (child.kind == sqf::parser::sqf::bison::astkind::IDENT) {
                m_diagnostics.push_back(diag_cannot_determine_variable_from_expression_007(
                        file_of(a).id_pk,
                        child,
                        ::sqf::parser::sqf::bison::astkind::STRING));
            } else {
                m_diagnostics.push_back(diag_type_missmatch_006(
                        file_of(a).id_pk,
                        child,
                        ::sqf::parser::sqf::bison::astkind::STRING));
            }
        }
    } else if (right_value.kind == sqf::parser::sqf::bison::astkind::IDENT) {
        m_diagnostics.push_back(diag_cannot_determine_variable_from_expression_007(
                file_of(a).id_pk,
                right_value,
                ::sqf::parser::sqf::bison::astkind::STRING,
                ::sqf::parser::sqf::bison::astkind::ARRAY));
    } else {
        m_diagnostics.push_back(diag_type_missmatch_006(
                file_of(a).id_pk,
                right_value,
                ::sqf::parser::sqf::bison::astkind::STRING,
                ::sqf::parser::sqf::bison::astkind::ARRAY));
    }
    for (auto &variable_node: variable_nodes) {
        auto reference = make_reference(variable_node);
        reference.file_fk = file_of(a).id_pk;
        auto variable = get_or_create_variable(sqf_destringify(variable_node.token.contents));
        reference.variable_fk = variable.id_pk;
        reference.types = database::tables::t_reference::type_flags::nil;
        reference.access = t_reference::access_flags::set;
        m_references.push_back(reference);
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
        const ::sqf::parser::sqf::bison::astnode &node) const {
    // !WARNING! while this method looks similar to is_right_side_of_assignment, it is not the same!
    // The return values are not straight inverted, as we are checking explicitly whether this is the
    // left side of an assignment, not just the opposite of the right side.
    using namespace ::sqf::parser::sqf::bison;
    if (node.kind == astkind::ASSIGNMENT_LOCAL)
        return true;
    if (parent_nodes.empty())
        return false;
    auto &parent = *parent_nodes.back();
    if (parent.kind == astkind::ASSIGNMENT_LOCAL)
        return false;
    return parent.kind == astkind::ASSIGNMENT
           && !parent.children.empty()
           && &parent.children.front() == &node;
}

bool sqfvm::language_server::analysis::sqf_ast::visitors::variables_visitor::is_right_side_of_assignment(
        const std::vector<const ::sqf::parser::sqf::bison::astnode *> &parent_nodes,
        const ::sqf::parser::sqf::bison::astnode &node) const {
    // !WARNING! while this method looks similar to is_left_side_of_assignment, it is not the same!
    // The return values are not straight inverted, as we are checking explicitly whether this is the
    // right side of an assignment, not just the opposite of the left side.
    using namespace ::sqf::parser::sqf::bison;
    if (node.kind == astkind::ASSIGNMENT_LOCAL)
        return false;
    if (parent_nodes.empty())
        return false;
    auto &parent = *parent_nodes.back();
    if (parent.kind == astkind::ASSIGNMENT_LOCAL)
        return true;
    return parent.kind == astkind::ASSIGNMENT
           && !parent.children.empty()
           && &parent.children.front() != &node;
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
        sqf_ast_analyzer &a,
        const ::sqf::parser::sqf::bison::astnode &node,
        const std::vector<const ::sqf::parser::sqf::bison::astnode *> &parent_nodes
) {
    bool is_detached = is_detached_scope(parent_nodes);
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
    variables_visitor::scope s{0, scope, is_detached};
    m_scope_stack.push_back(s);
    if (is_detached) {
        // ToDo: implement properly (not every {} scope has _this)
        auto reference = make_reference(node);
        reference.file_fk = file_of(a).id_pk;
        auto variable = get_or_create_variable("_this");
        reference.variable_fk = variable.id_pk;
        reference.access = t_reference::access_flags::set;
        reference.is_magic_variable = true;
        m_references.push_back(reference);
    }
    return scope;
}

void sqfvm::language_server::analysis::sqf_ast::visitors::variables_visitor::add_magic_variables_to_current_scope(
        sqf_ast_analyzer &a,
        const ::sqf::parser::sqf::bison::astnode &node,
        const std::vector<const ::sqf::parser::sqf::bison::astnode *> &parent_nodes) {
    if (!parent_nodes.empty()) {
        auto parent = parent_nodes.back();
        if (node_is_expression(*parent)) {
            std::vector<std::string> magic_variables{};
            if (iequal(parent->token.contents, "apply")) {
                magic_variables.emplace_back("_x");
            } else if (iequal(parent->token.contents, "select")) {
                magic_variables.emplace_back("_x");
            } else if (iequal(parent->token.contents, "count")) {
                magic_variables.emplace_back("_x");
            } else if (iequal(parent->token.contents, "findIf")) {
                magic_variables.emplace_back("_x");
            } else if (iequal(parent->token.contents, "catch")) {
                magic_variables.emplace_back("_exception");
            } else if (iequal(parent->token.contents, "forEach")) {
                magic_variables.emplace_back("_x");
                magic_variables.emplace_back("_y");
                magic_variables.emplace_back("_forEachIndex");
            }
            for (auto &magic_variable: magic_variables) {
                auto reference = make_reference(node);
                reference.file_fk = file_of(a).id_pk;
                auto variable = get_or_create_variable(magic_variable);
                reference.variable_fk = variable.id_pk;
                reference.access = t_reference::access_flags::set;
                reference.is_magic_variable = true;
                m_references.push_back(reference);
            }
        }
    }
}

bool sqfvm::language_server::analysis::sqf_ast::visitors::variables_visitor::is_detached_scope(
        const std::vector<const ::sqf::parser::sqf::bison::astnode *> &parent_nodes) const {
    bool is_detached = false;
    if (!parent_nodes.empty()) {
        auto parent = parent_nodes.back();
        if (!node_is_expression(*parent)
            || !(
                iequal(parent->token.contents, "then")
                || iequal(parent->token.contents, "else")
                || iequal(parent->token.contents, "call")
                || iequal(parent->token.contents, "while")
                || iequal(parent->token.contents, "do")
                || iequal(parent->token.contents, "switch")
                || iequal(parent->token.contents, ":")
                || iequal(parent->token.contents, "default")
                || iequal(parent->token.contents, "isNil")
                || iequal(parent->token.contents, "waitUntil")
                || iequal(parent->token.contents, "try")
                || iequal(parent->token.contents, "catch")
                || iequal(parent->token.contents, "count")
                || iequal(parent->token.contents, "forEach")
                || iequal(parent->token.contents, "apply")
                || iequal(parent->token.contents, "select")
                || iequal(parent->token.contents, "findIf")
        )) {
            is_detached = true;
        }
    }
    return is_detached;
}

bool sqfvm::language_server::analysis::sqf_ast::visitors::variables_visitor::node_is_expression(
        const sqf::parser::sqf::bison::astnode &parent) {
    return parent.kind == sqf::parser::sqf::bison::astkind::EXP0
           || parent.kind == sqf::parser::sqf::bison::astkind::EXP1
           || parent.kind == sqf::parser::sqf::bison::astkind::EXP2
           || parent.kind == sqf::parser::sqf::bison::astkind::EXP3
           || parent.kind == sqf::parser::sqf::bison::astkind::EXP4
           || parent.kind == sqf::parser::sqf::bison::astkind::EXP5
           || parent.kind == sqf::parser::sqf::bison::astkind::EXP6
           || parent.kind == sqf::parser::sqf::bison::astkind::EXP7
           || parent.kind == sqf::parser::sqf::bison::astkind::EXP8
           || parent.kind == sqf::parser::sqf::bison::astkind::EXP9
           || parent.kind == sqf::parser::sqf::bison::astkind::EXPU;
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
                        return iequal(val.variable_name, name) && val.scope == scope.full_name;
                    });
            if (find_res != m_variables.end()) {
                return *find_res;
            }
            if (scope.is_detached)
                break;
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
                    return iequal(val.variable_name, name);
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

void sqfvm::language_server::analysis::sqf_ast::visitors::variables_visitor::analyze(
        sqfvm::language_server::analysis::sqf_ast::sqf_ast_analyzer &sqf_ast_analyzer,
        const sqfvm::language_server::database::context &context) {
    using namespace sqlite_orm;

    // Find all variables which are only set once and never read
    for (auto &variable: m_variables) {
        for (auto it = m_references.begin(); it != m_references.end(); it++) {
            if (it->variable_fk != variable.id_pk || it->access != t_reference::access_flags::set ||
                it->is_magic_variable)
                continue; // ToDo: Optimize the lookup as we are O(N^M) here
            const auto &initial_reference = it;

            if (it->types == database::tables::t_reference::type_flags::nil)
                continue; // we treat nil assignment as intentional

            // We just need to check the following references due to read order
            auto next_reference = std::find_if(initial_reference + 1, m_references.end(),
                                               [&variable](const t_reference &reference) {
                                                   return reference.variable_fk == variable.id_pk;
                                               });
            if (next_reference == m_references.end() || next_reference->access != t_reference::access_flags::get) {
                if (is_private_variable(variable)) {
                    m_diagnostics.push_back(diag_private_variable_value_is_never_used_001(
                            variable,
                            *initial_reference));
                } else {
                    m_diagnostics.push_back(diag_global_variable_value_is_never_used_in_file_002(
                            variable,
                            *initial_reference));
                }
            }
        }
    }

    // Find all variables which are never set
    for (auto &variable: m_variables) {
        for (auto it = m_references.begin(); it != m_references.end(); it++) {
            if (it->variable_fk != variable.id_pk || it->access != t_reference::access_flags::get ||
                it->is_magic_variable)
                continue; // ToDo: Optimize the lookup as we are O(N^M) here
            const auto &initial_reference = it;
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
                            variable,
                            *initial_reference));
                } else {
                    m_diagnostics.push_back(diag_global_variable_never_assigned_in_file_in_file_004(
                            variable,
                            *initial_reference));
                }
            }
        }
    }

    // Find all variables which are never set
    for (auto &reference: m_references) {
        if (reference.is_magic_variable)
            continue;
        auto find_res = std::find_if(
                m_variables.begin(),
                m_variables.end(),
                [&reference](const t_variable &var) {
                    return var.id_pk == reference.variable_fk;
                });
        if (find_res == m_variables.end())
            continue;
        auto reference_content = text_of(sqf_ast_analyzer).substr(reference.offset, reference.length);
        if (!reference_content.empty() && (reference_content[0] == '"' || reference_content[0] == '\'')) {
            // Compare variable name with variable name in reference
            auto destringified = sqf_destringify(reference_content);
            if (find_res->variable_name != destringified) {
                m_diagnostics.push_back(diag_variable_name_not_similar_005(
                        *find_res,
                        reference,
                        destringified));
            }
        } else {
            // Compare variable name with variable name in reference
            if (find_res->variable_name != reference_content) {
                m_diagnostics.push_back(diag_variable_name_not_similar_005(
                        *find_res,
                        reference,
                        reference_content));
            }
        }
    }
}
