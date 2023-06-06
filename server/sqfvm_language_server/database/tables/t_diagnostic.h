#ifndef SQFVM_LANGUAGE_SERVER_DATABASE_TABLES_T_DIAGNOSTIC_H
#define SQFVM_LANGUAGE_SERVER_DATABASE_TABLES_T_DIAGNOSTIC_H

#include <cstdint>
#include <string>

namespace sqfvm::language_server::database::tables {
    // Represents a file in the workspace in a virtual way.
    struct t_diagnostic {
        static constexpr const char *table_name = "tDiagnostic";
        enum severity_level {
            fatal,
            error,
            warning,
            info,
            verbose,
            trace
        };

        // The primary key of this t_file
        uint64_t id_pk;

        // Foreign key referring to the t_file this belongs to.
        uint64_t file_fk;

        // The line of this diagnostic in the t_file referred to via file_fk.
        uint64_t line;

        // The column of this diagnostic in the t_file referred to via file_fk.
        uint64_t column;

        // The column of this diagnostic in the t_file referred to via file_fk.
        uint64_t offset;

        // The length of this diagnostic in the t_file referred to via file_fk.
        uint64_t length;

        // The severity of this diagnostic
        severity_level severity;

        // The diagnostic message
        std::string message;

        // The content at the position of the diagnostic, if available
        std::string content;

        // The code of the diagnostic message, if applicable
        std::string code;
    };
}


#endif //SQFVM_LANGUAGE_SERVER_DATABASE_TABLES_T_DIAGNOSTIC_H
