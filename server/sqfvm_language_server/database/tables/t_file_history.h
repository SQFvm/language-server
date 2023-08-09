#ifndef SQFVM_LANGUAGE_SERVER_DATABASE_TABLES_T_FILE_HISTORY_H
#define SQFVM_LANGUAGE_SERVER_DATABASE_TABLES_T_FILE_HISTORY_H

#include <cstdint>
#include <string>

namespace sqfvm::language_server::database::tables {
// Represents the content of a file at a specific point in time.
    struct t_file_history {
        static constexpr const char *table_name = "tFileHistory";
        // The primary key of this t_reference.
        uint64_t id_pk;

        // Foreign key referring to the t_file this belongs to.
        uint64_t file_fk;

        // The type of access done.
        std::string content;

        // Unix-Timestamp this change was recorded.
        uint64_t time_stamp_created;

        // Whether this change was provided by the file system or language server.
        // External changes are those which are detected by the file system watcher and done by version control for example.
        // Internal changes are those which are detected by the language server and done by the user.
        bool is_external;
    };
}


#endif //SQFVM_LANGUAGE_SERVER_DATABASE_TABLES_T_FILE_HISTORY_H
