#ifndef SQFVM_LANGUAGE_SERVER_DATABASE_TABLES_T_CODE_ACTION_CHANGE_H
#define SQFVM_LANGUAGE_SERVER_DATABASE_TABLES_T_CODE_ACTION_CHANGE_H

#include <cstdint>
#include <string>
#include <optional>

namespace sqfvm::language_server::database::tables {
    // Represents a file in the workspace in a virtual way.
    struct t_code_action_change {
        static constexpr const char *table_name = "tCodeActionChange";
        enum file_operation {
            file_change,
            file_create,
            file_delete,
            file_rename,
        };

        // The primary key of this t_file
        uint64_t id_pk;

        // The code action this change belongs to.
        uint64_t code_action_fk;

        // The kind of change this is.
        file_operation operation;


        // If kind is file_rename, this is the current path of the file.
        // Otherwise, this is empty.
        std::optional<std::string> old_path;

        // If kind is file_rename, this is the new path of the file.
        // If kind is file_create, this is the path of the file to create.
        // If kind is file_delete, this is the path of the file to delete.
        // If kind is file_change, this is the path of the file to change.
        std::string path;


        // If kind is file_change, this is the line where this change starts in the file.
        // Otherwise, this is empty.
        std::optional<uint64_t> start_line;

        // If kind is file_change, this is the column where this change starts in the file.
        // Otherwise, this is empty.
        std::optional<uint64_t> start_column;

        // If kind is file_change, this is the line where this change ends in the file.
        // Otherwise, this is empty.
        std::optional<uint64_t> end_line;

        // If kind is file_change, this is the column where this change ends in the file.
        // Otherwise, this is empty.
        std::optional<uint64_t> end_column;

        // If kind is file_change, this is the new content to be placed in place of start_line and start_column to end_line and end_column.
        // if kind is file_create, this is the content of the file to create.
        // Otherwise, this is empty.
        std::optional<std::string> content;
    };
}


#endif //SQFVM_LANGUAGE_SERVER_DATABASE_TABLES_T_CODE_ACTION_CHANGE_H
