#include "sqf_language_server.h"

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

void sqf_language_server::after_initialize(const lsp::data::initialize_params& params)
{
    // Prepare sqfvm
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

    // Setup Pathing & Parse every file inside the workspace
    if (params.workspaceFolders.has_value())
    {
        for (auto workspaceFolder : params.workspaceFolders.value())
        {
            auto workspacePath = sanitize(workspaceFolder.uri);
            sqfvm.fileio().add_mapping(workspacePath, "/");

            std::filesystem::recursive_directory_iterator dir_start(workspacePath, std::filesystem::directory_options::skip_permission_denied);
            std::filesystem::recursive_directory_iterator dir_end;

            // ToDo: Make parsing async
            for (auto it = dir_start; it != dir_end; it++)
            {
                if (it->is_directory())
                {
                    continue;
                }
                auto path = it->path();
                if (!path.has_extension())
                {
                    if (path.filename() == "$PBOPREFIX$")
                    {
                        // ToDo: If modified, remove existing mapping and reparse as whole. (or message the user that a reload is required)
                        // ToDo: If created, add mapping and reparse as whole. (or message the user that a reload is required)
                        auto pboprefix_path = path.parent_path().string();
                        auto pboprefix_contents_o = sqfvm.fileio().read_file_from_disk(path.string());
                        auto pboprefix_contents = pboprefix_contents_o.value();
                        sqfvm.fileio().add_mapping(pboprefix_path, pboprefix_contents[0] != '/' ? "/" + pboprefix_contents : pboprefix_contents);
                    }
                }
                else if (path.extension() == ".sqf")
                {
                    auto uri = sanitize(path.string());
                    auto fpath = sanitize(uri);
                    auto& doc = (text_documents[fpath] = { *this, sqfvm, fpath, text_document::document_type::SQF });
                    doc.analyze(*this, sqfvm, {});
                }
            }
        }
    }
}

void sqf_language_server::on_textDocument_didChange(const lsp::data::did_change_text_document_params& params)
{
    using namespace std::string_view_literals;
    auto fpath = sanitize(params.textDocument.uri);

    // Check if file already exists
    auto findRes = text_documents.find(fpath);
    if (findRes != text_documents.end())
    {
        // Only perform analysis again on the text provided by params
        auto& doc = findRes->second;
        doc.analyze(*this, sqfvm, params.contentChanges.front().text);
    }
    else if (fpath.length() > 4 && std::string_view(fpath.data() + fpath.length() - 4, 4) == ".sqf"sv)
    {
        // Create file at fpath only if file is an actual sqf file and perform analysis.
        auto& doc = (text_documents[fpath] = { *this, sqfvm, fpath, text_document::document_type::SQF });
        doc.analyze(*this, sqfvm, params.contentChanges.front().text);
    }
}

std::optional<std::vector<lsp::data::folding_range>> sqf_language_server::on_textDocument_foldingRange(const lsp::data::folding_range_params& params)
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