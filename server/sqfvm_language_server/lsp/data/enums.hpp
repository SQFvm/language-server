//
// Created by marco.silipo on 20.08.2023.
//

#ifndef SQFVM_LANGUAGE_SERVER_ENUMS_HPP
#define SQFVM_LANGUAGE_SERVER_ENUMS_HPP

#include <optional>
#include <string>
#include <nlohmann/json.hpp>
#include <vector>
#include <variant>

namespace lsp {
    namespace data {
        enum class resource_operations {
            Empty = 0b000,
            /*
               Supports creating new files and folders.
            */
            Create = 0b001,
            /*
               Supports renaming existing files and folders.
            */
            Rename = 0b010,
            /*
               Supports deleting existing files and folders.
            */
            Delete = 0b100
        };

        inline resource_operations operator|(resource_operations lhs, resource_operations rhs) {
            return static_cast<resource_operations> (
                    static_cast<std::underlying_type<resource_operations>::type>(lhs)
                    | static_cast<std::underlying_type<resource_operations>::type>(rhs)
            );
        }

        inline resource_operations operator&(resource_operations lhs, resource_operations rhs) {
            return static_cast<resource_operations> (
                    static_cast<std::underlying_type<resource_operations>::type>(lhs)
                    & static_cast<std::underlying_type<resource_operations>::type>(rhs)
            );
        }

        enum class failure_handling {
            empty,
            /*
               Applying the workspace change is simply aborted if one of the changes provided
               fails. All operations executed before the failing operation stay executed.
            */
            abort,
            /*
               All operations are executed transactional. That means they either all
               succeed or no changes at all are applied to the workspace.
            */
            transactional,
            /*
               If the workspace edit contains only textual file changes they are executed transactional.
               If resource changes (create, rename or delete file) are part of the change the failure
               handling strategy is abort.
            */
            textOnlyTransactional,
            /*
               The client tries to undo the operations already executed. But there is no
               guarantee that this is succeeding.
            */
            undo
        };
        enum class folding_range_kind {
            /**
             * Folding range for a comment
             * 'comment'
             */
            Comment,
            /**
             * Folding range for a imports or includes
             * 'imports'
             */
            Imports,
            /**
              * Folding range for a region (e.g. `#region`)
              * 'region'
              */
            Region
        };
        enum class symbol_kind {
            File = 1,
            Module = 2,
            Namespace = 3,
            Package = 4,
            Class = 5,
            Method = 6,
            Property = 7,
            Field = 8,
            Constructor = 9,
            Enum = 10,
            Interface = 11,
            Function = 12,
            Variable = 13,
            Constant = 14,
            String = 15,
            Number = 16,
            Boolean = 17,
            Array = 18,
            Object = 19,
            Key = 20,
            Null = 21,
            EnumMember = 22,
            Struct = 23,
            Event = 24,
            Operator = 25,
            TypeParameter = 26
        };

        enum class message_type {
            /**
             * An error message.
             */
            Error = 1,
            /**
             * A warning message.
             */
            Warning = 2,
            /**
              * An information message.
              */
            Info = 3,
            /**
               * A log message.
               */
            Log = 4
        };

        enum class trace_mode {
            off, message, verbose
        };

        enum class completion_item_tag {
            Deprecated = 1
        };

        /**
         * Describes the content type that a client supports in various
         * result literals like `Hover`, `ParameterInfo` or `CompletionItem`.
         *
         * Please note that `MarkupKinds` must not start with a `$`. This kinds
         * are reserved for internal usage.
         */
        enum class markup_kind {
            /**
             * Plain text is supported as a content format
             * Value String: "plaintext"
             */
            PlainText, /**
             * Markdown is supported as a content format
             * Value String: "markdown
             */
            Markdown
        };

        enum class completion_item_kind {
            Text = 1,
            Method = 2,
            Function = 3,
            Constructor = 4,
            Field = 5,
            Variable = 6,
            Class = 7,
            Interface = 8,
            Module = 9,
            Property = 10,
            Unit = 11,
            Value = 12,
            Enum = 13,
            Keyword = 14,
            Snippet = 15,
            Color = 16,
            File = 17,
            Reference = 18,
            Folder = 19,
            EnumMember = 20,
            Constant = 21,
            Struct = 22,
            Event = 23,
            Operator = 24,
            TypeParameter = 25
        };

