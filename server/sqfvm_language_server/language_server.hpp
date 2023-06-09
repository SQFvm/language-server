#ifndef SQFVM_LANGUAGE_SERVER_LANGUAGE_SERVER_HPP
#define SQFVM_LANGUAGE_SERVER_LANGUAGE_SERVER_HPP
#include "sqfvm_factory.hpp"
#include "lsp/lspserver.hpp"
#include "git_sha1.h"
#include "analysis/analyzer.hpp"
#include "runtime/runtime.h"
#include "database/context.hpp"

#include <filesystem>
#include <vector>
#include <memory>

namespace sqfvm::language_server
{
    class language_server : public ::lsp::server
    {
        ::lsp::data::initialize_params m_client_params;
        std::filesystem::path m_folder;
        std::filesystem::path m_db_path;
        analysis::analyzer_factory m_analyzer_factory;
        std::shared_ptr<database::context> m_context;
        std::unordered_map<std::string, std::string> m_file_contents;
        sqfvm_factory m_sqfvm_factory;

        void delete_file(database::tables::t_file file);
        void analyse_file(const database::tables::t_file& file);
    protected:
        ::lsp::data::initialize_result on_initialize(const ::lsp::data::initialize_params& params) override
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
        void on_shutdown() override { }
        void after_initialize(const ::lsp::data::initialize_params& params) override;
        void on_workspace_didChangeConfiguration(const ::lsp::data::did_change_configuration_params& params) override;
        void on_textDocument_didChange(const ::lsp::data::did_change_text_document_params& params) override;



        std::optional<std::vector<::lsp::data::folding_range>> on_textDocument_foldingRange(const ::lsp::data::folding_range_params& params) override;
        std::optional<::lsp::data::completion_list> on_textDocument_completion(const ::lsp::data::completion_params& params) override;
    public:
        language_server();
    };
}
#endif // SQFVM_LANGUAGE_SERVER_LANGUAGE_SERVER_HPP