#include "text_document.h"
#include "sqf_language_server.h"

#include <runtime/runtime.h>
#include <parser/preprocessor/default.h>
#include <runtime/d_array.h>
#include <runtime/d_string.h>
#include <runtime/d_scalar.h>
#include <parser/sqf/tokenizer.hpp>

using namespace sqf::types;
using namespace sqf::runtime;

void text_document::recalculate_ast(
    sqf_language_server& language_server,
    sqf::runtime::runtime& sqfvm,
    std::optional<std::string_view> contents_override)
{
    auto parser = dynamic_cast<sqf::parser::sqf::parser&>(sqfvm.parser_sqf());
    bool errflag = false;
    auto preprocessed = contents_override.has_value() ?
        sqfvm.parser_preprocessor().preprocess(sqfvm, *contents_override, { m_path, {} }) :
        sqfvm.parser_preprocessor().preprocess(sqfvm, { m_path, {} });
    if (preprocessed.has_value())
    {
        m_contents = preprocessed.value();
        sqf::parser::sqf::tokenizer t(m_contents.begin(), m_contents.end(), m_path);
        if (!parser.get_tree(sqfvm, t, &m_root_ast))
        {
            m_root_ast = {};
        }
    }
    else
    {
        lsp::data::diagnostics diag;
        diag.code = "FATAL";
        diag.range.start.line = 0;
        diag.range.start.character = 0;
        diag.range.end.line = 0;
        diag.range.end.character = 0;
        diag.message = "Failed to preprocess (or read) file.";
        diag.severity = lsp::data::diagnostic_severity::Error;
        diag.source = "SQF-VM LS";
        diagnostics.diagnostics.push_back(diag);
    }
}

void text_document::analysis_ensure_L0001_L0003(sqf_language_server& language_server, std::vector<variable_declaration::sptr>& known, size_t level, sqf::parser::sqf::bison::astnode& node, const std::string& orig, bool private_check, variable_declaration::sptr* out_var_decl)
{
    using sqf::parser::sqf::parser;
    std::string variable = orig;
    std::transform(variable.begin(), variable.end(), variable.begin(), [](char c) { return (char)std::tolower(c); });
    auto findRes = std::find_if(known.begin(), known.end(),
        [&variable](variable_declaration::sptr& it) { return it->variable == variable; });
    if (findRes == known.end())
    {
        auto var = std::make_shared<variable_declaration>(level, node.token.line, node.token.column, variable);
        known.push_back(var);
        if (var->variable[0] == '_') // safe as empty std string `0` is `\0`
        {
            auto ref = m_private_declarations.emplace_back(var);
            if (out_var_decl)
            {
                *out_var_decl = ref;
            }
        }
        else
        {
            var->owner = m_path;
            auto ref = m_global_declarations.emplace_back(var);
            if (out_var_decl)
            {
                *out_var_decl = ref;
            }
        }
    }
    else if (variable[0] != '_' && (*findRes)->owner == m_path)
    {
        if (std::find(m_global_declarations.begin(), m_global_declarations.end(), *findRes) == m_global_declarations.end())
        {
            m_global_declarations.push_back(*findRes);
        }
        if (out_var_decl)
        {
            *out_var_decl = *findRes;
        }
    }
    else if (node.kind == sqf::parser::sqf::bison::astkind::ASSIGNMENT_LOCAL || private_check)
    {
        analysis_raise_L0001(node, orig);
        private_check = true;
        if (out_var_decl)
        {
            *out_var_decl = *findRes;
        }
    }
    if (private_check)
    {
        if (variable[0] != '_')
        {
            analysis_raise_L0003(node, orig);
        }
    }
}