        enum class diagnostic_tag {
            /**
             * Unused or unnecessary code.
             *
             * Clients are allowed to render diagnostics with this tag faded out instead of having
             * an error squiggle.
             */
            Unnecessary = 1, /**
             * Deprecated or obsolete code.
             *
             * Clients are allowed to rendered diagnostics with this tag strike through.
             */
            Deprecated = 2
        };


        enum class code_action_kind {
            /**
             * Empty kind.
             */
            Empty,

            /**
             * Base kind for quickfix actions: 'quickfix'.
             */
            QuickFix,

            /**
             * Base kind for refactoring actions: 'refactor'.
             */
            Refactor,

            /**
             * Base kind for refactoring extraction actions: 'refactor.extract'.
             *
             * Example extract actions:
             *
             * - Extract method
             * - Extract function
             * - Extract variable
             * - Extract interface from class
             * - ...
             */
            RefactorExtract,

            /**
             * Base kind for refactoring inline actions: 'refactor.inline'.
             *
             * Example inline actions:
             *
             * - Inline function
             * - Inline variable
             * - Inline constant
             * - ...
             */
            RefactorInline,

            /**
             * Base kind for refactoring rewrite actions: 'refactor.rewrite'.
             *
             * Example rewrite actions:
             *
             * - Convert JavaScript function to class
             * - Add or remove parameter
             * - Encapsulate field
             * - Make method static
             * - Move method to base class
             * - ...
             */
            RefactorRewrite,

            /**
             * Base kind for source actions: `source`.
             *
             * Source code actions apply to the entire file.
             */
            Source,

            /**
             * Base kind for an organize imports source action: `source.organizeImports`.
             */
            SourceOrganizeImports
        };

        enum class text_document_sync_kind {
            /**
            * Documents should not be synced at all.
            */
            None = 0,

            /**
            * Documents are synced by always sending the full content
            * of the document.
            */
            Full = 1,

            /**
            * Documents are synced by sending the full content on open.
            * After that only incremental updates to the document are
            * send.
            */
            Incremental = 2
        };

        /**
         * How a completion was triggered
         */
        enum class completion_trigger_kind {
            /**
             * Completion was triggered by typing an identifier (24x7 code
             * complete), manual invocation (e.g Ctrl+Space) or via API.
             */
            Invoked = 1,

            /**
             * Completion was triggered by a trigger character specified by
             * the `triggerCharacters` properties of the `CompletionRegistrationOptions`.
             */
            TriggerCharacter = 2,

            /**
             * Completion was re-triggered as the current completion list is incomplete.
             */
            TriggerForIncompleteCompletions = 3
        };

        /**
         * Defines whether the insert text in a completion item should be interpreted as
         * plain text or a snippet.
         */
        enum class insert_text_format {
            /**
             * The primary text to be inserted is treated as a plain string.
             */
            PlainText = 1,

            /**
             * The primary text to be inserted is treated as a snippet.
             *
             * A snippet can define tab stops and placeholders with `$1`, `$2`
             * and `${3:foo}`. `$0` defines the final tab stop, it defaults to
             * the end of the snippet. Placeholders with equal identifiers are linked,
             * that is typing in one will update others too.
             */
            Snippet = 2
        };

        enum class diagnostic_severity {
            /**
             * Reports an error.
             */
            Error = 1, /**
             * Reports a warning.
             */
            Warning = 2, /**
              * Reports an information.
              */
            Information = 3, /**
               * Reports a hint.
               */
            Hint = 4
        };

        enum class text_document_save_reason {
            /**
             * Manually triggered, e.g. by the user pressing save, by starting debugging,
             * or by an API call.
             */
            Manual = 1, /**
             * Automatic after a delay.
             */
            AfterDelay = 2, /**
              * When the editor lost focus.
              */
            FocusOut = 3,
        };

        /**
         * The reason why code actions were requested.
         *
         * @since 3.17.0
         */
        enum class code_action_trigger_kind {
            /**
             * Code actions were explicitly requested by the user or by an extension.
             */
            Invoked = 1,
            /**
             * Code actions were requested automatically.
             *
             * This typically happens when current selection in a file changes, but can
             * also be triggered when file content changes.
             */
            Automatic = 2,
        };
    }
}

#endif //SQFVM_LANGUAGE_SERVER_ENUMS_HPP
