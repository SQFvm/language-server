#include "language_server.hpp"

#include "analysis/sqf_ast/sqf_ast_analyzer.hpp"
#include "analysis/config_ast/config_ast_analyzer.hpp"


#include <string_view>
#include <fstream>
#include <utility>
#include <vector>
#include <set>
#include <sstream>

#if defined(__GNUC__)
#include <date/tz.h>
#endif

using namespace std::string_view_literals;
using namespace sqlite_orm;

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
    if (!database::context::operations::for_each_file_outdated_and_not_deleted(
            *m_context,
            context_err_log(),
            [&](auto &file) {
                analyse_file(file);
                return false;
            }))
        return;
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

void sqfvm::language_server::language_server::push_file_history(
        const ::sqfvm::language_server::database::tables::t_file &file,
        std::string contents,
        bool is_external) {
    auto time_stamp = (uint64_t) std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count();
    auto _ = database::context::operations::insert(
            *m_context,
            context_err_log(),
            database::tables::t_file_history{
                    .file_fk = file.id_pk,
                    .content = std::move(contents),
                    .time_stamp_created = time_stamp,
                    .is_external = is_external,
            });
}


sqfvm::language_server::language_server::language_server() : m_sqfvm_factory(this) {
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
    m_analyzer_factory.set(
            ".ext", [](
                    auto ls_path,
                    auto db_path,
                    auto &factory,
                    auto file,
                    auto text) -> std::unique_ptr<analysis::analyzer> {
                return std::make_unique<analysis::config_ast::config_ast_analyzer>(
                        db_path,
                        file,
                        factory,
                        std::move(text),
                        ls_path);
            });
    m_analyzer_factory.set(
            ".cpp", [](
                    auto ls_path,
                    auto db_path,
                    auto &factory,
                    auto file,
                    auto text) -> std::unique_ptr<analysis::analyzer> {
                return std::make_unique<analysis::config_ast::config_ast_analyzer>(
                        db_path,
                        file,
                        factory,
                        std::move(text),
                        ls_path);
            });
}

sqfvm::language_server::language_server::language_server(jsonrpc &&rpc) : server(std::move(rpc)),
                                                                          m_sqfvm_factory(this) {
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
    m_analyzer_factory.set(
            ".ext", [](
                    auto ls_path,
                    auto db_path,
                    auto &factory,
                    auto file,
                    auto text) -> std::unique_ptr<analysis::analyzer> {
                return std::make_unique<analysis::config_ast::config_ast_analyzer>(
                        db_path,
                        file,
                        factory,
                        std::move(text),
                        ls_path);
            });
    m_analyzer_factory.set(
            ".cpp", [](
                    auto ls_path,
                    auto db_path,
                    auto &factory,
                    auto file,
                    auto text) -> std::unique_ptr<analysis::analyzer> {
                return std::make_unique<analysis::config_ast::config_ast_analyzer>(
                        db_path,
                        file,
                        factory,
                        std::move(text),
                        ls_path);
            });
}

void sqfvm::language_server::language_server::add_ignored_paths(const std::filesystem::path &workspace,
                                                                const std::filesystem::path &lsp_folder) {
    auto ignore_list = lsp_folder / "ls-ignore.txt";
    if (!exists(ignore_list)) {
        auto file = std::ofstream(ignore_list);
        file << "################################################################\n";
        file << "### This file contains a list of paths to ignore changes of. ###\n";
        file << "### The paths are relative to the workspace root.            ###\n";
        file << "### The paths are separated by newlines.                     ###\n";
        file << "### Note that this is not behaving like a .gitignore file,   ###\n";
        file << "### and you cannot invert some paths by prefixing them with  ###\n";
        file << "### a ! or use wildcards.                                    ###\n";
        file << "### Any subfolder of a path is also ignored.                 ###\n";
        file << "### Important: No leading or trailing whitespace is allowed  ###\n";
        file << "###            on any line.                                  ###\n";
        file << "### Changing anything in this file will have no effect until ###\n";
        file << "### the language server is restarted.                        ###\n";
        file << "### Keep in mind that already analyzed files will not be     ###\n";
        file << "### re-analyzed OR removed from the database.                ###\n";
        file << "################################################################\n";
        file << lsp_folder.lexically_relative(workspace).string() << "\n";
        file << ".vscode\n";
        file << ".github\n";
        file << ".git\n";
        file << ".hemtt\n";
    }
    std::ifstream file(ignore_list);
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty())
            continue;
        if (line[0] == '#')
            continue;
        auto path = workspace / line;
        if (exists(path))
            m_file_system_watcher.ignore(path);
    }
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
    using namespace sqfvm::language_server::database::tables;
    mark_related_files_as_outdated(file);
    file.is_deleted = true;
    m_context->storage().update<t_file>(file);
    m_context->storage().remove_all<t_diagnostic>(
            where(c(&t_diagnostic::file_fk) == file.id_pk));
    m_context->storage().remove_all<t_reference>(
            where(c(&t_reference::file_fk) == file.id_pk));
    m_context->storage().remove_all<t_variable>(
            where(c(&t_variable::opt_file_fk) == file.id_pk));
    m_context->storage().remove_all<t_file_history>(
            where(c(&t_file_history::file_fk) == file.id_pk));
    m_context->storage().remove_all<t_file_include>(
            where(c(&t_file_include::file_included_in_fk) == file.id_pk
                  or c(&t_file_include::file_included_fk) == file.id_pk
                  or c(&t_file_include::source_file_fk) == file.id_pk));
    m_context->storage().remove_all<t_hover>(
            where(c(&t_hover::file_fk) == file.id_pk));
    m_context->storage().remove_all<t_variable>(
            where(c(&t_variable::opt_file_fk) == file.id_pk));
    publish_diagnostics(file);
}

