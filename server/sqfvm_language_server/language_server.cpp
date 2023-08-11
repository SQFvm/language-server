#include "language_server.hpp"

#include "analysis/sqf_ast/sqf_ast_analyzer.hpp"


#include <Poco/Delegate.h>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <set>
#include <sstream>

using namespace std::string_view_literals;
using namespace sqlite_orm;

void sqfvm::language_server::language_server::after_initialize(const ::lsp::data::initialize_params &params) {
    auto root_uri_string = params.rootUri.has_value() ? params.rootUri.value().path() : "./"sv;
    std::filesystem::path uri(root_uri_string);
    uri = std::filesystem::absolute(uri).lexically_normal();
    m_lsp_folder = uri / ".vscode"sv / "sqfvm-lsp";
    m_db_path = m_lsp_folder / "sqlite3.db";
    // ToDo: Start file system watcher to detect changes in the workspace not triggered by the language server

    ensure_git_ignore_file_exists();

    m_context = std::make_shared<database::context>(m_db_path);
    m_sqfvm_factory.add_mapping(uri.string(), "");
    m_directory_watcher = std::make_shared<Poco::DirectoryWatcher>(
            uri.string(),
            Poco::DirectoryWatcher::DW_FILTER_ENABLE_ALL,
            2048);
    m_directory_watcher->itemAdded += Poco::delegate(
            this,
            &sqfvm::language_server::language_server::file_system_item_added);
    m_directory_watcher->itemRemoved += Poco::delegate(
            this,
            &sqfvm::language_server::language_server::file_system_item_removed);
    m_directory_watcher->itemModified += Poco::delegate(
            this,
            &sqfvm::language_server::language_server::file_system_item_modified);

    // Handle SQLite3 database
    if (!m_context->good()) {
        std::stringstream sstream;
        sstream << "Failed to open SQLite3 database '" << m_db_path << "': " << m_context->error()
                << ". Language server will not work.";
        window_logMessage(::lsp::data::message_type::Error, sstream.str());
        return;
    } else {
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
                case sync_schema_result::new_table_created:
                    sstream << "new_table_created\n";
                    break;
                case sync_schema_result::already_in_sync:
                    sstream << "already_in_sync\n";
                    break;
                case sync_schema_result::old_columns_removed:
                    sstream << "old_columns_removed\n";
                    break;
                case sync_schema_result::new_columns_added:
                    sstream << "new_columns_added\n";
                    break;
                case sync_schema_result::new_columns_added_and_old_columns_removed:
                    sstream << "new_columns_added_and_old_columns_removed\n";
                    break;
                case sync_schema_result::dropped_and_recreated:
                    sstream << "dropped_and_recreated\n";
                    break;
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
            auto file_path = iter->path().lexically_normal();
            if (m_analyzer_factory.has(file_path.extension().string())) {
                auto last_write_time = iter->last_write_time();
                uint64_t timestamp = (uint64_t) std::chrono::duration_cast<std::chrono::milliseconds>(
                        last_write_time.time_since_epoch())
                        .count();
                std::optional<database::tables::t_file> file;
                try {
                    auto file_results = m_context->storage().get_all<database::tables::t_file>(
                            where(c(&database::tables::t_file::path) == file_path.string()));
                    file = file_results.empty() ? std::nullopt : std::optional(file_results[0]);
                }
                catch (std::exception &e) {
                    std::stringstream sstream;
                    sstream << "Failed to find file '" << file_path.string() << "' with '" << e.what()
                            << "'. Language server will not work.";
                    window_logMessage(::lsp::data::message_type::Error, sstream.str());
                    return;
                }
                if (file.has_value()) {
                    file->is_deleted = false;
                    file->is_outdated = file->is_outdated || file->last_changed < timestamp;
                    file->last_changed = timestamp;
                    try {
                        m_context->storage().update(file.value());
                    }
                    catch (std::exception &e) {
                        std::stringstream sstream;
                        sstream << "Failed to update file '" << file_path.string() << "' with '" << e.what()
                                << "'. Language server will not work.";
                        window_logMessage(::lsp::data::message_type::Error, sstream.str());
                        return;
                    }
                    if (file->is_outdated) {
                        auto file_contents = sqf::fileio::passthrough::read_file_from_disk(file_path.string());
                        if (file_contents.has_value())
                            push_file_history(*file, *file_contents, true);
                    }
                } else {
                    database::tables::t_file f;
                    f.path = file_path.string();
                    f.last_changed = timestamp;
                    f.is_deleted = false;
                    f.is_outdated = true;
                    try {
                        m_context->storage().insert(f);
                    }
                    catch (std::exception &e) {
                        std::stringstream sstream;
                        sstream << "Failed to insert file '" << f.path << "' with '" << e.what()
                                << "'. Language server will not work.";
                        window_logMessage(::lsp::data::message_type::Error, sstream.str());
                        return;
                    }
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

    try {
        for (const auto &file: m_context->storage().get_all<database::tables::t_file>(
                where(c(&database::tables::t_file::is_deleted) == true))) {
            delete_file(file);
        }
    }
    catch (std::exception &e) {
        std::stringstream sstream;
        sstream << "Failed to delete deleted files with '" << e.what()
                << "'. Language server will not work.";
        window_logMessage(::lsp::data::message_type::Error, sstream.str());
        return;
    }

    try {
        for (const auto &file: m_context->storage().get_all<database::tables::t_file>(
                where(c(&database::tables::t_file::is_outdated) == false))) {
            publish_diagnostics(file);
        }
    }
    catch (std::exception &e) {
        std::stringstream sstream;
        sstream << "Failed to republish diagnostics with '" << e.what()
                << "'. Language server will not work.";
        window_logMessage(::lsp::data::message_type::Error, sstream.str());
        return;
    }

    analyze_outdated_files();
}

void sqfvm::language_server::language_server::analyze_outdated_files() {
    try {
        for (const auto &file: m_context->storage().get_all<database::tables::t_file>(
                where(c(&database::tables::t_file::is_outdated) == true &&
                      c(&database::tables::t_file::is_deleted) == false))) {
            analyse_file(file);
        }
    }
    catch (std::exception &e) {
        std::stringstream sstream;
        sstream << "Failed to analyze outdated files with '" << e.what()
                << "'.";
        window_logMessage(lsp::data::message_type::Error, sstream.str());
        return;
    }
}

void sqfvm::language_server::language_server::on_workspace_didChangeConfiguration(
        const ::lsp::data::did_change_configuration_params &params) {
}

std::optional<std::vector<lsp::data::location>> sqfvm::language_server::language_server::on_textDocument_references(
        const lsp::data::references_params &params) {
    auto path = std::filesystem::path(
            std::string(params.textDocument.uri.path().begin(),
                        params.textDocument.uri.path().end()))
            .lexically_normal();
    auto files = m_context->storage().get_all<database::tables::t_file>(
            where(c(&database::tables::t_file::path) == path.string()));
    if (files.empty())
        return std::nullopt;
    auto file = files.front();
    auto references = m_context->storage().get_all<database::tables::t_reference>(
            where(c(&database::tables::t_reference::file_fk) == file.id_pk
                  && c(&database::tables::t_reference::line) == params.position.line + 1
                  && c(&database::tables::t_reference::is_magic_variable) == false));
    if (references.empty())
        return std::nullopt;
    std::optional<uint64_t> variable_id_opt;
    for (auto &reference: references) {
        if (reference.column >= params.position.character + 1 ||
            reference.column + reference.length <= params.position.character + 1)
            continue;
        variable_id_opt = reference.variable_fk;
        break;
    }
    if (!variable_id_opt.has_value())
        return std::nullopt;
    auto variable_id = variable_id_opt.value();
    auto variable_references = m_context->storage().get_all<database::tables::t_reference>(
            where(c(&database::tables::t_reference::variable_fk) == variable_id));
    std::vector<lsp::data::location> locations;
    for (const auto &reference: variable_references) {
        auto file_opt = m_context->storage().get_optional<database::tables::t_file>(reference.file_fk);
        auto file_path = file_opt->path;
        lsp::data::uri file_uri("file:///" + file_path);
        locations.emplace_back(lsp::data::location{
                .uri = file_uri,
                .range = lsp::data::range{
                        .start = lsp::data::position{
                                .line = reference.line - 1,
                                .character = reference.column
                        },
                        .end = lsp::data::position{
                                .line = reference.line - 1,
                                .character = reference.column + reference.length
                        }
                },
        });
    }
    return {locations};
}

std::optional<::sqfvm::language_server::database::tables::t_file>
sqfvm::language_server::language_server::get_file_from_path(
        std::filesystem::path path,
        bool create_if_not_exists) {
    try {
        return db_get_file_from_path(*m_context, path, create_if_not_exists);
    }
    catch (std::exception &e) {
        std::stringstream sstream;
        sstream << "Failed to insert file '" << path << "' with '" << e.what()
                << "'. Language server will not work for this file.";
        window_logMessage(::lsp::data::message_type::Error, sstream.str());
        return {};
    }
}

void sqfvm::language_server::language_server::on_textDocument_didChange(
        const ::lsp::data::did_change_text_document_params &params) {
    auto path = std::filesystem::path(
            std::string(params.textDocument.uri.path().begin(),
                        params.textDocument.uri.path().end()))
            .lexically_normal();
    auto file_opt = get_file_from_path(path, true);
    if (!file_opt.has_value())
        return;
    auto file = file_opt.value();
    file.is_outdated = true;
    m_context->storage().update(file);

    push_file_history(file, params.contentChanges[0].text);
    mark_related_files_as_outdated(file);
    analyze_outdated_files();
}

void sqfvm::language_server::language_server::push_file_history(
        ::sqfvm::language_server::database::tables::t_file file,
        std::string contents,
        bool is_external) {
    m_context->storage().insert(database::tables::t_file_history{
            .file_fk = file.id_pk,
            .content = contents,
            .time_stamp_created = (uint64_t) std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count(),
            .is_external = false,
    });
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
    mark_related_files_as_outdated(file);
    m_context->storage().remove_all<database::tables::t_diagnostic>(
            where(c(&database::tables::t_diagnostic::file_fk) == file.id_pk));
    m_context->storage().remove_all<database::tables::t_reference>(
            where(c(&database::tables::t_reference::file_fk) == file.id_pk));
    m_context->storage().remove_all<database::tables::t_variable>(
            where(c(&database::tables::t_variable::opt_file_fk) == file.id_pk));
    file.is_deleted = true;
    m_context->storage().update<database::tables::t_file>(file);
}

void sqfvm::language_server::language_server::mark_related_files_as_outdated(
        const sqfvm::language_server::database::tables::t_file &file) {
    std::set<uint64_t> outdated_file_ids{};
    auto variables = m_context->storage().get_all<database::tables::t_variable>(
            where(c(&database::tables::t_variable::opt_file_fk) == file.id_pk
                  and c(&database::tables::t_variable::scope) == "missionNamespace"));
    for (const auto &variable: variables) {
        auto non_local_references = m_context->storage().get_all<database::tables::t_reference>(
                where(c(&database::tables::t_reference::variable_fk) == variable.id_pk
                      and c(&database::tables::t_reference::file_fk) != file.id_pk));
        for (const auto &reference: non_local_references) {
            outdated_file_ids.insert(reference.file_fk);
        }
    }

    // Mark all files as outdated
    for (const auto &outdated_file_id: outdated_file_ids) {
        auto outdated_file = m_context->storage().get<database::tables::t_file>(outdated_file_id);
        outdated_file.is_outdated = true;
        m_context->storage().update(outdated_file);
    }

}

void sqfvm::language_server::language_server::analyse_file(
        const sqfvm::language_server::database::tables::t_file &file) {
    auto runtime = m_sqfvm_factory.create([](auto &_) {}, *m_context);

    // Create analyzer
    auto extension = std::filesystem::path(file.path).extension().string();
    auto contents = m_context->storage().get_all<sqfvm::language_server::database::tables::t_file_history>(
            where(c(&database::tables::t_file_history::file_fk) == file.id_pk),
            order_by(&database::tables::t_file_history::time_stamp_created).desc(),
            limit(1));
    std::string content;
    if (contents.empty()) {
        auto file_contents = sqf::fileio::passthrough::read_file_from_disk(file.path);
        if (file_contents.has_value())
            push_file_history(file, *file_contents, true);
        else
            return;
        content = file_contents.value();
    } else {
        content = contents[0].content;
    }
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
        publish_diagnostics(file);
    }
    catch (std::exception &e) {
        std::stringstream sstream;
        sstream << "Failed to analyze '" << file.path << "': " << e.what();
        window_logMessage(::lsp::data::message_type::Error, sstream.str());
    }
}

void sqfvm::language_server::language_server::publish_diagnostics(
        const sqfvm::language_server::database::tables::t_file &file) {
    auto file_diagnostics = m_context->storage().get_all<database::tables::t_diagnostic>(
            where(c(&database::tables::t_diagnostic::file_fk) == file.id_pk));
    lsp::data::publish_diagnostics_params params = {};
    params.uri = sanitize_to_uri(file.path);
    for (const auto &diagnostic: file_diagnostics) {
        lsp::data::diagnostics diag = {};
        diag.code = diagnostic.code;
        diag.message = diagnostic.message;
        diag.range.start.line = diagnostic.line;
        diag.range.start.character = diagnostic.column;
        diag.range.end.line = diagnostic.line;
        diag.range.end.character = diagnostic.column + diagnostic.length;
        diag.severity = diagnostic.severity == database::tables::t_diagnostic::fatal
                        ? lsp::data::diagnostic_severity::Error
                        : diagnostic.severity == database::tables::t_diagnostic::error
                          ? lsp::data::diagnostic_severity::Error
                          : diagnostic.severity == database::tables::t_diagnostic::warning
                            ? lsp::data::diagnostic_severity::Warning
                            : diagnostic.severity == database::tables::t_diagnostic::info
                              ? lsp::data::diagnostic_severity::Information
                              : lsp::data::diagnostic_severity::Hint;
        params.diagnostics.push_back(diag);
    }
    textDocument_publishDiagnostics(params);
}

void sqfvm::language_server::language_server::file_system_item_removed(
        const Poco::DirectoryWatcher::DirectoryEvent &event) {
    std::filesystem::path path = event.item.path();
    if (is_subpath(path, m_lsp_folder.parent_path()))
        return;
    auto file_opt = get_file_from_path(path.string(), true);
    if (!file_opt.has_value()) {
        return;
    }
    delete_file(file_opt.value());
    analyze_outdated_files();
}

void sqfvm::language_server::language_server::file_system_item_added(
        const Poco::DirectoryWatcher::DirectoryEvent &event) {
    std::filesystem::path path = event.item.path();
    if (is_subpath(path, m_lsp_folder.parent_path()))
        return;
    auto file_opt = get_file_from_path(path.string(), true);
    if (!file_opt.has_value()) {
        return;
    }
    analyze_outdated_files();
}

void sqfvm::language_server::language_server::file_system_item_modified(
        const Poco::DirectoryWatcher::DirectoryEvent &event) {
    std::filesystem::path path = event.item.path();
    if (is_subpath(path, m_lsp_folder.parent_path()))
        return;
    auto file_opt = get_file_from_path(path.string(), true);
    if (!file_opt.has_value()) {
        return;
    }
    auto file = file_opt.value();
    file.is_outdated = true;
    m_context->storage().update(file);
    mark_related_files_as_outdated(file);
    analyze_outdated_files();
}

void sqfvm::language_server::language_server::ensure_git_ignore_file_exists() {
    std::filesystem::path git_ignore_path = m_lsp_folder / ".gitignore";
    if (!std::filesystem::exists(git_ignore_path)) {
        std::ofstream file(git_ignore_path);
        file << ".gitignore" << std::endl;
        file << "sqlite3.db" << std::endl;
        file << "sqlite3.db-journal" << std::endl;
        file << "sqlite3.db-wal" << std::endl;
        file << "sqlite3.db-shm" << std::endl;
        file.close();
    }
}
