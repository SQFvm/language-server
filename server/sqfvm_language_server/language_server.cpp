#include "language_server.hpp"

#include "analysis/sqf_ast/sqf_ast_analyzer.hpp"


#include <Poco/Delegate.h>
#include <string_view>
#include <fstream>
#include <utility>
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
    ensure_git_ignore_file_exists();
    m_context = std::make_shared<database::context>(m_db_path);
    m_context->migrate();
    m_sqfvm_factory.add_mapping(uri.string(), "");
    m_file_system_watcher.watch(uri);
    m_file_system_watcher.callback_add(
            [&](auto &path, bool is_directory) { file_system_item_added(path, is_directory); });
    m_file_system_watcher.callback_remove(
            [&](auto &path, bool is_directory) { file_system_item_removed(path, is_directory); });
    m_file_system_watcher.callback_modify(
            [&](auto &path, bool is_directory) { file_system_item_modified(path, is_directory); });

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

    log_sqlite_migration_report();

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

    auto runtime = m_sqfvm_factory.create([](auto &_) {}, *m_context, std::make_shared<analysis::slspp_context>());

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
                        std::chrono::clock_cast<std::chrono::system_clock>(last_write_time).time_since_epoch())
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
                add_or_update_pboprefix_mapping_logging(file_path);
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

void sqfvm::language_server::language_server::log_sqlite_migration_report() {
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
    window_logMessage(lsp::data::message_type::Log, sstream.str());
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
        const std::filesystem::path &path,
        bool create_if_not_exists) {
    try {
        return m_context->db_get_file_from_path(path, create_if_not_exists);
    }
    catch (std::exception &e) {
        std::stringstream sstream;
        sstream << "Failed to insert file '" << path << "' with '" << e.what()
                << "'. Language server will not work for this file.";
        window_logMessage(::lsp::data::message_type::Error, sstream.str());
        return {};
    }
}

void sqfvm::language_server::language_server::remove_pboprefix_mapping(
        const std::filesystem::path &pboprefix) {
    m_sqfvm_factory.remove_mapping(pboprefix.parent_path().string());
}

std::string sqfvm::language_server::language_server::add_or_update_pboprefix_mapping_safe(
        const std::filesystem::path &pboprefix) {
    if (!exists(pboprefix))
        return ("File '" + pboprefix.string() + "' does not exist.");
    auto content_opt = sqf::fileio::passthrough::read_file_from_disk(pboprefix.string());
    if (!content_opt.has_value())
        return ("Failed to read file '" + pboprefix.string() + "'.");
    auto content = content_opt.value();
    m_sqfvm_factory.update_mapping(pboprefix.parent_path().string(), content);
    return {};
}

void sqfvm::language_server::language_server::add_or_update_pboprefix_mapping_logging(
        const std::filesystem::path &pboprefix) {
    auto result = add_or_update_pboprefix_mapping_safe(pboprefix);
    if (!result.empty()) {
        window_log(::lsp::data::message_type::Error, [&](auto &sstream) {
            sstream << "Failed to read '" << pboprefix << "': " << result;
        });
    }
}