void text_document::analysis_params(sqf_language_server& language_server, sqf::runtime::runtime& sqfvm, sqf::parser::sqf::bison::astnode& current, size_t level, std::vector<variable_declaration::sptr>& known)
{
    if (current.children.empty()) { /* only handle if we have children */ return; }
    for (auto child : current.children)
    {
        if (child.kind == sqf::parser::sqf::bison::astkind::STRING)
        { // Simple parsing - Add variable
            auto variable = sqf::types::d_string::from_sqf(child.token.contents);
            if (variable != "")
            {
                analysis_ensure_L0001_L0003(language_server, known, level, child, variable, true, nullptr);
            }
        }
        else
        { // "Complex parsing" - Ensure correctness and add variable from first child
            if (child.children.size() < 2 || child.children.size() > 4)
            { // Ensure at least 2 children
                analysis_raise_L0006_array_size_missmatch(child, 2, 4, child.children.size());
            }
            else
            {
                if (child.children.size() >= 1 && child.children[0].kind != sqf::parser::sqf::bison::astkind::STRING)
                {
                    analysis_raise_L0007_type_error<1>(child.children[0], { t_string() }, {});
                }
                else
                {
                    auto variable = sqf::types::d_string::from_sqf(child.children[0].token.contents);
                    if (variable != "")
                    {
                        analysis_ensure_L0001_L0003(language_server, known, level, child.children[0], variable, true, nullptr);
                    }
                }
                if (child.children.size() >= 3 && child.children[2].kind != sqf::parser::sqf::bison::astkind::ARRAY)
                {
                    analysis_raise_L0007_type_error<1>(child.children[2], { t_array() }, {});
                }
                if (child.children.size() >= 4 &&
                    child.children[3].kind != sqf::parser::sqf::bison::astkind::ARRAY &&
                    child.children[3].kind != sqf::parser::sqf::bison::astkind::NUMBER &&
                    child.children[3].kind != sqf::parser::sqf::bison::astkind::HEXNUMBER)
                {
                    analysis_raise_L0007_type_error<2>(child.children[3], { t_string(), t_scalar() }, {});
                }
            }
        }
    }
}