void sqfvm::language_server::language_server::mark_related_files_as_outdated(
        const sqfvm::language_server::database::tables::t_file &file) {
    std::set<uint64_t> outdated_file_ids{};

    auto includes = m_context->storage().get_all<database::tables::t_file_include>(
            where(c(&database::tables::t_file_include::file_included_fk) == file.id_pk));
    for (const auto &include: includes) {
        outdated_file_ids.insert(include.file_included_in_fk);
        auto file_included_in = m_context->storage().get<database::tables::t_file>(include.file_included_in_fk);
        mark_related_files_as_outdated(file_included_in);
    }


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
#if defined(__GNUC__)
            date::clock_cast<std::chrono::system_clock>(last_write_time(std::filesystem::path(file.path)))
                    .time_since_epoch())
#else
            std::chrono::clock_cast<std::chrono::system_clock>(last_write_time(std::filesystem::path(file.path)))
                    .time_since_epoch())
#endif 
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

    bool file_ignored = m_file_system_watcher.is_ignored(file.path);
    if (file.is_ignored != file_ignored) {
        auto lFile = m_context->storage().get<database::tables::t_file>(file.id_pk);
        lFile.is_ignored = file_ignored;
        m_context->storage().update(lFile);
        if (file_ignored) {
            m_context->storage().remove_all<database::tables::t_diagnostic>(
                    where(c(&database::tables::t_diagnostic::file_fk) == file.id_pk));
        }
    }
    // if extension is either .cpp or .ext, skip the file at the given path unless it's filename is either config.cpp or description.ext
    if ((extension == ".cpp" || extension == ".ext")) {
        auto filename = std::filesystem::path(file.path).filename().string();
        if (!iequal(filename, "config.cpp") && !iequal(filename, "description.ext"))
            return;
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
        if (!file_ignored) {
            analyzer_opt.value()->analyze();
            analyzer_opt.value()->commit();
        }
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
    std::lock_guard<std::mutex> lock(m_analyze_mutex);
    if (is_directory) {
        auto files = m_context->storage().get_all<database::tables::t_file>(
                where(like(&database::tables::t_file::path, path.string() + "%")));
        for (const auto &file: files) {
            delete_file(file);
        }
    } else if (iequal(path.filename().string(), "$PBOPREFIX$")) {
        remove_pboprefix_mapping(path);
        mark_all_files_as_outdated();
        debug_print_sqfvm_vpath_start_parameters();
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
    std::lock_guard<std::mutex> lock(m_analyze_mutex);
    if (is_directory) {
        for (auto &p: std::filesystem::recursive_directory_iterator(path)) {
            if (std::filesystem::is_directory(p))
                continue;
            mark_file_as_outdated(p);
        }
    } else if (iequal(path.filename().string(), "$PBOPREFIX$")) {
        add_or_update_pboprefix_mapping_logging(path);
        mark_all_files_as_outdated();
        debug_print_sqfvm_vpath_start_parameters();
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
    std::lock_guard<std::mutex> lock(m_analyze_mutex);
    if (iequal(path.filename().string(), "$PBOPREFIX$")) {
        add_or_update_pboprefix_mapping_logging(path);
        mark_all_files_as_outdated();
        debug_print_sqfvm_vpath_start_parameters();
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
    auto parent_path = git_ignore_path.parent_path();
    if (!std::filesystem::exists(git_ignore_path)) {
        std::filesystem::create_directories(parent_path);
    }
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
