#ifndef SQFVM_LANGUAGE_SERVER_LANGUAGE_SERVER_HPP
#define SQFVM_LANGUAGE_SERVER_LANGUAGE_SERVER_HPP

#include "sqfvm_factory.hpp"
#include "lsp/server.hpp"
#include "git_sha1.h"
#include "analysis/analyzer.hpp"
#include "runtime/runtime.h"
#include "database/context.hpp"
#include "file_system_watcher.hpp"

#include <Poco/DirectoryWatcher.h>
#include <filesystem>
#include <vector>
#include <memory>
#include <sstream>
#include <functional>

namespace sqfvm::language_server {
    class language_server : public ::lsp::server {
        ::lsp::data::initialize_params m_client_params;
        std::filesystem::path m_lsp_folder;
        std::filesystem::path m_db_path;
        analysis::analyzer_factory m_analyzer_factory;
        std::shared_ptr<database::context> m_context;
        std::unordered_map<std::string, std::string> m_file_contents;
        sqfvm_factory m_sqfvm_factory;
        file_system_watcher m_file_system_watcher;

        bool delete_file(const std::filesystem::path &file);

        void delete_file(database::tables::t_file file);

        void mark_related_files_as_outdated(const sqfvm::language_server::database::tables::t_file &file);

        void analyse_file(const database::tables::t_file &file);

        void publish_diagnostics(const database::tables::t_file &file, bool publish_sub_files = true);

        void analyze_outdated_files();

        void push_file_history(
                const ::sqfvm::language_server::database::tables::t_file &file,
                std::string contents,
                bool is_external = false);

        void file_system_item_added(const std::filesystem::path &path,
                                    bool is_directory);

        void file_system_item_removed(const std::filesystem::path &path,
                                      bool is_directory);

        void file_system_item_modified(const std::filesystem::path &path,
                                       bool is_directory);

        [[nodiscard]] std::string add_or_update_pboprefix_mapping_safe(const std::filesystem::path &);

        void mark_all_files_as_outdated();
        void add_or_update_pboprefix_mapping_logging(const std::filesystem::path &);
        void remove_pboprefix_mapping(const std::filesystem::path &);

        std::optional<database::tables::t_file> get_file_from_path(
                const std::filesystem::path &path,
                bool create_if_not_exists = false);

        void window_log(
                ::lsp::data::message_type type,
                const std::function<void(std::stringstream &sstream)> &func) {
            std::stringstream sstream;
            func(sstream);
            window_logMessage(type, sstream.str());
        }

    protected:
        ::lsp::data::initialize_result on_initialize(const ::lsp::data::initialize_params &params) override;

        void on_shutdown() override {}

        void after_initialize(const ::lsp::data::initialize_params &params) override;

        void on_workspace_didChangeConfiguration(const ::lsp::data::did_change_configuration_params &params) override;

        void on_textDocument_didChange(const ::lsp::data::did_change_text_document_params &params) override;

        std::optional<std::vector<lsp::data::location>>
        on_textDocument_references(const lsp::data::references_params &params) override;


        std::optional<std::vector<::lsp::data::folding_range>>
        on_textDocument_foldingRange(const ::lsp::data::folding_range_params &params) override;

        std::optional<::lsp::data::completion_list>
        on_textDocument_completion(const ::lsp::data::completion_params &params) override;

        std::optional<std::vector<std::variant<lsp::data::command, lsp::data::code_action>>>
        on_textDocument_codeAction(const lsp::data::code_action_params &params) override;

    public:
        language_server();

        void ensure_git_ignore_file_exists();

        void log_sqlite_migration_report();

        void mark_file_as_outdated(const std::filesystem::path &path);
    };
}
#endif // SQFVM_LANGUAGE_SERVER_LANGUAGE_SERVER_HPP