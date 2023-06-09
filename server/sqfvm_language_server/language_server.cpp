#include "language_server.hpp"

#include "analysis/sqf_ast/sqf_ast_analyzer.hpp"


#include <string_view>
#include <unordered_map>
#include <vector>
#include <sstream>

using namespace std::string_view_literals;
using namespace sqlite_orm;

void sqfvm::language_server::language_server::after_initialize(const ::lsp::data::initialize_params &params) {
    auto root_uri_string = params.rootUri.has_value() ? params.rootUri.value().path() : "./"sv;
    std::filesystem::path uri(root_uri_string);
    uri = std::filesystem::absolute(uri);
    m_folder = uri / ".vscode"sv / "sqfvm-lsp";
    m_db_path = m_folder / "sqlite3.db";

    m_context = std::make_shared<database::context>(m_db_path);

    // Handle SQLite3 database
    if (!m_context->good()) {
        std::stringstream sstream;
        sstream << "Failed to open SQLite3 database '" << m_db_path << "': " << m_context->error() << ". Language server will not work.";
        window_logMessage(::lsp::data::message_type::Error, sstream.str());
        return;
    }
    else {
        std::stringstream sstream;
        sstream << "Opened SQLite3 database at '" << m_db_path << "'.";
        window_logMessage(::lsp::data::message_type::Log, sstream.str());
    }

    {
        std::stringstream sstream;
        sstream << "SQLITE migration report:\n";
        for (auto &it: m_context->sync_result()) {
            sstream << it.first << ": ";
            switch (it.second) {
                case sync_schema_result::new_table_created: sstream << "new_table_created\n"; break;
                case sync_schema_result::already_in_sync: sstream << "already_in_sync\n"; break;
                case sync_schema_result::old_columns_removed: sstream << "old_columns_removed\n"; break;
                case sync_schema_result::new_columns_added: sstream << "new_columns_added\n"; break;
                case sync_schema_result::new_columns_added_and_old_columns_removed: sstream << "new_columns_added_and_old_columns_removed\n"; break;
                case sync_schema_result::dropped_and_recreated: sstream << "dropped_and_recreated\n"; break;
            }
        }
        window_logMessage(::lsp::data::message_type::Log, sstream.str());
    }

    // Mark all files as deleted, so we can remove them later if they are not in the workspace anymore
    try {
        m_context->storage().update_all(set(c(&database::tables::t_file::is_deleted) = true));
    }
    catch (std::exception &e) {
        std::stringstream sstream;
        sstream << "Failed to perform pre-check (mark all files as deleted) with '" << e.what()
                << "'. Language server will not work.";
        window_logMessage(::lsp::data::message_type::Error, sstream.str());
        return;
    }

    auto runtime = m_sqfvm_factory.create([](auto &_) {}, *m_context);

    // Mark all files according to their state (deleted, outdated)
    for (auto &workspace_folder: params.workspace_folders.value()) {
        std::filesystem::path workspace_path(workspace_folder.uri.path());

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
                    file->is_deleted = false;
                    file->is_outdated = file->last_changed != timestamp;
                    file->last_changed = timestamp;
                    m_context->storage().update(file.value());
                } else {
                    database::tables::t_file f;
                    f.path = file_path.string();
                    f.last_changed = timestamp;
                    f.is_deleted = false;
                    f.is_outdated = true;
                    m_context->storage().insert(f);
                }
            } else if (file_path.filename() == "$PBOPREFIX$") {
                // ToDo: If modified, remove existing mapping and reparse as whole. (or message the user that a reload is required)
                // ToDo: If created, add mapping and reparse as whole. (or message the user that a reload is required)
                auto pboprefix_path = file_path.parent_path().string();
                auto pboprefix_contents_o = runtime->fileio().read_file_from_disk(file_path.string());
                if (pboprefix_contents_o.has_value()) {
                    auto pboprefix_contents = pboprefix_contents_o.value();
                    m_sqfvm_factory.add_mapping(pboprefix_path, pboprefix_contents);
                } else {
                    std::stringstream sstream;
                    sstream << "Failed to read " << pboprefix_path << ". Skipping.";
                    window_logMessage(lsp::data::message_type::Error, sstream.str());
                }
            }
        }
    }

    for (const auto &file: m_context->storage().get_all<database::tables::t_file>(
            where(c(&database::tables::t_file::is_deleted) == true))) {
        delete_file(file);
    }

    for (const auto &file: m_context->storage().get_all<database::tables::t_file>(
            where(c(&database::tables::t_file::is_outdated) == true))) {
        analyse_file(file);
    }
}

void sqfvm::language_server::language_server::on_workspace_didChangeConfiguration(
        const ::lsp::data::did_change_configuration_params &params) {
}

void sqfvm::language_server::language_server::on_textDocument_didChange(
        const ::lsp::data::did_change_text_document_params &params) {
}

std::optional<std::vector<::lsp::data::folding_range>>
sqfvm::language_server::language_server::on_textDocument_foldingRange(
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
                    auto db_path,
                    auto &factory,
                    auto file,
                    auto &text) -> std::unique_ptr<analysis::analyzer> {
                return std::make_unique<analysis::sqf_ast::sqf_ast_analyzer>(db_path, factory, file, text);
            });
}

void sqfvm::language_server::language_server::delete_file(sqfvm::language_server::database::tables::t_file file) {
    // Create analyzer
    auto extension = std::filesystem::path(file.path).extension().string();
    std::string content{};
    auto analyzer_opt = m_analyzer_factory.get(extension, m_context->db_path(), m_sqfvm_factory, file, content);
    if (!analyzer_opt.has_value()) {
        return;
    }
    analyzer_opt.value()->analyze();
    m_context->storage().remove<database::tables::t_file>(file.id_pk);
}

void
sqfvm::language_server::language_server::analyse_file(const sqfvm::language_server::database::tables::t_file &file) {
    auto runtime = m_sqfvm_factory.create([](auto &_) {}, *m_context);

    // Create analyzer
    auto extension = std::filesystem::path(file.path).extension().string();
    std::string content = runtime->fileio().read_file_from_disk(file.path).value_or("");
    auto analyzer_opt = m_analyzer_factory.get(extension, m_context->db_path(), m_sqfvm_factory, file, content);
    if (!analyzer_opt.has_value()) {
        return;
    }
    {
        std::stringstream sstream;
        sstream << "Analyzing '" << file.path << "'";
        window_logMessage(::lsp::data::message_type::Log, sstream.str());
    }
    try {
        analyzer_opt.value()->analyze();
        analyzer_opt.value()->commit();
    }
    catch (std::exception& e) {
        std::stringstream sstream;
        sstream << "Failed to analyze '" << file.path << "': " << e.what();
        window_logMessage(::lsp::data::message_type::Error, sstream.str());
    }
}
