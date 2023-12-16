#include "language_server.hpp"

#include "analysis/sqf_ast/sqf_ast_analyzer.hpp"


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
    add_ignored_paths(uri, m_lsp_folder);
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
    if (!database::context::operations::mark_all_files_as_deleted(*m_context, context_err_log()))
        return;

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
            if (m_file_system_watcher.is_ignored(file_path))
                continue;
            if (m_analyzer_factory.has(file_path.extension().string())) {
                auto last_write_time = iter->last_write_time();
                uint64_t timestamp = (uint64_t) std::chrono::duration_cast<std::chrono::milliseconds>(
#if defined(__GNUC__)
                        date::clock_cast<std::chrono::system_clock>(last_write_time).time_since_epoch())
                        .count();
#else
                        std::chrono::clock_cast<std::chrono::system_clock>(last_write_time).time_since_epoch())
                        .count();
#endif
                auto [op_success, file] = database::context::operations::find_file_by_path(*m_context,
                                                                                           context_err_log(),
                                                                                           file_path);
                if (!op_success)
                    continue;

                if (file.has_value()) {
                    file->is_deleted = false;
                    file->is_outdated = file->is_outdated || file->last_changed < timestamp;
                    file->last_changed = timestamp;
                    if (!database::context::operations::update(*m_context, context_err_log(), *file))
                        continue;
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
                    if (!database::context::operations::insert(*m_context, context_err_log(), f))
                        continue;
                }
            } else if (file_path.filename() == "$PBOPREFIX$") {
                add_or_update_pboprefix_mapping_logging(file_path);
            }
        }
    }
    debug_print_sqfvm_vpath_start_parameters();

    if (!database::context::operations::for_each_file_not_outdated(
            *m_context,
            context_err_log(),
            [&](auto &file) {
                publish_diagnostics(file);
                return false;
            }))
        return;

    analyze_outdated_files();

    if (!database::context::operations::delete_files_flagged_with_is_deleted(*m_context, context_err_log()))
        return;
}

void sqfvm::language_server::language_server::on_workspace_didChangeConfiguration(
        const ::lsp::data::did_change_configuration_params &params) {
    // Path mappings
    m_sqfvm_factory.clear_workspace_mappings();
    if (params.settings.has_value() && params.settings->contains("sqfVmLanguageServer")) {
        auto settings = params.settings.value()["sqfVmLanguageServer"];
        if (settings.is_object() && settings.contains("Executable")) {
            auto executable = settings["Executable"];
            if (executable.is_object() && executable.contains("PathMappings")) {
                auto path_mappings = executable["PathMappings"];
                if (path_mappings.is_array()) {
                    for (auto &mapping: path_mappings) {
                        if (mapping.is_object() && mapping.contains("physical") && mapping.contains("virtual")) {
                            auto physical = mapping["physical"];
                            auto virtual_ = mapping["virtual"];
                            if (physical.is_string() && virtual_.is_string()) {
                                m_sqfvm_factory.add_mapping(physical, virtual_, true);
                            }
                        }
                    }
                }
            }
        }
    }
}