void text_document::recalculate_analysis_helper(
    sqf_language_server& language_server,
    sqf::runtime::runtime& sqfvm,
    sqf::parser::sqf::bison::astnode& current,
    size_t level,
    std::vector<variable_declaration::sptr>& known,
    analysis_info parent_type,
    const std::vector<variable_declaration::sptr>& actual_globals
    )
{
    using sqf::parser::sqf::parser;
    using namespace sqf::parser::sqf::bison;
    m_asthints.push_back({ &current, current.token.offset, current.token.line, current.token.column });


    switch (current.kind)
    {
    /*
        Error Codes:
            - L-0001
            - L-0003
        Handles:
            - Duplicate-Declaration detection
            - Adding variables to the known-stack
    */
    case astkind::ASSIGNMENT_LOCAL:
    case astkind::ASSIGNMENT: {
        std::string variable(current.token.contents.begin(), current.token.contents.end());
        analysis_ensure_L0001_L0003(language_server, known, level, current, variable, false, nullptr);
        recalculate_analysis_helper_ident(language_server, sqfvm, current, level, known, analysis_info::NA, actual_globals);
        recalculate_analysis_helper(language_server, sqfvm, current.children[0], level, known, analysis_info::NA, actual_globals);
    } break;

    /*
        Handles:
            - Add built-in variables `_x`, `_foreachindex` of foreach
            - Add built-in variables `_x` of count
            - Cleanup of "known" variables after leaving code-block
    */
    case astkind::CODE: {
        switch (parent_type)
        {
        case text_document::analysis_info::DECLARE_FOREACHINDEX_AND_X:
            known.push_back(std::make_shared<variable_declaration>( level, current.token.line, current.token.column, "_foreachindex"));
            /* fallthrough */
        case text_document::analysis_info::DECLARE_X:
            known.push_back(std::make_shared<variable_declaration>(level, current.token.line, current.token.column, "_x"));
            break;
        }
        for (auto child : current.children)
        {
            recalculate_analysis_helper(language_server, sqfvm, child, level + 1, known, analysis_info::NA, actual_globals);
        }

        // Erase lower known variables
        for (size_t i = 0; i < known.size(); i++)
        {
            if (known[i]->level == level + 1)
            {
                known.at(i) = known.back();
                known.erase(known.begin() + known.size() - 1);
                i--;
            }
        }
    } break;

    /*
        Error Codes:
            - L-0002
        Handles:
            - Undefined variable warnings
            - Variable-Usage reference updating
    */
    case astkind::IDENT: {
        recalculate_analysis_helper_ident(language_server, sqfvm, current, level, known, parent_type, actual_globals);
    } goto l_default;

    /*
        Handles:
            - Passing analysis_info to lower method
            - Applying clean `known` vector for `spawn`
    */
    case astkind::EXP1:
    case astkind::EXP2:
    case astkind::EXP3:
    case astkind::EXP4:
    case astkind::EXP5:
    case astkind::EXP6:
    case astkind::EXP7:
    case astkind::EXP8:
    case astkind::EXP9:
    case astkind::EXP0: {
        auto op = std::string(current.token.contents);
        std::transform(op.begin(), op.end(), op.begin(), [](char& c) { return (char)std::tolower((int)c); });

        if (op == "spawn")
        {
            for (auto child : current.children)
            {
                std::vector<variable_declaration::sptr> known2 = language_server.global_declarations();
                known2.push_back(std::make_shared<variable_declaration>(0, 0, 0, "_this"));
                recalculate_analysis_helper(language_server, sqfvm, child, level + 1, known2, analysis_info::NA, actual_globals);
            }

            break;
        }
        else if (op == "params")
        {
            analysis_params(language_server, sqfvm, current.children[1], level, known);
            goto l_default;
        }
        else if (op == "foreach")
        {
            for (auto child : current.children)
            {
                recalculate_analysis_helper(language_server, sqfvm, child, level + 1, known, analysis_info::DECLARE_FOREACHINDEX_AND_X, actual_globals);
            }

            break;
        }
        else if (op == "count" || op == "select" || op == "apply" || op == "findif")
        {
            for (auto child : current.children)
            {
                recalculate_analysis_helper(language_server, sqfvm, child, level + 1, known, analysis_info::DECLARE_X, actual_globals);
            }

            break;
        }
        else
        {
            goto l_default;
        }
    }
    /*
        Handles:
        - Applying clean `known` vector for `spawn`
    */
    case astkind::EXPU: {
        auto op = std::string(current.token.contents);
        std::transform(op.begin(), op.end(), op.begin(), [](char& c) { return (char)std::tolower((int)c); });

        if (op == "spawn")
        {
            for (auto child : current.children)
            {
                std::vector<variable_declaration::sptr> known2 = language_server.global_declarations();
                known2.push_back(std::make_shared<variable_declaration>(0, 0, 0, "_this"));
                recalculate_analysis_helper(language_server, sqfvm, child, level + 1, known2, analysis_info::NA, actual_globals);
            }

            break;
        }
        else if (op == "private")
        {
            for (auto child : current.children)
            {
                recalculate_analysis_helper(language_server, sqfvm, child, level, known, analysis_info::PRIVATE, actual_globals);
            }

            break;
        }
        else if (op == "params")
        {
            analysis_params(language_server, sqfvm, current.children[0], level, known);
            goto l_default;
        }
        else if (op == "for" && current.children[0].kind == astkind::STRING)
        {
            auto variable = sqf::types::d_string::from_sqf(current.children[0].token.contents);
            auto var = std::make_shared<variable_declaration>(level, current.children[0].token.line, current.children[0].token.column, variable);
            known.push_back(var);
            if (var->variable[0] == '_') // safe as empty std string `0` is `\0`
            {
                m_private_declarations.push_back(var);
            }
            else
            {
                analysis_raise_L0003(current.children[0], var->variable);
            }
            goto l_default;
        }
        else
        {
            goto l_default;
        }
    }
    /*
        Error Codes:
            - L-0001
            - L-0003
        Handles:
            - On parent_type == PRIVATE:
            - Duplicate-Declaration detection
            - Adding variables to the known-stack
    */
    case astkind::STRING: {
        if (parent_type == analysis_info::PRIVATE)
        {
            auto variable = sqf::types::d_string::from_sqf(current.token.contents);
            analysis_ensure_L0001_L0003(language_server, known, level, current, variable, true, nullptr);
        }
    } break;

    l_default:
    default: {
        for (auto child : current.children)
        {
            recalculate_analysis_helper(language_server, sqfvm, child, level, known, parent_type, actual_globals);
        }
    } break;
    }
}

