#include "languageserver.h"


#include <runtime/runtime.h>
#include <runtime/d_string.h>
#include <operators/ops_config.h>
#include <operators/ops_diag.h>
#include <operators/ops_generic.h>
#include <operators/ops_group.h>
#include <operators/ops_logic.h>
#include <operators/ops_markers.h>
#include <operators/ops_math.h>
#include <operators/ops_namespace.h>
#include <operators/ops_object.h>
#include <operators/ops_sqfvm.h>
#include <operators/ops_string.h>

#include <parser/config/default.h>
#include <parser/sqf/default.h>
#include <parser/preprocessor/default.h>
#include <fileio/default.h>

#include <unordered_map>
#include <filesystem>

class sqf_language_server : public lsp::server
{
public:
    class QueueLogger : public Logger {
        sqf_language_server& language_server;
    public:
        QueueLogger(sqf_language_server& ref) : Logger(), language_server(ref) {}

        virtual void log(const LogMessageBase& base) override
        {
            auto& message = dynamic_cast<const logmessage::RuntimeLogMessageBase&>(base);
            if (message == nullptr)
            {
                return;
            }

            auto location = message.location();
            auto findRes = language_server.text_documents.find(location.path);
            if (findRes == language_server.text_documents.end())
            {
                return;
            }

            lsp::data::publish_diagnostics_params& params = findRes->second.diagnostics;

            lsp::data::diagnostics msg;
            msg.range.start.line = location.line - 1;
            msg.range.start.character = location.col;
            msg.range.end.line = location.line - 1;
            msg.range.end.character = location.col;
            
            
            msg.code = std::to_string(base.getErrorCode());
            msg.message = message.formatMessage();
            msg.source = "SQF-VM";

            switch (message.getLevel())
            {
            case loglevel::fatal:
            case loglevel::error:
                msg.severity = lsp::data::diagnostic_severity::Error;
                break;
            case loglevel::warning:
                msg.severity = lsp::data::diagnostic_severity::Warning;
                break;
            case loglevel::info:
                msg.severity = lsp::data::diagnostic_severity::Information;
                break;
            case loglevel::verbose:
            case loglevel::trace:
            default:
                msg.severity = lsp::data::diagnostic_severity::Hint;
                break;
            }
            params.diagnostics.push_back(msg);
        }
    };
    class text_document
    {
    private:
        struct variable_declaration
        {
            int level;
            std::string variable;
        };
        std::string m_path;
        std::string m_contents;
        sqf::parser::sqf::impl_default::astnode m_root_ast;
        std::vector<lsp::data::folding_range> m_foldings;

