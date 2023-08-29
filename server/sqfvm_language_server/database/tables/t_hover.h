#ifndef SQFVM_LANGUAGE_SERVER_DATABASE_TABLES_T_HOVER_H
#define SQFVM_LANGUAGE_SERVER_DATABASE_TABLES_T_HOVER_H

#include <cstdint>
#include <string>

namespace sqfvm::language_server::database::tables {
    // Represents hover information for a file.
    struct t_hover {
        static constexpr const char *table_name = "tHover";

        // The primary key of this t_hover
        uint64_t id_pk;

        // Foreign key referring to the t_file this belongs to.
        uint64_t file_fk;

        // The line where this hover starts in the t_file referred to via file_fk.
        uint64_t start_line;

        // The column where this hover starts in the t_file referred to via file_fk.
        uint64_t start_column;

        // The line where this hover ends in the t_file referred to via file_fk.
        uint64_t end_line;

        // The column where this hover ends in the t_file referred to via file_fk.
        uint64_t end_column;

        // The markdown content of this hover.
        std::string markdown;
    };
}


#endif //SQFVM_LANGUAGE_SERVER_DATABASE_TABLES_T_HOVER_H