void text_document::recalculate_analysis_helper_ident(sqf_language_server& language_server, sqf::runtime::runtime& sqfvm, sqf::parser::sqf::bison::astnode& current, size_t level, std::vector<variable_declaration::sptr>& known, analysis_info parent_type, const std::vector<variable_declaration::sptr>& actual_globals)
{
    std::string variable(current.token.contents.begin(), current.token.contents.end());


    std::transform(variable.begin(), variable.end(), variable.begin(), [](char c) { return (char)std::tolower(c); });
    auto findRes = std::find_if(known.begin(), known.end(),
        [&variable](variable_declaration::sptr it) { return it->variable == variable; });
    if (findRes == known.end())
    {
        std::string content(current.token.contents.begin(), current.token.contents.end());
        analysis_raise_L0002(current, content);
    }
    else
    {
        if (variable[0] == '_') // safe as empty std string `0` is `\0`
        {
            auto it = std::find_if(m_private_declarations.begin(), m_private_declarations.end(),
                [&variable](variable_declaration::sptr it) -> bool { return it->variable == variable; });
            if (it != m_private_declarations.end())
            {
                (*it)->usages.push_back({ current.token.line, current.token.column });
            }
        }
        else
        {
            auto it = std::find_if(actual_globals.begin(), actual_globals.end(),
                [&variable](variable_declaration::sptr it) -> bool { return it->variable == variable; });
            if (it != actual_globals.end())
            {
                (*it)->usages.push_back({ current.token.line, current.token.column });
            }
        }
    }
}

void text_document::recalculate_analysis(sqf_language_server& language_server, sqf::runtime::runtime& sqfvm)
{
    m_private_declarations.clear();
    m_asthints.clear();
    std::vector<variable_declaration::sptr> actual = language_server.global_declarations();
    std::vector<variable_declaration::sptr> known = actual;
    known.push_back(std::make_shared<variable_declaration>(0, 0, 0, "_this" ));
    recalculate_analysis_helper(language_server, sqfvm, m_root_ast, 0, known, analysis_info::NA, actual);
}

void text_document::analyze(sqf_language_server& language_server, sqf::runtime::runtime& sqfvm, std::optional<std::string_view> contents_override)
{
    // Preparation
    bool had_diagnostics = !diagnostics.diagnostics.empty();
    diagnostics.diagnostics.clear();
    auto prev_gl = m_global_declarations;
    m_global_declarations.clear();

    if (type == document_type::SQF)
    {
        // Perform different analysis steps
        recalculate_ast(language_server, sqfvm, contents_override);
        recalculate_foldings(sqfvm);
        recalculate_analysis(language_server, sqfvm);
    }
    // calculate delta in the globals
    std::vector<variable_declaration::sptr> added;
    for (auto it : m_global_declarations)
    {
        auto res = std::find(prev_gl.begin(), prev_gl.end(), it);
        if (res == prev_gl.end())
        {
            added.push_back(it);
        }
    }
    std::vector<variable_declaration::sptr> removed;
    for (auto it : prev_gl)
    {
        auto res = std::find(m_global_declarations.begin(), m_global_declarations.end(), it);
        if (res == m_global_declarations.end())
        {
            removed.push_back(it);
        }
    }

    // Apply removal
    language_server.with_global_declarations_do(
        [&](auto& global_declarations) {
            for (auto it : removed)
            {
                auto res = std::find(global_declarations.begin(), global_declarations.end(), it);
                if (res != global_declarations.end())
                {
                    global_declarations.erase(res);
                }
            }

            // Apply additions
            for (auto it : added)
            {
                global_declarations.push_back(it);
            }
        });

    // Send diagnostics
    if (had_diagnostics || !diagnostics.diagnostics.empty())
    {
        language_server.textDocument_publishDiagnostics(diagnostics);
    }
}