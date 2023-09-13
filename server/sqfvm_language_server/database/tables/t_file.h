#ifndef SQFVM_LANGUAGE_SERVER_DATABASE_TABLES_T_FILE_H
#define SQFVM_LANGUAGE_SERVER_DATABASE_TABLES_T_FILE_H

#include <cstdint>
#include <string>

namespace sqfvm::language_server::database::tables {
    // Represents a file in the workspace in a virtual way.
    struct t_file {
        static constexpr const char *table_name = "tFile";

        // The file is outdated and should be reanalyzed.
        bool is_outdated;

        // The file is deleted and should be removed from the database.
        bool is_deleted;

        // The file is ignored and should be ignored by the language server.
        bool is_ignored;

        // The primary key of this t_file
        uint64_t id_pk;

        // Unix-Timestamp this file was last changed
        uint64_t last_changed;

        // The physical path, relative to the workspace, of this file.
        std::string path;
    };
}


#endif //SQFVM_LANGUAGE_SERVER_DATABASE_TABLES_T_FILE_H