        void recalculate_ast(sqf_language_server& language_server, sqf::runtime::runtime& sqfvm, std::string_view contents)
        {
            auto parser = dynamic_cast<sqf::parser::sqf::impl_default&>(sqfvm.parser_sqf());
            bool errflag = false;
            auto preprocessed = contents.empty() ?
                sqfvm.parser_preprocessor().preprocess(sqfvm, { m_path, {} }) :
                sqfvm.parser_preprocessor().preprocess(sqfvm, contents, { m_path, {} });
            if (preprocessed.has_value())
            {
                m_contents = preprocessed.value();
                m_root_ast = parser.get_ast(sqfvm, m_contents, { m_path, {} }, &errflag);
                if (errflag)
                {
                    m_root_ast = {};
                }
            }
            else
            {
                m_root_ast = {};
            }
        }

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
        void recalculate_analysis_helper(
            sqf::runtime::runtime& sqfvm,
            sqf::parser::sqf::impl_default::astnode& current,
            int layer,
            std::vector<variable_declaration>& known)
        {
            switch (current.kind)
            {
            case sqf::parser::sqf::impl_default::nodetype::ASSIGNMENTLOCAL:
            case sqf::parser::sqf::impl_default::nodetype::ASSIGNMENT: {
                auto variable = current.children[0].content;
                auto findRes = std::find_if(known.begin(), known.end(),
                    [&variable](variable_declaration& it) { return it.variable == variable; });
                if (findRes == known.end())
                {
                    known.push_back({ layer, variable });
                }
                else if (current.kind == sqf::parser::sqf::impl_default::nodetype::ASSIGNMENTLOCAL)
                {
                    lsp::data::diagnostics diag;
                    diag.code = "L-0001";
                    diag.range.start.line = current.line - 1;
                    diag.range.start.character = current.column;
                    diag.range.end.line = current.line - 1;
                    diag.range.end.character = current.column;
                    diag.message = "'" + variable + "' hides previous declaration.";
                    diag.severity = lsp::data::diagnostic_severity::Warning;
                    diag.source = "SQF-VM LS";
                    diagnostics.diagnostics.push_back(diag);
                }
                recalculate_analysis_helper(sqfvm, current.children[1], layer + 1, known);
            } break;
            case sqf::parser::sqf::impl_default::nodetype::CODE: {
                for (auto child : current.children)
                {
                    recalculate_analysis_helper(sqfvm, child, layer + 1, known);
                }

                // Erase lower known variables
                for (size_t i = 0; i < known.size(); i++)
                {
                    if (known[i].level == layer + 1)
                    {
                        known.at(i) = known.back();
                        known.erase(known.begin() + known.size() - 1);
                        i--;
                    }
                }
            } break;
            case sqf::parser::sqf::impl_default::nodetype::VARIABLE: {
                auto findRes = std::find_if(known.begin(), known.end(),
                    [&current](variable_declaration& it) { return it.variable == current.content; });
                if (findRes == known.end())
                {
                    lsp::data::diagnostics diag;
                    diag.code = "L-0002";
                    diag.range.start.line = current.line - 1;
                    diag.range.start.character = current.column;
                    diag.range.end.line = current.line - 1;
                    diag.range.end.character = current.column;
                    diag.message = "Variable '" + current.content + "' not defined.";
                    diag.severity = lsp::data::diagnostic_severity::Warning;
                    diag.source = "SQF-VM LS";
                    diagnostics.diagnostics.push_back(diag);
                }
            } goto l_default; /* L-0002 */
            case sqf::parser::sqf::impl_default::nodetype::BEXP1:
            case sqf::parser::sqf::impl_default::nodetype::BEXP2:
            case sqf::parser::sqf::impl_default::nodetype::BEXP3:
            case sqf::parser::sqf::impl_default::nodetype::BEXP4:
            case sqf::parser::sqf::impl_default::nodetype::BEXP5:
            case sqf::parser::sqf::impl_default::nodetype::BEXP6:
            case sqf::parser::sqf::impl_default::nodetype::BEXP7:
            case sqf::parser::sqf::impl_default::nodetype::BEXP8:
            case sqf::parser::sqf::impl_default::nodetype::BEXP9:
            case sqf::parser::sqf::impl_default::nodetype::BEXP10:
            case sqf::parser::sqf::impl_default::nodetype::BINARYEXPRESSION: {
                auto op = std::string(current.children[1].content);
                std::transform(op.begin(), op.end(), op.begin(), [](char& c) { return (char)std::tolower((int)c); });

                if (op == "spawn")
                {
                    for (auto child : current.children)
                    {
                        std::vector<variable_declaration> known2;
                        recalculate_analysis_helper(sqfvm, child, layer + 1, known2);
                    }

                    break;
                }
                else
                {
                    goto l_default;
                }
            }
            case sqf::parser::sqf::impl_default::nodetype::UNARYEXPRESSION: {
                auto op = std::string(current.children[0].content);
                std::transform(op.begin(), op.end(), op.begin(), [](char& c) { return (char)std::tolower((int)c); });

                if (op == "spawn")
                {
                    for (auto child : current.children)
                    {
                        std::vector<variable_declaration> known2;
                        recalculate_analysis_helper(sqfvm, child, layer + 1, known2);
                    }

                    break;
                }
                else if (op == "for" && current.children[1].kind == sqf::parser::sqf::impl_default::nodetype::STRING)
                {
                    auto variable = sqf::types::d_string::from_sqf(current.children[1].content);
                    known.push_back({ layer, variable });
                    goto l_default;
                }
                else
                {
                    goto l_default;
                }
            }

            l_default:
            default: {
                for (auto child : current.children)
                {
                    recalculate_analysis_helper(sqfvm, child, layer, known);
                }
            } break;
            }
        }
        void recalculate_analysis(sqf::runtime::runtime& sqfvm)
        {
            m_foldings.clear();
            std::vector<variable_declaration> known;
            recalculate_analysis_helper(sqfvm, m_root_ast, 0, known);
        }
    public:
        lsp::data::publish_diagnostics_params diagnostics;
        text_document() {}
        text_document(sqf_language_server& language_server, sqf::runtime::runtime& sqfvm, std::string path) : m_path(path)
        {
            reread(language_server, sqfvm, {});
            diagnostics.uri = sanitize(path);
        }

