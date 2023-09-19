//
// Created by marco.silipo on 17.08.2023.
//
#include "context.hpp"

#include <utility>
#include "../util.hpp"

using namespace sqlite_orm;
using namespace ::sqfvm::language_server::database::tables;

void sqfvm::language_server::database::context::db_clear() {
    m_storage.remove_all<internal::t_db_generation>();
    m_storage.remove_all<t_diagnostic>();
    m_storage.remove_all<t_reference>();
    m_storage.remove_all<t_variable>();
    m_storage.remove_all<t_code_action_change>();
    m_storage.remove_all<t_code_action>();
    m_storage.remove_all<t_hover>();
    m_storage.remove_all<t_file_history>();
    m_storage.remove_all<t_file>();
}


std::optional<t_file> sqfvm::language_server::database::context::db_get_file_from_path(
        std::filesystem::path path,
        bool create_if_not_exists) {
    auto orm = storage();
    path = path.lexically_normal();
    auto files = orm.get_all<t_file>(where(c(&t_file::path) == path.string()));
    t_file file;
    if (files.empty()) {
        if (!create_if_not_exists)
            return {};
        file = t_file{
                .is_outdated = true,
                .is_deleted = false,
                .last_changed = unix_timestamp(),
                .path = path.string(),
        };
        auto result = orm.insert(file);
        file.id_pk = result;
    } else {
        file = files.front();
        if (file.is_deleted && exists(path)) {
            file.is_deleted = false;
            orm.update(file);
        }
    }
    return {file};
}

#pragma region DB Operations
using namespace sqfvm::language_server::database;
using namespace sqfvm::language_server::database::tables;
namespace {
    context::operations::success_t log_on_error_or_true(
            const sqfvm::language_server::database::context::operations::errlogfnc_t &log,
            const std::function<void()> &fnc,
            const std::function<void(std::stringstream &sstream)> &operation) {
        try {
            fnc();
            return static_cast<context::operations::success_t>(true);
        } catch (const std::exception &e) {
            std::stringstream sstream;
            sstream << "The database operation:\n";
            operation(sstream);
            sstream << "\n failed with '" << e.what() << "'.";
            log(e.what());
            return static_cast<context::operations::success_t>(false);
        }
    }

    template<typename T>
    std::pair<context::operations::success_t, T> log_on_error_or_pair(
            const sqfvm::language_server::database::context::operations::errlogfnc_t &log,
            const std::function<T()> &fnc,
            const std::function<void(std::stringstream &sstream)> &operation) {
        try {
            return std::make_pair(static_cast<context::operations::success_t>(true), fnc());
        } catch (const std::exception &e) {
            std::stringstream sstream;
            sstream << "The database operation:\n";
            operation(sstream);
            sstream << "\n failed with '" << e.what() << "'.";
            log(e.what());
            return std::make_pair(static_cast<context::operations::success_t>(false), fnc());
        }
    }

    template<typename T>
    nlohmann::json to_json(const T &t) {
        return nlohmann::json(t);
    }

    template<>
    nlohmann::json to_json(const t_file &t) {
        return nlohmann::json{
                {"id_pk",        t.id_pk},
                {"path",         t.path},
                {"is_outdated",  t.is_outdated},
                {"is_deleted",   t.is_deleted},
                {"is_deleted",   t.is_ignored},
                {"last_changed", t.last_changed},
        };
    }

    template<>
    nlohmann::json to_json(const t_variable &t) {
        return nlohmann::json{
                {"id_pk",          t.id_pk},
                {"variable_name",  t.variable_name},
                {"scope",          t.scope},
                {"opt_file_fk",    t.opt_file_fk.has_value() ? nlohmann::json(nullptr) : nlohmann::json(t.opt_file_fk.value())}
        };
    }
    template<>
    nlohmann::json to_json(const t_file_history &t) {
        return nlohmann::json{
                {"id_pk",               t.id_pk},
                {"file_fk",             t.file_fk},
                {"content",             t.content},
                {"time_stamp_created",  t.time_stamp_created},
                {"is_external",         t.is_external},
        };
    }
}

context::operations::success_t context::operations::mark_all_files_as_deleted(
        context &self,
        const errlogfnc_t &fnc) {
    return log_on_error_or_true(
            fnc,
            [&]() {
                auto orm = self.storage();
                orm.update_all(set(c(&t_file::is_deleted) = true));
            }, [&](auto &sstream) {
                sstream << "mark_all_files_as_deleted()";
            });
}

std::pair<context::operations::success_t, std::optional<t_file>> context::operations::find_file_by_path(
        context &self,
        const context::operations::errlogfnc_t &fnc,
        std::filesystem::path fpath) {
    return log_on_error_or_pair<std::optional<t_file>>(
            fnc,
            [&]() -> std::optional<t_file> {
                auto orm = self.storage();
                auto files = orm.get_all<t_file>(where(c(&t_file::path) == fpath.string()));
                if (files.empty())
                    return std::nullopt;
                return files.front();
            },
            [&](auto &sstream) {
                sstream << "find_file_by_path(\n"
                           "    fpath: " << fpath.string() << "\n)";
            });
}

context::operations::success_t context::operations::update(
        context &self,
        const context::operations::errlogfnc_t &fnc,
        tables::t_file file) {
    return log_on_error_or_true(
            fnc,
            [&]() {
                auto orm = self.storage();
                orm.update(file);
            }, [&](auto &sstream) {
                sstream << "update(\n"
                           "    file: " << to_json(file) << "\n)";
            });
}

context::operations::success_t context::operations::insert(
        context &self,
        const context::operations::errlogfnc_t &fnc,
        tables::t_file file) {
    return log_on_error_or_true(
            fnc,
            [&]() {
                auto orm = self.storage();
                orm.insert(file);
            }, [&](auto &sstream) {
                sstream << "insert(\n"
                           "    file: " << to_json(file) << "\n)";
            });
}

