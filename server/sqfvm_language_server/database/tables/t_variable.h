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

        // Either the scope-qualified name of this t_variable, or the namespace this t_variable belongs to.
        std::string scope;

        // The file this t_variable belongs to. nullopt if this t_variable is a global.
        std::optional<uint64_t> opt_file_fk;
    };
}


#endif //SQFVM_LANGUAGE_SERVER_DATABASE_TABLES_T_VARIABLE_H