        std::string_view contents() const { return m_contents; }
        void reread(sqf_language_server& language_server, sqf::runtime::runtime& sqfvm, std::string_view contents)
        {
            bool had_diagnostics = !diagnostics.diagnostics.empty();
            diagnostics.diagnostics.clear();
            recalculate_ast(language_server, sqfvm, contents);
            recalculate_foldings(sqfvm);
            recalculate_analysis(sqfvm);
            if (had_diagnostics || !diagnostics.diagnostics.empty())
            {
                language_server.textDocument_publishDiagnostics(diagnostics);
            }
        }

        std::vector<lsp::data::folding_range>& foldings() { return m_foldings; }
    };
protected:

    // Inherited via server
    virtual lsp::data::initialize_result on_initialize(const lsp::data::initialize_params& params) override
    {
        client = params;
        // Prepare server capabilities
        lsp::data::initialize_result res;
        res.serverInfo = lsp::data::initialize_result::server_info{};
        res.serverInfo->name = "SQF-VM Language Server";
        res.serverInfo->version = "0.1.0";
        res.capabilities.textDocumentSync = lsp::data::initialize_result::server_capabilities::text_document_sync_options{};
        res.capabilities.textDocumentSync->change = lsp::data::text_document_sync_kind::Full;
        res.capabilities.textDocumentSync->openClose = true;
        res.capabilities.textDocumentSync->save = lsp::data::initialize_result::server_capabilities::text_document_sync_options::SaveOptions{};
        res.capabilities.textDocumentSync->save->includeText = true;
        res.capabilities.textDocumentSync->willSave = true;
        res.capabilities.foldingRangeProvider = lsp::data::initialize_result::server_capabilities::folding_range_registration_options{};
        res.capabilities.foldingRangeProvider->documentSelector = lsp::data::document_filter{ };
        res.capabilities.foldingRangeProvider->documentSelector->language = "sqf";
        res.capabilities.completionProvider = lsp::data::initialize_result::server_capabilities::completion_options{};

        // prepare sqfvm
        sqfvm.fileio(std::make_unique<sqf::fileio::impl_default>());
        sqfvm.parser_config(std::make_unique<sqf::parser::config::impl_default>(logger));
        sqfvm.parser_preprocessor(std::make_unique<sqf::parser::preprocessor::impl_default>(logger));
        sqfvm.parser_sqf(std::make_unique<sqf::parser::sqf::impl_default>(logger));
        sqf::operators::ops_config(sqfvm);
        sqf::operators::ops_diag(sqfvm);
        sqf::operators::ops_generic(sqfvm);
        sqf::operators::ops_group(sqfvm);
        sqf::operators::ops_logic(sqfvm);
        sqf::operators::ops_markers(sqfvm);
        sqf::operators::ops_math(sqfvm);
        sqf::operators::ops_namespace(sqfvm);
        sqf::operators::ops_object(sqfvm);
        sqf::operators::ops_sqfvm(sqfvm);
        sqf::operators::ops_string(sqfvm);

        return res;
    }
    virtual void on_shutdown() override {}