context::operations::success_t context::operations::delete_files_flagged_with_is_deleted(
        context &self,
        const context::operations::errlogfnc_t &fnc) {
    return log_on_error_or_true(
            fnc,
            [&]() {
                auto orm = self.storage();
                orm.remove_all<t_file>(where(c(&t_file::is_deleted) == true));
            }, [&](auto &sstream) {
                sstream << "delete_files_flagged_with_is_deleted()";
            });
}

context::operations::success_t context::operations::for_each_file_not_outdated(
        context &self,
        const context::operations::errlogfnc_t &fnc,
        const std::function<abort_t(const tables::t_file &)> &fnc2) {
    return log_on_error_or_true(
            fnc,
            [&]() {
                auto orm = self.storage();
                for (const auto &file: orm.get_all<t_file>(where(c(&t_file::is_outdated) == false))) {
                    auto result = fnc2(file);
                    if (result)
                        break;
                }
            }, [&](auto &sstream) {
                sstream << "for_each_file_not_outdated(...)";
            });
}

context::operations::success_t context::operations::for_each_file_outdated_and_not_deleted(
        context &self,
        const context::operations::errlogfnc_t &fnc,
        const std::function<abort_t(const tables::t_file &)> &fnc2) {
    return log_on_error_or_true(
            fnc,
            [&]() {
                auto orm = self.storage();
                for (const auto &file: orm.get_all<t_file>(
                        where(c(&t_file::is_outdated) == true && c(&t_file::is_deleted) == false))) {
                    auto result = fnc2(file);
                    if (result)
                        break;
                }
            }, [&](auto &sstream) {
                sstream << "for_each_file_outdated_and_not_deleted(...)";
            });
}

std::pair<context::operations::success_t, std::vector<t_reference>>
context::operations::find_all_references_by_file_and_line(
        context &self,
        const context::operations::errlogfnc_t &fnc,
        const t_file &file,
        uint64_t line,
        bool exclude_magical) {
    return find_all_references_by_file_and_line(self, fnc, file.id_pk, line, exclude_magical);
}

std::pair<context::operations::success_t, std::vector<tables::t_reference>> context::operations::find_all_references_by_file_and_line(
        context &self,
        const context::operations::errlogfnc_t &fnc,
        uint64_t file_id,
        uint64_t line,
        bool exclude_magical) {
    if (exclude_magical) {
        return log_on_error_or_pair<std::vector<t_reference>>(
                fnc,
                [&]() -> std::vector<t_reference> {
                    auto orm = self.storage();
                    return orm.get_all<t_reference>(
                            where(c(&t_reference::file_fk) == file_id
                                  && c(&t_reference::line) == line
                                  && c(&t_reference::is_magic_variable) == false));
                },
                [&](auto &sstream) {
                    sstream << "find_all_references_by_file_and_line(\n"
                            << "    file_id: " << file_id << ",\n"
                            << "    line: " << line << ",\n"
                            << "    exclude_magical: " << exclude_magical << "\n"
                            << ")";
                });
    } else {
        return log_on_error_or_pair<std::vector<t_reference>>(
                fnc,
                [&]() -> std::vector<t_reference> {
                    auto orm = self.storage();
                    return orm.get_all<t_reference>(
                            where(c(&t_reference::file_fk) == file_id
                                  && c(&t_reference::line) == line));
                },
                [&](auto &sstream) {
                    sstream << "find_all_references_by_file_and_line(\n"
                            << "    file_id: " << file_id << ",\n"
                            << "    line: " << line << ",\n"
                            << "    exclude_magical: " << exclude_magical << "\n"
                            << ")";
                });
    }
}

std::pair<context::operations::success_t, std::vector<tables::t_reference>> context::operations::get_all_variables_of_variable(
        context &self,
        const context::operations::errlogfnc_t &fnc,
        const t_variable &variable) {
    return get_all_variables_of_variable(self, fnc, variable.id_pk);
}

std::pair<context::operations::success_t, std::vector<tables::t_reference>>
context::operations::get_all_variables_of_variable(
        context &self,
        const context::operations::errlogfnc_t &fnc,
        uint64_t variable_id) {
    return log_on_error_or_pair<std::vector<t_reference>>(
            fnc,
            [&]() -> std::vector<t_reference> {
                auto orm = self.storage();
                return orm.get_all<t_reference>(where(c(&t_reference::variable_fk) == variable_id));
            },
            [&](auto &sstream) {
                sstream << "get_all_variables_of_variable(\n"
                        << "    variable_id: " << variable_id << "\n"
                        << ")";
            });
}

std::pair<context::operations::success_t, std::optional<tables::t_file>> context::operations::find_file_by_id(
        context &self,
        const context::operations::errlogfnc_t &fnc,
        uint64_t id) {
    return log_on_error_or_pair<std::optional<t_file>>(
            fnc,
            [&]() -> std::optional<t_file> {
                auto orm = self.storage();
                auto file_opt = orm.get_optional<t_file>(id);
                return file_opt;
            },
            [&](auto &sstream) {
                sstream << "find_file_by_id(\n"
                        << "    id: " << id << "\n"
                        << ")";
            });
}

context::operations::success_t context::operations::insert(
        context &self,
        const context::operations::errlogfnc_t &fnc,
        tables::t_file_history file_history) {
    return log_on_error_or_true(
            fnc,
            [&]() {
                auto orm = self.storage();
                orm.insert(file_history);
            }, [&](auto &sstream) {
                sstream << "insert(\n"
                        << "    file_history: " << to_json(file_history) << "\n"
                        << ")";
            });
}

#pragma endregion