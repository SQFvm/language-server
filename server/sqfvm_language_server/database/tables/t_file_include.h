#ifndef SQFVM_LANGUAGE_SERVER_DATABASE_TABLES_T_FILE_INCLUDE_H
#define SQFVM_LANGUAGE_SERVER_DATABASE_TABLES_T_FILE_INCLUDE_H

#include <cstdint>

namespace sqfvm::language_server::database::tables {
// Represents the content of a file at a specific point in time.
    struct t_file_include {
        static constexpr const char *table_name = "tFileInclude";
        // The primary key of this t_reference.
        uint64_t id_pk;

        // Foreign key referring to the t_file this belongs to.
        uint64_t file_included_fk;

        // Foreign key referring to the t_file this was included in.
        uint64_t file_included_in_fk;

        // Foreign key referring to the t_file which owns this t_file_include.
        uint64_t source_file_fk;
    };
}


#endif //SQFVM_LANGUAGE_SERVER_DATABASE_TABLES_T_FILE_INCLUDE_H
