#ifndef SQFVM_LANGUAGE_SERVER_DATABASE_TABLES_T_SCOPE_H
#define SQFVM_LANGUAGE_SERVER_DATABASE_TABLES_T_SCOPE_H

#include <cstdint>
#include <string>
#include <optional>

namespace sqfvm::lsp::database::tables {
    // Represents a variable scope, allowing to specify exactly in which scope a t_variable lives.
    struct t_scope {
        static constexpr const char *table_name = "tScope";

        // The primary key of this t_scope.
        uint64_t id_pk;

        // Foreign key referring to the t_file this belongs to.
        uint64_t file_fk;

        // An optional parent-scope.
        std::optional<uint64_t> opt_parent_scope_fk;

    };
}


#endif //SQFVM_LANGUAGE_SERVER_DATABASE_TABLES_T_SCOPE_H
