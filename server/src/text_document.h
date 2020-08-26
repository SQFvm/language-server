#pragma once
#include "variable_declaration.h"
#include "util.h"

#include <parser/sqf/default.h>

#include <string>
#include <vector>

class sqf_language_server;

class text_document
{
public:
    enum class document_type
    {
        NA,
        SQF
    };
    struct asthint
    {
        sqf::parser::sqf::impl_default::astnode* actual;
        size_t offset;
        size_t line;
        size_t column;
    };

    // Helper class to navigate an ast linear
    class astnav
    {
    private:
        size_t m_index;
        std::vector<asthint>& m_hints;
    public:
        astnav(size_t index, std::vector<asthint>& hints) :
            m_index(index),
            m_hints(hints)
        {

        }

        // Navigate to next asthint
        bool next()
        {
            m_index++;
            if (m_index >= m_hints.size())
            {
                m_index--;
                return false;
            }
            return true;
        }
        // Navigate to previous asthint
        bool previous()
        {
            if (m_index == 0)
            {
                return false;
            }
            m_index--;
            return true;
        }
        asthint& operator*() const { return m_hints[m_index]; }
        asthint* operator->() const { return &m_hints[m_index]; }
    };
private:
    enum class analysis_info
    {
        NA,
        DECLARE_FOREACHINDEX_AND_X,
        DECLARE_X,
        PRIVATE
    };
    std::string m_path;
    std::string m_contents;
    sqf::parser::sqf::impl_default::astnode m_root_ast;
    std::vector<lsp::data::folding_range> m_foldings;
    std::vector<asthint> m_asthints;

    std::vector<variable_declaration::sptr> m_private_declarations;
    std::vector<variable_declaration::sptr> m_global_declarations;

    void recalculate_ast(sqf_language_server& language_server, sqf::runtime::runtime& sqfvm, std::optional<std::string_view> contents_override);
    void recalculate_foldings_recursive(sqf::runtime::runtime& sqfvm, sqf::parser::sqf::impl_default::astnode& current)
    {
        switch (current.kind)
        {
        case sqf::parser::sqf::impl_default::nodetype::ARRAY:
        case sqf::parser::sqf::impl_default::nodetype::CODE:
        {
            // todo: find a way to force vscode into using offsets instead of lines
            lsp::data::folding_range frange;
            frange.startCharacter = current.file_offset;
            frange.startLine = current.line - 1; // lines start at 0
            frange.endCharacter = current.file_offset + current.length;

            // find current nodes, last child in tree and set its line as end.
            sqf::parser::sqf::impl_default::astnode* prev = &current;
            sqf::parser::sqf::impl_default::astnode* node = &current;
            while (node)
            {
                prev = node;
                node = node->children.empty() ? nullptr : &node->children.back();
            }
            frange.endLine = prev->line;
            m_foldings.push_back(frange);
        } break;
        }
        for (auto child : current.children)
        {
            recalculate_foldings_recursive(sqfvm, child);
        }
    }
    void recalculate_foldings(sqf::runtime::runtime& sqfvm)
    {
        m_foldings.clear();
        recalculate_foldings_recursive(sqfvm, m_root_ast);
    }
    void analysis_raise_L0001(sqf::parser::sqf::impl_default::astnode& node, const std::string& variable)
    {
        lsp::data::diagnostics diag;
        diag.code = "L-0001";
        diag.range.start.line = node.line - 1;
        diag.range.start.character = node.column;
        diag.range.end.line = node.line - 1;
        diag.range.end.character = node.column;
        diag.message = "'" + variable + "' hides previous declaration.";
        diag.severity = lsp::data::diagnostic_severity::Warning;
        diag.source = "SQF-VM LS";
        diagnostics.diagnostics.push_back(diag);
    }
    void analysis_raise_L0002(sqf::parser::sqf::impl_default::astnode& node, const std::string& variable)
    {
        lsp::data::diagnostics diag;
        diag.code = "L-0002";
        diag.range.start.line = node.line - 1;
        diag.range.start.character = node.column;
        diag.range.end.line = node.line - 1;
        diag.range.end.character = node.column;
        diag.message = "Variable '" + variable + "' not defined.";
        diag.severity = lsp::data::diagnostic_severity::Warning;
        diag.source = "SQF-VM LS";
        diagnostics.diagnostics.push_back(diag);
    }
    void analysis_raise_L0003(sqf::parser::sqf::impl_default::astnode& node, const std::string& variable)
    {
        lsp::data::diagnostics diag;
        diag.code = "L-0003";
        diag.range.start.line = node.line - 1;
        diag.range.start.character = node.column;
        diag.range.end.line = node.line - 1;
        diag.range.end.character = node.column;
        diag.message = "'" + variable + "' is not starting with an underscore ('_').";
        diag.severity = lsp::data::diagnostic_severity::Error;
        diag.source = "SQF-VM LS";
        diagnostics.diagnostics.push_back(diag);
    }

    // Performs the checks for L-0001 & L-0003.
    // 
    // @param known:            The list of known variable as reference
    // @param level:            The current variable level
    // @param node:             The actual variable node, handled
    // @param variable:         The variable name (should be tolowered first)
    // @param private_check:    Wether the variable should be treated as private declaration
    void analysis_ensure_L0001_L0003(
        sqf_language_server& language_server, std::vector<variable_declaration::sptr>& known,
        size_t level, sqf::parser::sqf::impl_default::astnode& node, const std::string& orig, bool private_check, variable_declaration::sptr* out_var_decl);
    void recalculate_analysis_helper(sqf_language_server& language_server, sqf::runtime::runtime& sqfvm, sqf::parser::sqf::impl_default::astnode& current, size_t level, std::vector<variable_declaration::sptr>& known, analysis_info parent_type);
    void recalculate_analysis(sqf_language_server& language_server, sqf::runtime::runtime& sqfvm);
public:
    lsp::data::publish_diagnostics_params diagnostics;
    document_type type;
    text_document() {}
    text_document(sqf_language_server& language_server, sqf::runtime::runtime& sqfvm, std::string path, document_type type) : m_path(path), type(type)
    {
        diagnostics.uri = sanitize_to_uri(path);
    }

    std::string_view contents() const { return m_contents; }
    void analyze(sqf_language_server& language_server, sqf::runtime::runtime& sqfvm, std::optional<std::string_view> contents_override);

    std::vector<lsp::data::folding_range>& foldings() { return m_foldings; }

    // Finds the closest astnode to the position provided and gives it back.
    // Might return empty astnav if line is not existing, no astnode exists on that line or
    // the file failed to parse
    std::optional<astnav> navigate(size_t line, size_t column)
    {
        size_t i;

        // Look for the perfect match, linewise
        for (i = 0; i < m_asthints.size(); i++)
        {
            if (m_asthints[i].line == line)
            {
                // Found match. Break loop.
                break;
            }
            if (m_asthints[i].line > line)
            {
                // Current line > targeted line. Match not found.
                return {};
            }
        }
        if (i == m_asthints.size())
        {
            return {};
        }

        // Look for the best match, columnwise (prioritize left)
        size_t j;
        for (j = i; j < m_asthints.size() && m_asthints[j].line == line; j++)
        {
            if (m_asthints[j].column == line)
            {
                // Perfect match. Return immediate
                return astnav(j, m_asthints);
            }
            if (m_asthints[j].column > column)
            { // Astnodes column > lookup column. Break loop.
                break;
            }
        }
        return astnav(j - 1, m_asthints);
    }
};