void sqfvm::language_server::language_server::on_textDocument_didChange(
        const ::lsp::data::did_change_text_document_params &params) {
    auto path = std::filesystem::path(
            std::string(params.textDocument.uri.path().begin(),
                        params.textDocument.uri.path().end()))
            .lexically_normal();

    if (iequal(path.filename().string(), "$PBOPREFIX$")) {
        // We intentionally ignore the results here to delay the update of the mapping until the user saves the file
        window_log(::lsp::data::message_type::Info, [&](auto &sstream) {
            sstream << "Detected change in '"
                    << path
                    << "'. Language server will update the mapping when the file is saved.";
        });
    } else {
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
}

void sqfvm::language_server::language_server::push_file_history(
        const ::sqfvm::language_server::database::tables::t_file &file,
        std::string contents,
        bool is_external) {
    auto time_stamp = (uint64_t) std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count();
    m_context->storage().insert(database::tables::t_file_history{
            .file_fk = file.id_pk,
            .content = std::move(contents),
            .time_stamp_created = time_stamp,
            .is_external = is_external,
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
                    auto ls_path,
                    auto db_path,
                    auto &factory,
                    auto file,
                    auto text) -> std::unique_ptr<analysis::analyzer> {
                return std::make_unique<analysis::sqf_ast::sqf_ast_analyzer>(
                        db_path,
                        file,
                        factory,
                        std::move(text),
                        ls_path);
            });
}

bool sqfvm::language_server::language_server::delete_file(const std::filesystem::path &file) {
    auto file_opt = get_file_from_path(file.string(), false);
    if (!file_opt.has_value()) {
        return false;
    }
    delete_file(file_opt.value());
    return true;
}

void sqfvm::language_server::language_server::delete_file(sqfvm::language_server::database::tables::t_file file) {
    mark_related_files_as_outdated(file);
    file.is_deleted = true;
    m_context->storage().update<database::tables::t_file>(file);
    m_context->storage().remove_all<database::tables::t_diagnostic>(
            where(c(&database::tables::t_diagnostic::file_fk) == file.id_pk));
    m_context->storage().remove_all<database::tables::t_reference>(
            where(c(&database::tables::t_reference::file_fk) == file.id_pk));
    m_context->storage().remove_all<database::tables::t_variable>(
            where(c(&database::tables::t_variable::opt_file_fk) == file.id_pk));
    publish_diagnostics(file);
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
    uint64_t timestamp = (uint64_t) std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::clock_cast<std::chrono::system_clock>(last_write_time(std::filesystem::path(file.path)))
                    .time_since_epoch())
            .count();
    // Create analyzer
    auto extension = std::filesystem::path(file.path).extension().string();
    auto contents = m_context->storage().get_all<sqfvm::language_server::database::tables::t_file_history>(
            where(c(&database::tables::t_file_history::file_fk) == file.id_pk),
            order_by(&database::tables::t_file_history::time_stamp_created).desc(),
            limit(1));
    std::string content;
    if (contents.empty() || contents[0].time_stamp_created < timestamp) {
        auto file_contents = sqf::fileio::passthrough::read_file_from_disk(file.path);
        if (file_contents.has_value())
            push_file_history(file, *file_contents, true);
        else
            return;
        content = file_contents.value();
    } else {
        content = contents[0].content;
    }
    auto analyzer_opt = m_analyzer_factory.get(
            extension,
            m_lsp_folder,
            m_context->db_path(),
            m_sqfvm_factory,
            file,
            content);
    if (!analyzer_opt.has_value()) {
        return;
    }
    window_log(::lsp::data::message_type::Log, [&](auto &sstream) {
        sstream << "Analyzing '" << file.path << "'";
    });
    try {
        analyzer_opt.value()->analyze();
        analyzer_opt.value()->commit();
    }
    catch (std::exception &e) {
        std::stringstream sstream;
        sstream << "Failed to analyze '" << file.path << "': " << e.what();
        window_logMessage(::lsp::data::message_type::Error, sstream.str());
        m_context->storage().remove_all<database::tables::t_reference>(
                where(c(&database::tables::t_reference::file_fk) == file.id_pk));
        m_context->storage().insert(database::tables::t_diagnostic{
                .file_fk = file.id_pk,
                .source_file_fk = file.id_pk,
                .severity = database::tables::t_diagnostic::error,
                .message = sstream.str(),
                .code = "VV-ERR",
        });
    }
    try {
        publish_diagnostics(file);
    }
    catch (std::exception &e) {
        window_log(::lsp::data::message_type::Error, [&](auto &sstream) {
            sstream << "Failed to publish diagnostics for '" << file.path << "': " << e.what();
        });
    }
}

