#pragma once
#include "lspserver.hpp"
#include "sqlite.hpp"
#include "git_sha1.h"


namespace sqfvm::lsp
{
    class lssqf : public ::lsp::server
    {
        sqlite::database m_db;
        ::lsp::data::initialize_params m_client_params;
    protected:
        virtual ::lsp::data::initialize_result on_initialize(const ::lsp::data::initialize_params& params) override
        {
            m_client_params = params;
            ::lsp::data::initialize_result res;
            res.serverInfo = ::lsp::data::initialize_result::server_info{};
            res.serverInfo->name = "SQF-VM Language Server";
            res.serverInfo->version = std::string(g_GIT_SHA1);
            res.capabilities.textDocumentSync = ::lsp::data::initialize_result::server_capabilities::text_document_sync_options{};
            res.capabilities.textDocumentSync->change = ::lsp::data::text_document_sync_kind::Full;
            res.capabilities.textDocumentSync->openClose = true;
            res.capabilities.textDocumentSync->save = ::lsp::data::initialize_result::server_capabilities::text_document_sync_options::SaveOptions{};
            res.capabilities.textDocumentSync->save->includeText = true;
            res.capabilities.textDocumentSync->willSave = false;
            res.capabilities.foldingRangeProvider = ::lsp::data::initialize_result::server_capabilities::folding_range_registration_options{};
            res.capabilities.foldingRangeProvider->documentSelector = ::lsp::data::document_filter{ };
            res.capabilities.foldingRangeProvider->documentSelector->language = "sqf";
            // res.capabilities.completionProvider = lsp::data::initialize_result::server_capabilities::completion_options{};

            return res;
        }
        virtual void on_shutdown() override { }
        virtual void after_initialize(const ::lsp::data::initialize_params& params) override;
        virtual void on_workspace_didChangeConfiguration(const ::lsp::data::did_change_configuration_params& params) override;
        virtual void on_textDocument_didChange(const ::lsp::data::did_change_text_document_params& params) override;



        virtual std::optional<std::vector<::lsp::data::folding_range>> on_textDocument_foldingRange(const ::lsp::data::folding_range_params& params) override;
        virtual std::optional<::lsp::data::completion_list> on_textDocument_completion(const ::lsp::data::completion_params& params);
    public:
        lssqf() { }
    };
}