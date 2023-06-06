#include "language_server.hpp"

#include "analysis/sqf/sqf_analyzer.hpp"


#include <string_view>
#include <unordered_map>
#include <vector>
#include <sstream>

using namespace std::string_view_literals;
using namespace sqlite_orm;

void sqfvm::language_server::language_server::after_initialize(const ::lsp::data::initialize_params &params) {
    std::filesystem::path uri(params.rootUri.has_value() ? params.rootUri.value().full() : "./"sv);
    uri = std::filesystem::absolute(uri);
    m_folder = uri / ".vscode"sv / "sqfvm-lsp";
    m_db_path = m_folder / "sqlite3.db";

    m_context = std::make_shared<database::context>(m_db_path);

    // Handle SQLite3 database
    if (!m_context->good()) {
        std::stringstream sstream;
        sstream << "Failed to open SQLite3 database '" << m_db_path << "'. Language server will not work.";
        window_logMessage(::lsp::data::message_type::Error, sstream.str());
        return;
    }

    // Mark all files as deleted, so we can remove them later if they are not in the workspace anymore
    m_context->storage().update_all(
            set(c(&database::tables::t_file::flags) = bitwise_or(
                    c(&database::tables::t_file::flags),
                    database::tables::t_file::file_flags::deleted)));

    // Mark all files according to their state (deleted, outdated)
    for (auto &workspace_folder: params.workspace_folders.value()) {
        std::filesystem::path workspace_path(workspace_folder.uri.full());

        // Iterate over all files recursive
        std::filesystem::recursive_directory_iterator
                iter(workspace_path, std::filesystem::directory_options::skip_permission_denied);
        std::filesystem::recursive_directory_iterator iter_end;
        for (; iter != iter_end; iter++) {
            auto &file_path = iter->path();
            if (m_analyzer_factory.has(file_path.extension().string())) {
                auto last_write_time = iter->last_write_time();
                uint64_t timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                        last_write_time.time_since_epoch())
                        .count();
                auto file = m_context->storage().get_optional<database::tables::t_file>(
                        columns(&database::tables::t_file::path),
                        where(is_equal(c(&database::tables::t_file::path), file_path.string())));
                if (file.has_value()) {
                    file->flags = file->flags & ~database::tables::t_file::file_flags::deleted;
                    m_context->storage().update(file.value());
                } else {
                    database::tables::t_file f;
                    f.path = file_path.string();
                    f.last_changed = timestamp;
                    f.flags = database::tables::t_file::file_flags::outdated;
                    m_context->storage().insert(f);
                }
            }
        }
    }

    for (auto file: m_context->storage().get_all<database::tables::t_file>(
            where(is_equal(c(&database::tables::t_file::flags),
                           database::tables::t_file::file_flags::deleted)))) {
        delete_file(file);
    }

    for (auto file: m_context->storage().get_all<database::tables::t_file>(
            where(is_equal(c(&database::tables::t_file::flags),
                           database::tables::t_file::file_flags::outdated)))) {
        analyse_file(file);
    }
}

void sqfvm::language_server::language_server::on_workspace_didChangeConfiguration(
        const ::lsp::data::did_change_configuration_params &params) {
}

void sqfvm::language_server::language_server::on_textDocument_didChange(
        const ::lsp::data::did_change_text_document_params &params) {
}

std::optional<std::vector<::lsp::data::folding_range>> sqfvm::language_server::language_server::on_textDocument_foldingRange(
        const ::lsp::data::folding_range_params &params) {
    return {};
}

std::optional<::lsp::data::completion_list> sqfvm::language_server::language_server::on_textDocument_completion(
        const ::lsp::data::completion_params &params) {
    return {};
}

sqfvm::language_server::language_server::language_server() {
    // Setup SQF-VM
    // Setup analyzers
    m_analyzer_factory.set(
            ".sqf", [](
                    auto file,
                    auto &text) -> std::unique_ptr<analysis::analyzer> {
                return std::make_unique<analysis::sqf::sqf_analyzer>(file, text);
            });
}

void sqfvm::language_server::language_server::delete_file(sqfvm::language_server::database::tables::t_file file) {

}

void sqfvm::language_server::language_server::analyse_file(sqfvm::language_server::database::tables::t_file file) {

}