    // After Initialize
    virtual void after_initialize(const lsp::data::initialize_params& params) override
    {
        if (params.workspaceFolders.has_value())
        {
            for (auto workspaceFolder : params.workspaceFolders.value())
            {
                std::filesystem::recursive_directory_iterator dir_start(sanitize(workspaceFolder.uri), std::filesystem::directory_options::skip_permission_denied);
                std::filesystem::recursive_directory_iterator dir_end;

                for (auto it = dir_start; it != dir_end; it++)
                {
                    if (it->is_directory())
                    {
                        continue;
                    }
                    auto fpath = sanitize(workspaceFolder.uri);
                    text_documents[fpath] = { *this, sqfvm, fpath };
                }
            }
        }
    }

    virtual void on_textDocument_didChange(const lsp::data::did_change_text_document_params& params) override
    {
        auto findRes = text_documents.find(sanitize(params.textDocument.uri));
        if (findRes != text_documents.end())
        {
            auto& doc = findRes->second;
            doc.reread(*this, sqfvm, params.contentChanges.front().text);
        }
        else
        {
            auto fpath = sanitize(params.textDocument.uri);
            text_documents[fpath] = { *this, sqfvm, fpath };
        }
    }
    virtual std::optional<std::vector<lsp::data::folding_range>> on_textDocument_foldingRange(const lsp::data::folding_range_params& params) override
    {
        auto findRes = text_documents.find(sanitize(params.textDocument.uri));
        if (findRes != text_documents.end())
        {
            auto& doc = findRes->second;
            return doc.foldings();
        }
        else
        {
            return {};
        }
    }

public:
    std::unordered_map<std::string, text_document> text_documents;
    lsp::data::initialize_params client;
    QueueLogger logger;
    sqf::runtime::runtime sqfvm;
    sqf_language_server() : logger(*this), sqfvm(logger, {}) {}
    sqf_language_server(const sqf_language_server& copy) = delete;

    // Method to get a clear & clean uri string out of the uri provided by vscode.
    static std::string sanitize(const lsp::data::uri& uri)
    {
        std::string dpath;
        dpath.reserve(uri.host().length() + 1 + uri.path().length());
        dpath.append(uri.host());
        dpath.append("/");
        dpath.append(uri.path());
        std::filesystem::path data_path(dpath);
        data_path = data_path.lexically_normal();
        dpath = data_path.string();
        std::replace(dpath.begin(), dpath.end(), '\\', '/');
        return dpath;
    }
    // Method to get a clear & clean uri out of the string provided by sqfvm.
    static lsp::data::uri sanitize(const std::string str) { return sanitize(std::string_view(str)); }
    // Method to get a clear & clean uri out of the string provided by sqfvm.
    static lsp::data::uri sanitize(const std::string_view sv)
    {
        auto path = std::filesystem::path(sv).lexically_normal();
        auto str = "file:///" + path.string();
        std::replace(str.begin(), str.end(), '\\', '/');
        return lsp::data::uri(str);
    }
};

int main(int argc, char** argv)
{
#ifdef _DEBUG
    _CrtDbgReport(_CRT_ASSERT, "", 0, "", "Waiting for vs.");
#endif // _DEBUG

    // x39::uri a("aba://aba:aba@aba:aba/aba?aba#aba");
    // x39::uri b("aba://aba:aba@aba:aba?aba#aba");
    // x39::uri c("aba://aba:aba@aba?aba#aba");
    // x39::uri d("aba://aba@aba?aba#aba");
    // x39::uri e("aba://aba?aba#aba");
    // x39::uri f("aba://aba?aba");
    // x39::uri g("aba://aba");
    // x39::uri h("file://D%3A/Git/Sqfvm/vm/tests/");
    // x39::uri i("https://www.google.com/search?rlz=1C1CHBF_deDE910DE910&sxsrf=ALeKk02J_XcmnGpP0UfYPa2S-usVtUnZXw%3A1597937338384&ei=upY-X4TzFpHikgWc7pXwBQ&q=file%3A%2F%2F%2FD%3A%2Fasdasd");
    sqf_language_server ls;
    ls.listen();
}