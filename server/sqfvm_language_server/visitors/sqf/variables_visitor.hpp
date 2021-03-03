#pragma once
#include "../../sqfanalyzer.hpp"
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>
#include <optional>

namespace sqfvm::lsp::visitors::sqf
{
    struct variables_visitor : public sqfvisitor
    {
        std::vector<std::string> scope_stack;
        std::optional<strmtch> assignment_temp_match;
        std::vector<std::optional<method>> method_temps;
        virtual ~variables_visitor() { }
        virtual void start(sqfanalyzer& a) override
        {
        }
        virtual void enter(sqfanalyzer& a, const ::sqf::parser::sqf::bison::astnode& node, const std::vector<const ::sqf::parser::sqf::bison::astnode&>& astkinds) override
        {
            switch (node.kind)
            {
                case ::sqf::parser::sqf::bison::astkind::CODE: {
                    // Check if we are at the right side of an ASSIGNMENT
                    if (assignment_temp_match.has_value()
                        && astkinds.size() > 2
                        && (astkinds.rbegin() + 1)->kind == ::sqf::parser::sqf::bison::astkind::ASSIGNMENT
                        && (astkinds.rbegin() + 1)->children[1] == node)
                    { // Yes - Change the temp of assignment to method_assigned and put to method_temps for later addition to methods used (with possible args)
                        method m;
                        m.name = assignment_temp_match->name;
                        m.position = assignment_temp_match->pos;
                        m.type = variable_type::method;
                        if (m.name.length() > 1 && m.name[0] == '_')
                        {
                            m.varscope = variable_scope::local;
                            m.scope = assignment_temp_match->scope;
                        }
                        else
                        {
                            m.varscope = variable_scope::global;
                            m.scope = "";
                        }
                        method_temps.push_back(m);
                        assignment_temp_match = {};
                    }
                    else
                    {
                        method_temps.push_back({});
                    }

                    // Push scope info
                    auto str_line = std::to_string(node.token.line);
                    auto str_column = std::to_string(node.token.column);
                    scope_stack.push_back(str_line + "-" + str_column);
                } break;
                case ::sqf::parser::sqf::bison::astkind::ASSIGNMENT_LOCAL: {
                    strmtch match;
                    size_t length = std::accumulate(scope_stack.begin(), scope_stack.end(), 0, [](std::string& str) -> size_t { return str.length() + 1; });
                    match.scope.reserve(length);
                    for (auto& str : scope_stack)
                    {
                        match.scope.append(str);
                        match.scope.append("-");
                    }
                    match.name = node.token.contents;
                    a.res.variables_set.push_back(match);
                } break;
                case ::sqf::parser::sqf::bison::astkind::IDENT: {
                    strmtch match;
                    size_t length = std::accumulate(scope_stack.begin(), scope_stack.end(), 0, [](std::string& str) -> size_t { return str.length() + 1; });
                    match.scope.reserve(length);
                    for (auto& str : scope_stack)
                    {
                        match.scope.append(str);
                        match.scope.append("-");
                    }
                    match.name = node.token.contents;
                    // Check if we are at the left side of an assignment
                    if (astkinds.size() > 2 && (astkinds.rbegin() + 1)->kind == ::sqf::parser::sqf::bison::astkind::ASSIGNMENT && (astkinds.rbegin() + 1)->children[0] == node)
                    { // Yes - We set this IDENT so put it into temp for later evaluation in parent or the code following
                        assignment_temp_match = match;
                    }
                    else
                    { // No - We get this IDENT
                        // Check if we are at the right side of a CALL
                        if (astkinds.size() > 2
                            && (astkinds.rbegin() + 1)->kind == ::sqf::parser::sqf::bison::astkind::EXP4
                            && (astkinds.rbegin() + 1)->token.contents == "call"
                            && (astkinds.rbegin() + 1)->children[1] == node)
                        { // Yes - We set this IDENT
                            a.res.methods_used.push_back(match);
                        }
                        else
                        { // No - We get this IDENT
                            a.res.variables_used.push_back(match);
                        }
                    }
                } break;
            }
        }
        virtual void exit(sqfanalyzer& a, const ::sqf::parser::sqf::bison::astnode& node, const std::vector<const ::sqf::parser::sqf::bison::astnode&>& astkinds) override
        {
            switch (node.kind)
            {
                case ::sqf::parser::sqf::bison::astkind::CODE:
                {
                    auto m = method_temps.back();
                    if (m.has_value())
                    {
                        a.res.methods_set.push_back(m.value());
                    }
                    method_temps.pop_back();
                    scope_stack.pop_back();
                }
                case ::sqf::parser::sqf::bison::astkind::ASSIGNMENT:
                {
                    if (assignment_temp_match.has_value())
                    {
                        a.res.variables_set.push_back(assignment_temp_match.value());
                        assignment_temp_match = {};
                    }
                }
                break;
            }
        }
        virtual void end(sqfanalyzer& a) override
        {
        }
    };
}