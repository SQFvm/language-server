#ifndef SQFVM_LANGUAGE_SERVER_DATABASE_TABLES_T_CODE_ACTION_H
#define SQFVM_LANGUAGE_SERVER_DATABASE_TABLES_T_CODE_ACTION_H

#include <cstdint>
#include <string>

namespace sqfvm::language_server::database::tables {
    // Represents a file in the workspace in a virtual way.
    struct t_code_action {
        static constexpr const char *table_name = "tCodeAction";
        enum action_kind {

            /**
             * Generic actions that don't fit into any other category.
             */
            generic,

            /**
             * Fixes for a specific problem.
             */
            quick_fix,

            /**
             * Generic refactorings.
             */
            refactor,

            /**
             * - Extract method
             * - Extract function
             * - Extract variable
             * - Extract interface from class
             * - ...
             */
            extract_refactor,

            /**
             * - Inline function
             * - Inline variable
             * - Inline constant
             * - ...
             */
            inline_refactor,

            /**
             * - Convert JavaScript function to class
             * - Add or remove parameter
             * - Encapsulate field
             * - Make method static
             * - Move method to base class
             * - ...
             */
            rewrite_refactor,

            /**
             * Source code actions apply to the entire file.
             */
            whole_file,
        };

        // The primary key of this t_file
        uint64_t id_pk;

        // Foreign key referring to the t_file this belongs to.
        uint64_t file_fk;

        // Foreign key referring to the t_file this belongs to.
        action_kind kind;

        // Identifier to group actions by.
        std::string identifier;

        // Text to present the user describing the action.
        std::string text;
    };
}


#endif //SQFVM_LANGUAGE_SERVER_DATABASE_TABLES_T_CODE_ACTION_H