void sqfvm::language_server::language_server::publish_diagnostics(
        const sqfvm::language_server::database::tables::t_file &file,
        bool publish_sub_files) {
    auto file_diagnostics = m_context->storage().get_all<database::tables::t_diagnostic>(
            where((c(&database::tables::t_diagnostic::source_file_fk) == file.id_pk
                   or c(&database::tables::t_diagnostic::file_fk) == file.id_pk)
                  and c(&database::tables::t_diagnostic::is_suppressed) == false));
    lsp::data::publish_diagnostics_params params = {};
    params.uri = sanitize_to_uri(file.path);
    std::set<uint64_t> additional_files{};
    for (const auto &diagnostic: file_diagnostics) {
        if (diagnostic.file_fk != file.id_pk) {
            additional_files.insert(diagnostic.file_fk);
            continue;
        }
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
    if (!publish_sub_files)
        return;
    for (auto &additional_file_id: additional_files) {
        auto additional_file = m_context->storage().get_optional<database::tables::t_file>(additional_file_id);
        if (!additional_file.has_value())
            continue;
        publish_diagnostics(additional_file.value(), false);
    }
}

void sqfvm::language_server::language_server::mark_file_as_outdated(const std::filesystem::path &path) {
    auto file_opt = get_file_from_path(path.string(), true);
    if (!file_opt.has_value()) {
        return;
    }
    auto file = file_opt.value();
    bool changed = false;
    if (!file.is_outdated) {
        file.is_outdated = true;
        changed = true;
    }
    if (changed)
        m_context->storage().update(file);
}

void sqfvm::language_server::language_server::mark_all_files_as_outdated() {
    m_context->storage().update_all(set(c(&database::tables::t_file::is_outdated) = true));
}

void sqfvm::language_server::language_server::file_system_item_removed(
        const std::filesystem::path &path,
        bool is_directory) {
    if (is_subpath(path, m_lsp_folder.parent_path()))
        return;
    if (is_directory) {
        auto files = m_context->storage().get_all<database::tables::t_file>(
                where(like(&database::tables::t_file::path, path.string() + "%")));
        for (const auto &file: files) {
            delete_file(file);
        }
    } else if (iequal(path.filename().string(), "$PBOPREFIX$")) {
        remove_pboprefix_mapping(path);
        mark_all_files_as_outdated();
    } else {
        if (!delete_file(path))
            return;
    }
    analyze_outdated_files();
}

void sqfvm::language_server::language_server::file_system_item_added(
        const std::filesystem::path &path,
        bool is_directory) {
    if (is_subpath(path, m_lsp_folder.parent_path()))
        return;
    if (is_directory) {
        for (auto &p: std::filesystem::recursive_directory_iterator(path)) {
            if (std::filesystem::is_directory(p))
                continue;
            mark_file_as_outdated(p);
        }
    } else if (iequal(path.filename().string(), "$PBOPREFIX$")) {
        add_or_update_pboprefix_mapping_logging(path);
        mark_all_files_as_outdated();
    } else {
        mark_file_as_outdated(path);
    }
    analyze_outdated_files();
}

void sqfvm::language_server::language_server::file_system_item_modified(
        const std::filesystem::path &path,
        bool is_directory) {
    if (is_directory)
        return;
    if (is_subpath(path, m_lsp_folder.parent_path()))
        return;
    if (iequal(path.filename().string(), "$PBOPREFIX$")) {
        add_or_update_pboprefix_mapping_logging(path);
        mark_all_files_as_outdated();
    } else {
        auto file_opt = get_file_from_path(path.string(), true);
        if (!file_opt.has_value()) {
            return;
        }
        auto file = file_opt.value();
        if (!file.is_outdated) {
            file.is_outdated = true;
            m_context->storage().update(file);
        }
        mark_related_files_as_outdated(file);
    }
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

::lsp::data::initialize_result sqfvm::language_server::language_server::on_initialize(
        const lsp::data::initialize_params &params) {
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
    // res.capabilities.foldingRangeProvider = ::lsp::data::initialize_result::server_capabilities::folding_range_registration_options{};
    // res.capabilities.foldingRangeProvider->documentSelector = ::lsp::data::document_filter{};
    // res.capabilities.foldingRangeProvider->documentSelector->language = "sqf";
    res.capabilities.completionProvider = lsp::data::initialize_result::server_capabilities::completion_options{.resolveProvider = true};
    res.capabilities.referencesProvider = lsp::data::initialize_result::server_capabilities::reference_options{.workDoneProgress = false};
    res.capabilities.codeActionProvider = lsp::data::initialize_result::server_capabilities::code_action_options{
            .codeActionKinds = {std::vector<lsp::data::code_action_kind>{
                    lsp::data::code_action_kind::QuickFix,
                    lsp::data::code_action_kind::Refactor,
                    lsp::data::code_action_kind::RefactorExtract,
                    lsp::data::code_action_kind::RefactorInline,
                    lsp::data::code_action_kind::Source,
                    lsp::data::code_action_kind::RefactorRewrite,
            }}
    };

    return res;
}

std::optional<std::vector<std::variant<lsp::data::command, lsp::data::code_action>>>
sqfvm::language_server::language_server::on_textDocument_codeAction(const lsp::data::code_action_params &params) {
    using namespace lsp::data;
    using namespace database::tables;
    using namespace std::string_literals;
    const size_t lsp_trash_offset = 1;
    auto path = std::filesystem::path(
            std::string(params.textDocument.uri.path().begin(),
                        params.textDocument.uri.path().end()))
            .lexically_normal();
    auto file_opt = get_file_from_path(path.string(), false);
    if (!file_opt.has_value())
        return std::nullopt;
    auto file = file_opt.value();

    std::vector<std::variant<lsp::data::command, lsp::data::code_action>> out_data{};

    {
        // Get code actions and their corresponding changes from database
        auto code_actions = m_context->storage().get_all<t_code_action>(
                where(c(&t_code_action::file_fk) == file.id_pk));
        for (const auto &code_action: code_actions) {
            std::vector<std::variant<text_document_edit, create_file, rename_file, lsp::data::delete_file>> out_changes{};
            auto changes = m_context->storage().get_all<t_code_action_change>(
                    where(c(&t_code_action_change::code_action_fk) == code_action.id_pk));
            bool in_range = false;
            for (const auto &change: changes) {
                in_range = in_range || change.start_line >= params.range.start.line
                                       && change.start_column >= params.range.start.character
                                       && change.end_line <= params.range.end.line
                                       && change.end_column <= params.range.end.character;
                auto change_path = sanitize_to_uri(change.path);
                switch (change.operation) {
                    case t_code_action_change::file_change:
                        out_changes.emplace_back(text_document_edit{
                                .textDocument = optional_versioned_text_document_identifier{
                                        .version = std::nullopt, // ToDo: Start tracking version numbers because LSP is a stupid piece of shit
                                        .uri = change_path,
                                },
                                .edits = {text_edit{
                                        .range = range{
                                                .start = position{
                                                        .line = change.start_line.value_or(0),
                                                        .character = change.start_column.value_or(0),
                                                },
                                                .end = position{
                                                        .line = change.end_line.value_or(0),
                                                        .character = change.end_column.value_or(0),
                                                },
                                        },
                                        .new_text = change.content.value_or(""s),
                                }},
                        });
                        break;
                    case t_code_action_change::file_create:
                        out_changes.emplace_back(create_file{
                                .uri = change_path,
                                .options = create_file::create_file_options{
                                        .overwrite = true,
                                        .ignore_if_exists = true,
                                },
                        });
                        out_changes.emplace_back(text_document_edit{
                                .textDocument = optional_versioned_text_document_identifier{
                                        .version = std::nullopt, // ToDo: Start tracking version numbers because LSP is a stupid piece of shit
                                        .uri = change_path,
                                },
                                .edits = {text_edit{
                                        .range = range{
                                                .start = position{0, 0},
                                                .end = position{0, 0},
                                        },
                                        .new_text = change.content.value_or(""s),
                                }},
                        });
                        break;
                    case t_code_action_change::file_delete:
                        out_changes.emplace_back(lsp::data::delete_file{
                                .uri = change_path,
                                .options = lsp::data::delete_file::delete_file_options{
                                        .recursive = true,
                                        .ignore_if_not_exists = true,
                                },
                        });
                        break;
                    case t_code_action_change::file_rename:
                        out_changes.emplace_back(rename_file{
                                .oldUri = sanitize_to_uri(change.old_path.value_or(""s)),
                                .newUri = change_path,
                                .options = rename_file::rename_file_options{
                                        .overwrite = true,
                                        .ignore_if_exists = true,
                                },
                        });
                        break;
                }
            }
            if (out_changes.empty() || !in_range)
                continue;
            out_data.push_back(lsp::data::code_action{
                    .title = code_action.text,
                    .kind = code_action.kind == database::tables::t_code_action::quick_fix
                            ? code_action_kind::QuickFix
                            : code_action.kind == database::tables::t_code_action::refactor
                              ? code_action_kind::Refactor
                              : code_action.kind == database::tables::t_code_action::extract_refactor
                                ? code_action_kind::RefactorExtract
                                : code_action.kind == database::tables::t_code_action::inline_refactor
                                  ? code_action_kind::RefactorInline
                                  : code_action.kind == database::tables::t_code_action::whole_file
                                    ? code_action_kind::Source
                                    : code_action.kind == database::tables::t_code_action::rewrite_refactor
                                      ? code_action_kind::RefactorRewrite
                                      : code_action_kind::Empty,
                    .isPreferred = true,
                    .edit = workspace_edit{
                            .changes = {},
                            .document_changes = out_changes,
                            .change_annotations = {},
                    },
            });
        }
    }
    return {out_data};
}
