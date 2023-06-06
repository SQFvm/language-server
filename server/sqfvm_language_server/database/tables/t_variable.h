#ifndef SQFVM_LANGUAGE_SERVER_DATABASE_TABLES_T_VARIABLE_H
#define SQFVM_LANGUAGE_SERVER_DATABASE_TABLES_T_VARIABLE_H

#include <cstdint>
#include <string>
#include <optional>

namespace sqfvm::language_server::database::tables {
    // Represents an entry of a variable, referable by multiple t_file's using t_reference
    struct t_variable {
        static constexpr const char *table_name = "tVariable";

        // The primary key of this t_variable.
        uint64_t id_pk;

        // The name of this t_variable.
        std::string name;

        // Foreign Key of the t_scope this t_variable belongs to.
        uint64_t scope_fk;

    };
}


#endif //SQFVM_LANGUAGE_SERVER_DATABASE_TABLES_T_VARIABLE_H
