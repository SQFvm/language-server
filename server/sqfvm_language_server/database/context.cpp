//
// Created by marco.silipo on 17.08.2023.
//
#include "context.hpp"
#include "../util.hpp"

using namespace sqlite_orm;
using namespace ::sqfvm::language_server::database::tables;

void sqfvm::language_server::database::context::db_clear() {
    m_storage.remove_all<internal::t_db_generation>();
    m_storage.remove_all<t_diagnostic>();
    m_storage.remove_all<t_reference>();
    m_storage.remove_all<t_variable>();
    m_storage.remove_all<t_file_history>();
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