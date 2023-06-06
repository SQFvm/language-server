#ifndef SQFVM_LANGUAGE_SERVER_DATABASE_TABLES_T_FILE_H
#define SQFVM_LANGUAGE_SERVER_DATABASE_TABLES_T_FILE_H

#include <cstdint>
#include <string>

namespace sqfvm::language_server::database::tables {
    // Represents a file in the workspace in a virtual way.
    struct t_file {
        static constexpr const char *table_name = "tFile";
        enum file_flags {
            none = 0x00,

            // Reserved for future use
            reserved_1 = 0x01,

            // The file is outdated and should be reanalyzed.
            outdated = 0x02,

            // The file is deleted and should be removed from the database.
            deleted = 0x04,

            all = ~none,
        };

        // The primary key of this t_file
        uint64_t id_pk;

        // Unix-Timestamp this file was last changed
        uint64_t last_changed;

        // The physical path, relative to the workspace, of this file.
        std::string path;

        // The file-flags, relevant to the system.
        file_flags flags;
    };
}


#endif //SQFVM_LANGUAGE_SERVER_DATABASE_TABLES_T_FILE_H
