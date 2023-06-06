#ifndef SQFVM_LANGUAGE_SERVER_DATABASE_TABLES_T_REFERENCE_H
#define SQFVM_LANGUAGE_SERVER_DATABASE_TABLES_T_REFERENCE_H

#include <cstdint>
#include <string>

namespace sqfvm::lsp::database::tables {
// Represents a variable in code
    struct t_reference {
        static constexpr const char *table_name = "tReference";
        enum class type_flags {
            none = 0x0000,

            code = 0x0001,
            scalar = 0x0002,
            boolean = 0x0004,
            object = 0x0008,
            hashmap = 0x0010,
            array = 0x0020,
            string = 0x0040,

            any = 0xFFFF,
        };
        enum class access_flags {
            none = 0x00,

            // The reference is received
            get = 0x01,

            // The reference is updated
            set = 0x02,

            all = get | set
        };

        // The primary key of this t_reference.
        uint64_t id_pk;

        // Foreign key referring to the t_file this belongs to.
        uint64_t file_fk;

        // Foreign key referring to the t_variable this refers to.
        uint64_t variable_fk;

        // The type of access done.
        access_flags access;

        // The line of this reference in the t_file referred to via file_fk.
        uint64_t line;

        // The column of this reference in the t_file referred to via file_fk.
        uint64_t column;

        // The column of this reference in the t_file referred to via file_fk.
        uint64_t offset;

        // The length of this reference in the t_file referred to via file_fk.
        uint64_t length;

        // The type this t_reference refers to.
        type_flags types;
    };
}


#endif //SQFVM_LANGUAGE_SERVER_DATABASE_TABLES_T_REFERENCE_H