std::optional<std::vector<lsp::data::location>> sqfvm::language_server::language_server::on_textDocument_references(
        const lsp::data::references_params &params) {
    auto path = std::filesystem::path(
            std::string(params.textDocument.uri.path().begin(),
                        params.textDocument.uri.path().end()))
            .lexically_normal();
    auto [op_success1, file] = database::context::operations::find_file_by_path(*m_context, context_err_log(), path);
    if (!op_success1 || !file.has_value())
        return std::nullopt;
    auto [op_success2, references] = database::context::operations::find_all_references_by_file_and_line(
            *m_context,
            context_err_log(),
            file.value(),
            params.position.line + 1,
            true);
    if (!op_success2 || references.empty())
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
    auto [op_success3, variable_references] = database::context::operations::get_all_variables_of_variable(
            *m_context,
            context_err_log(),
            variable_id);
    if (!op_success3 || variable_references.empty())
        return std::nullopt;
    std::vector<lsp::data::location> locations;
    for (const auto &reference: variable_references) {
        auto [op_success4, file_opt] = database::context::operations::find_file_by_id(
                *m_context,
                context_err_log(),
                reference.file_fk);
        if (!op_success4 || !file_opt.has_value())
            continue;
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

void sqfvm::language_server::language_server::on_textDocument_didOpen(
        const lsp::data::did_open_text_document_params &params) {
    m_versions[static_cast<::lsp::data::document_uri>(params.text_document.uri.full())] = params.text_document.version;
}

void sqfvm::language_server::language_server::on_textDocument_didChange(
        const ::lsp::data::did_change_text_document_params &params) {
    std::lock_guard<std::mutex> lock(m_analyze_mutex);
    m_versions[static_cast<::lsp::data::document_uri>(params.text_document.uri.full())] = params.text_document.version;
    auto path = std::filesystem::path(
            std::string(params.text_document.uri.path().begin(),
                        params.text_document.uri.path().end()))
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
        if (!database::context::operations::update(*m_context, context_err_log(), file))
            return;

        push_file_history(file, params.content_changes[0].text);
        mark_related_files_as_outdated(file);
        analyze_outdated_files();
    }
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
    res.capabilities.hoverProvider = lsp::data::initialize_result::server_capabilities::hover_options{.workDoneProgress = false};
    res.capabilities.inlayHintProvider = lsp::data::initialize_result::server_capabilities::inlay_hint_options{.work_done_progress = false, .resolve_provider = true};

    return res;
}


std::optional<std::vector<lsp::data::inlay_hint>>
sqfvm::language_server::language_server::on_textDocument_inlayHint(const lsp::data::inlay_hint_params &params) {
    using database::tables::t_reference;
    auto line_start = params.range.start.line + 1;
    auto line_end = params.range.end.line + 1;
    auto column_start = params.range.start.character;
    auto column_end = params.range.end.character;
    auto path = std::filesystem::path(
            std::string(params.text_document.uri.path().begin(),
                        params.text_document.uri.path().end()))
            .lexically_normal();
    auto file_opt = get_file_from_path(path.string(), false);
    if (!file_opt.has_value())
        return std::nullopt;
    auto file = file_opt.value();
    auto references_in_range = m_context->storage().get_all<database::tables::t_reference>(
            where(c(&database::tables::t_reference::file_fk) == file.id_pk
                  and c(&database::tables::t_reference::line) >= line_start
                  and c(&database::tables::t_reference::line) <= line_end
                  and (c(&database::tables::t_reference::line) != line_start
                       or c(&database::tables::t_reference::column) >= column_start)
                  and (c(&database::tables::t_reference::line) != line_end
                       or c(&database::tables::t_reference::column) <= column_end)));
    std::unordered_map<uint64_t, std::vector<database::tables::t_reference>> variable_map{};
    for (auto &reference: references_in_range) {
        if (reference.is_magic_variable)
            continue;
        variable_map[reference.variable_fk].push_back(reference);
    }
    std::unordered_map<uint64_t, std::string> variable_types_string{};
    std::stringstream sstream;
    for (auto &[variable_id, references]: variable_map) {
        sstream.str("");
        sstream << ": ";
        auto variable = m_context->storage().get<database::tables::t_variable>(variable_id);
        if (!variable.opt_file_fk.has_value()) {
            sstream << "ERROR";
        } else {
            t_reference::type_flags final_type = t_reference::type_flags::none;
            for (auto &reference: references) {
                if (final_type == t_reference::type_flags::any || final_type == t_reference::type_flags::all) {
                    break;
                }
                final_type = final_type | reference.types;
            }
            if (final_type == t_reference::type_flags::none
                || final_type == t_reference::type_flags::all
                || final_type == t_reference::type_flags::any) {
                // ToDo: Track this properly, allowing for not always having "any" as type
                continue; // Skip for now to prevent any from being reported
                sstream << "any";
            } else {
                bool flag = false;
                if ((final_type & t_reference::type_flags::code) == t_reference::type_flags::code) {
                    sstream << (!flag ? "code" : ", code");
                    flag = true;
                }
                if ((final_type & t_reference::type_flags::scalar) == t_reference::type_flags::scalar) {
                    sstream << (!flag ? "scalar" : ", scalar");
                    flag = true;
                }
                if ((final_type & t_reference::type_flags::boolean) == t_reference::type_flags::boolean) {
                    sstream << (!flag ? "boolean" : ", boolean");
                    flag = true;
                }
                if ((final_type & t_reference::type_flags::object) == t_reference::type_flags::object) {
                    sstream << (!flag ? "object" : ", object");
                    flag = true;
                }
                if ((final_type & t_reference::type_flags::hashmap) == t_reference::type_flags::hashmap) {
                    sstream << (!flag ? "hashmap" : ", hashmap");
                    flag = true;
                }
                if ((final_type & t_reference::type_flags::array) == t_reference::type_flags::array) {
                    sstream << (!flag ? "array" : ", array");
                    flag = true;
                }
                if ((final_type & t_reference::type_flags::string) == t_reference::type_flags::string) {
                    sstream << (!flag ? "string" : ", string");
                    flag = true;
                }
                if ((final_type & t_reference::type_flags::nil) == t_reference::type_flags::nil) {
                    sstream << (!flag ? "nil" : ", nil");
                    flag = true;
                }
            }
        }
        variable_types_string[variable_id] = sstream.str();
    }
    std::vector<lsp::data::inlay_hint> hints{};
    for (auto &[variable_id, references]: variable_map) {
        auto variable = m_context->storage().get<database::tables::t_variable>(variable_id);
        if (!variable.opt_file_fk.has_value())
            continue;
        auto variable_type = variable_types_string[variable_id];
        for (auto &reference: references) {
            if (line_start > reference.line || reference.line > line_end)
                continue;
            if (line_start == reference.line && reference.column < column_start)
                continue;
            if (line_end == reference.line && reference.column > column_end)
                continue;

            hints.push_back(lsp::data::inlay_hint{
                    .position = lsp::data::position{
                            .line = reference.line - 1,
                            .character = reference.column + reference.length,
                    },
                    .label = {
                            // lsp::data::inlay_hint_label_part{.value = variable.variable_name},
                            lsp::data::inlay_hint_label_part{.value = variable_type}
                    },
                    .kind = lsp::data::inlay_hint_kind::Type,
            });
        }
    }
    return hints;
}

void sqfvm::language_server::language_server::debug_print_sqfvm_vpath_start_parameters() {
    window_log(lsp::data::message_type::Log, [&](auto& sstream) {
        sstream << "SQF-VM VPath start parameters: ";
        for (auto& [physical, virtual_]: m_sqfvm_factory.get_mappings()) {
            if (physical.empty() || virtual_.empty())
                continue;
            sstream << " -v \"" << physical << "|" << virtual_ << "\"";
        }
    });
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
                in_range = in_range || change.start_line <= params.range.start.line
                                       && change.start_column <= params.range.start.character
                                       && change.end_line >= params.range.end.line
                                       && change.end_column >= params.range.end.character;
                auto change_path = sanitize_to_uri(change.path);
                auto document_uri = static_cast<::lsp::data::document_uri>(change_path.full());
                auto lsp_file_version = m_versions.contains(document_uri)
                                        ? std::optional<::lsp::data::integer>{m_versions.at(document_uri)}
                                        : std::nullopt;
                switch (change.operation) {
                    case t_code_action_change::file_change:
                        out_changes.emplace_back(text_document_edit{
                                .textDocument = optional_versioned_text_document_identifier{
                                        .version = lsp_file_version,
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
                                        .version = lsp_file_version,
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

std::optional<lsp::data::hover> sqfvm::language_server::language_server::on_textDocument_hover(
        const lsp::data::hover_params &params) {
    using namespace ::lsp::data;
    using namespace std::string_literals;

    auto path = std::filesystem::path(
            std::string(params.text_document.uri.path().begin(),
                        params.text_document.uri.path().end()))
            .lexically_normal();
    auto file_opt = get_file_from_path(path.string(), false);
    if (!file_opt.has_value())
        return std::nullopt;
    auto file = file_opt.value();
    auto hovers = m_context->storage().get_all<database::tables::t_hover>(
            where(c(&database::tables::t_hover::file_fk) == file.id_pk
                  && c(&database::tables::t_hover::start_line) <= params.position.line + 1
                  && c(&database::tables::t_hover::start_column) <= params.position.character + 1
                  && c(&database::tables::t_hover::end_line) >= params.position.line + 1
                  && c(&database::tables::t_hover::end_column) >= params.position.character + 1));
    if (hovers.empty())
        return std::nullopt;

    std::vector<hover> out_hovers;
    for (const auto &hover: hovers) {
        out_hovers.emplace_back(
                markup_content{markup_kind::Markdown, hover.markdown},
                range{
                        .start = position{
                                .line = hover.start_line,
                                .character = hover.start_column,
                        },
                        .end = position{
                                .line = hover.end_line,
                                .character = hover.end_column,
                        },
                });
    }

    if (out_hovers.size() == 1)
        return out_hovers.front();

    std::stringstream sstream;
    position smallest_start = out_hovers.front().range->start;
    position largest_end = out_hovers.front().range->end;

    for (const auto &hover: out_hovers) {
        if (hover.range->start.line < smallest_start.line ||
            (hover.range->start.line == smallest_start.line && hover.range->start.character < smallest_start.character))
            smallest_start = hover.range->start;
        if (hover.range->end.line > largest_end.line ||
            (hover.range->end.line == largest_end.line && hover.range->end.character > largest_end.character))
            largest_end = hover.range->end;
        if (sstream.tellp() > 0)
            sstream << "\n\n";
        sstream << hover.contents.value;
    }

    return hover{
            .contents = markup_content{markup_kind::Markdown, sstream.str()},
            .range = range{
                    .start = smallest_start,
                    .end = largest_end,
            },
    };
}
