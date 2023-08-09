#ifndef SQFVM_LANGUAGE_SERVER_LSP_LSPSERVER_HPP
#define SQFVM_LANGUAGE_SERVER_LSP_LSPSERVER_HPP

#include "jsonrpc.hpp"
#include "../uri.hpp"

#include <optional>
#include <string>
#include <nlohmann/json.hpp>
#include <vector>
#include <variant>

namespace lsp {
    namespace data {
        template<typename T>
        inline void from_json(const nlohmann::json &node, T &t) {
            t = T::from_json(node);
        }

        template<>
        inline void from_json<bool>(const nlohmann::json &node, bool &t) {
            t = node;
        }

        template<>
        inline void from_json<int>(const nlohmann::json &node, int &t) {
            t = node;
        }

        template<>
        inline void from_json<unsigned int>(const nlohmann::json &node, unsigned int &t) {
            t = node;
        }

        template<>
        inline void from_json<float>(const nlohmann::json &node, float &t) {
            t = node;
        }

        template<>
        inline void from_json<uint64_t>(const nlohmann::json &node, uint64_t &t) {
            t = node;
        }

        template<>
        inline void from_json<std::string>(const nlohmann::json &node, std::string &t) {
            t = node;
        }

        template<typename T>
        inline nlohmann::json to_json(const T &t) {
            return t.to_json();
        }

        template<>
        inline nlohmann::json to_json<bool>(const bool &t) {
            return t;
        }

        template<>
        inline nlohmann::json to_json<int>(const int &t) {
            return t;
        }

        template<>
        inline nlohmann::json to_json<unsigned int>(const unsigned int &t) {
            return t;
        }

        template<>
        inline nlohmann::json to_json<float>(const float &t) {
            return t;
        }

        template<>
        inline nlohmann::json to_json<uint64_t>(const uint64_t &t) {
            return t;
        }

        template<>
        inline nlohmann::json to_json<std::string>(const std::string &t) {
            return t;
        }


        enum class resource_operations {
            Empty = 0b000, /*
               Supports creating new files and folders.
            */
            Create = 0b001, /*
               Supports renaming existing files and folders.
            */
            Rename = 0b010, /*
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

        template<>
        inline void from_json<resource_operations>(const nlohmann::json &node, resource_operations &t) {
            t = resource_operations::Empty;
            for (const auto &resource_operation_json: node) {
                std::string resource_operation_string = resource_operation_json;
                if (resource_operation_string == "create") {
                    t = t | resource_operations::Create;
                } else if (resource_operation_string == "rename") {
                    t = t | resource_operations::Rename;
                } else if (resource_operation_string == "delete") {
                    t = t | resource_operations::Delete;
                }
            }
        }

        template<>
        inline nlohmann::json to_json<resource_operations>(const resource_operations &t) {
            nlohmann::json arr;
            if ((t & resource_operations::Create) == resource_operations::Create) {
                arr.push_back("create");
            }
            if ((t & resource_operations::Delete) == resource_operations::Delete) {
                arr.push_back("delete");
            }
            if ((t & resource_operations::Rename) == resource_operations::Rename) {
                arr.push_back("rename");
            }
            return arr;
        }

        enum class failure_handling {
            empty, /*
               Applying the workspace change is simply aborted if one of the changes provided
               fails. All operations executed before the failing operation stay executed.
            */
            abort, /*
               All operations are executed transactional. That means they either all
               succeed or no changes at all are applied to the workspace.
            */
            transactional, /*
               If the workspace edit contains only textual file changes they are executed transactional.
               If resource changes (create, rename or delete file) are part of the change the failure
               handling strategy is abort.
            */
            textOnlyTransactional, /*
               The client tries to undo the operations already executed. But there is no
               guarantee that this is succeeding.
            */
            undo
        };

        template<>
        inline void from_json<failure_handling>(const nlohmann::json &node, failure_handling &t) {

            t = failure_handling::empty;
            std::string actual = node;
            if (actual == "abort") {
                t = failure_handling::abort;
                return;
            }
            if (actual == "transactional") {
                t = failure_handling::transactional;
                return;
            }
            if (actual == "textOnlyTransactional") {
                t = failure_handling::textOnlyTransactional;
                return;
            }
            if (actual == "undo") {
                t = failure_handling::undo;
                return;
            }
        }

        template<>
        inline nlohmann::json to_json<failure_handling>(const failure_handling &t) {
            switch (t) {
                case failure_handling::abort:
                    return "abort";
                case failure_handling::transactional:
                    return "transactional";
                case failure_handling::textOnlyTransactional:
                    return "textOnlyTransactional";
                case failure_handling::undo:
                    return "undo";
                default:
                    return {};
            }
        }

        enum class folding_range_kind {
            /**
             * Folding range for a comment
             * 'comment'
             */
            Comment, /**
             * Folding range for a imports or includes
             * 'imports'
             */
            Imports, /**
              * Folding range for a region (e.g. `#region`)
              * 'region'
              */
            Region
        };

        template<>
        inline void from_json<folding_range_kind>(const nlohmann::json &node, folding_range_kind &t) {
            t = folding_range_kind::Comment;
            std::string actual = node;
            if (actual == "comment") {
                t = folding_range_kind::Comment;
                return;
            }
            if (actual == "imports") {
                t = folding_range_kind::Imports;
                return;
            }
            if (actual == "region") {
                t = folding_range_kind::Region;
                return;
            }
        }

        template<>
        inline nlohmann::json to_json<folding_range_kind>(const folding_range_kind &t) {
            switch (t) {
                case folding_range_kind::Comment:
                    return "comment";
                case folding_range_kind::Imports:
                    return "imports";
                case folding_range_kind::Region:
                    return "region";
                default:
                    return {};
            }
        }

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

        template<>
        inline void from_json<symbol_kind>(const nlohmann::json &node, symbol_kind &t) {
            t = static_cast<symbol_kind>(node.get<int>());
        }

        template<>
        inline nlohmann::json to_json<symbol_kind>(const symbol_kind &t) {
            return static_cast<int>(t);
        }

        enum class message_type {
            /**
             * An error message.
             */
            Error = 1, /**
             * A warning message.
             */
            Warning = 2, /**
              * An information message.
              */
            Info = 3, /**
               * A log message.
               */
            Log = 4
        };

        template<>
        inline void from_json<message_type>(const nlohmann::json &node, message_type &t) {
            t = static_cast<message_type>(node.get<int>());
        }

        template<>
        inline nlohmann::json to_json<message_type>(const message_type &t) {
            return static_cast<int>(t);
        }

        enum class trace_mode {
            off, message, verbose
        };

        template<>
        inline void from_json<trace_mode>(const nlohmann::json &node, trace_mode &t) {

            t = trace_mode::off;
            std::string actual = node;
            if (actual == "message") {
                t = trace_mode::message;
                return;
            }
            if (actual == "verbose") {
                t = trace_mode::verbose;
                return;
            }
        }

        template<>
        inline nlohmann::json to_json<trace_mode>(const trace_mode &t) {
            switch (t) {
                case trace_mode::off:
                    return "off";
                case trace_mode::message:
                    return "message";
                case trace_mode::verbose:
                    return "verbose";
                default:
                    return {};
            }
        }

        enum class completion_item_tag {
            Deprecated = 1
        };

        template<>
        inline void from_json<completion_item_tag>(const nlohmann::json &node, completion_item_tag &t) {
            t = static_cast<completion_item_tag>(node.get<int>());
        }

        template<>
        inline nlohmann::json to_json<completion_item_tag>(const completion_item_tag &t) {
            return static_cast<int>(t);
        }

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

        template<>
        inline void from_json<markup_kind>(const nlohmann::json &node, markup_kind &t) {

            t = markup_kind::PlainText;
            std::string actual = node;
            if (actual == "plaintext") {
                t = markup_kind::PlainText;
                return;
            }
            if (actual == "markdown") {
                t = markup_kind::Markdown;
                return;
            }
        }

        template<>
        inline nlohmann::json to_json<markup_kind>(const markup_kind &t) {
            switch (t) {
                case markup_kind::PlainText:
                    return "plaintext";
                case markup_kind::Markdown:
                    return "markdown";
                default:
                    return {};
            }
        }

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

        template<>
        inline void from_json<completion_item_kind>(const nlohmann::json &node, completion_item_kind &t) {
            t = static_cast<completion_item_kind>(node.get<int>());
        }

        template<>
        inline nlohmann::json to_json<completion_item_kind>(const completion_item_kind &t) {
            return static_cast<int>(t);
        }

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

        template<>
        inline void from_json<diagnostic_tag>(const nlohmann::json &node, diagnostic_tag &t) {
            t = static_cast<diagnostic_tag>(node.get<int>());
        }

        template<>
        inline nlohmann::json to_json<diagnostic_tag>(const diagnostic_tag &t) {
            return static_cast<int>(t);
        }

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

        template<>
        inline void from_json<code_action_kind>(const nlohmann::json &node, code_action_kind &t) {

            t = code_action_kind::Empty;
            std::string actual = node;
            if (actual == "quickfix") {
                t = code_action_kind::QuickFix;
                return;
            }
            if (actual == "refactor") {
                t = code_action_kind::Refactor;
                return;
            }
            if (actual == "refactor.extract") {
                t = code_action_kind::RefactorExtract;
                return;
            }
            if (actual == "refactor.inline") {
                t = code_action_kind::RefactorInline;
                return;
            }
            if (actual == "refactor.rewrite") {
                t = code_action_kind::RefactorRewrite;
                return;
            }
            if (actual == "source") {
                t = code_action_kind::Source;
                return;
            }
            if (actual == "source.organizeImports") {
                t = code_action_kind::SourceOrganizeImports;
                return;
            }
        }

        template<>
        inline nlohmann::json to_json<code_action_kind>(const code_action_kind &t) {
            switch (t) {
                case code_action_kind::QuickFix:
                    return "quickfix";
                case code_action_kind::Refactor:
                    return "refactor";
                case code_action_kind::RefactorExtract:
                    return "refactor.extract";
                case code_action_kind::RefactorInline:
                    return "refactor.inline";
                case code_action_kind::RefactorRewrite:
                    return "refactor.rewrite";
                case code_action_kind::Source:
                    return "source";
                case code_action_kind::SourceOrganizeImports:
                    return "source.organizeImports";
                default:
                    return {};
            }
        }

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

        template<>
        inline void from_json<text_document_sync_kind>(const nlohmann::json &node, text_document_sync_kind &t) {
            t = static_cast<text_document_sync_kind>(node.get<int>());
        }

        template<>
        inline nlohmann::json to_json<text_document_sync_kind>(const text_document_sync_kind &t) {
            return static_cast<int>(t);
        }

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

        template<>
        inline void from_json<text_document_save_reason>(const nlohmann::json &node, text_document_save_reason &t) {
            t = static_cast<text_document_save_reason>(node.get<int>());
        }

        template<>
        inline nlohmann::json to_json<text_document_save_reason>(const text_document_save_reason &t) {
            return static_cast<int>(t);
        }

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

        template<>
        inline void from_json<completion_trigger_kind>(const nlohmann::json &node, completion_trigger_kind &t) {
            t = static_cast<completion_trigger_kind>(node.get<int>());
        }

        template<>
        inline nlohmann::json to_json<completion_trigger_kind>(const completion_trigger_kind &t) {
            return static_cast<int>(t);
        }

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

        template<>
        inline void from_json<insert_text_format>(const nlohmann::json &node, insert_text_format &t) {
            t = static_cast<insert_text_format>(node.get<int>());
        }

        template<>
        inline nlohmann::json to_json<insert_text_format>(const insert_text_format &t) {
            return static_cast<int>(t);
        }

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

        template<>
        inline void from_json<diagnostic_severity>(const nlohmann::json &node, diagnostic_severity &t) {
            t = static_cast<diagnostic_severity>(node.get<int>());
        }

        template<>
        inline nlohmann::json to_json<diagnostic_severity>(const diagnostic_severity &t) {
            return static_cast<int>(t);
        }

        template<typename T>
        inline void from_json(const nlohmann::json &node, std::vector<T> &ts) {
            ts = std::vector<T>();
            for (const auto &subnode: node) {
                T t;
                from_json(subnode, t);
                ts.push_back(t);
            }
        }

        template<typename T>
        inline void from_json(const nlohmann::json &node, const char *key, T &t) {
            from_json(node[key], t);
        }

        template<typename T>
        inline void from_json(const nlohmann::json &node, const char *key, std::optional<T> &opt) {
            if (node.contains(key) && !node[key].is_null()) {
                T t{};
                from_json(node, key, t);
                opt = t;
            } else {
                opt = {};
            }
        }

        template<typename T>
        inline nlohmann::json to_json(const std::vector<T> &ts) {
            nlohmann::json json = nlohmann::json::array();
            for (const auto &t: ts) {
                json.push_back(to_json(t));
            }
            return json;
        }

        template<typename T>
        inline nlohmann::json to_json(const std::optional<T> &t) {
            if (t.has_value()) {
                return to_json(t.value());
            } else {
                return nullptr;
            }
        }

        template<typename T>
        inline void set_json(nlohmann::json &json, const char *key, const std::optional<T> &t) {
            if (t.has_value()) {
                json[key] = to_json(t.value());
            }
        }

        template<typename T>
        inline void set_json(nlohmann::json &json, const char *key, const T &t) {
            json[key] = to_json(t);
        }

        struct uri :
                public ::x39::uri {
            uri() : ::x39::uri() {
            }

            explicit uri(const std::string &input) : ::x39::uri(input) {
            }

            explicit uri(std::string_view input) : ::x39::uri(input) {
            }

            uri(std::string_view schema, std::string_view user, std::string_view password, std::string_view host,
                std::string_view port, std::string_view path, std::string_view query, std::string_view fragment)
                    : ::x39::uri(schema, user, password, host, port, path, query, fragment) {
            }

            static uri from_json(const nlohmann::json &node) {
                return uri(node.get<std::string>());
            }

            [[nodiscard]] nlohmann::json to_json() const {
                return encoded();
            }
        };

        /*
        * A document filter denotes a document through properties like language, scheme or pattern.
        * An example is a filter that applies to TypeScript files on disk.
        * Another example is a filter the applies to JSON files with name package.json:
        */
        struct document_filter {
            /**
             * A language id, like `typescript`.
             */
            std::string language;

            /**
             * A Uri [scheme](#Uri.scheme), like `file` or `untitled`.
             */
            std::string scheme;

            /**
             * A glob pattern, like `*.{ts,js}`.
             *
             * Glob patterns can have the following syntax:
             * - `*` to match one or more characters in a path segment
             * - `?` to match on one character in a path segment
             * - `**` to match any number of path segments, including none
             * - `{}` to group conditions (e.g. `**?/*.{ts,js}` matches all TypeScript and JavaScript files)
             * - `[]` to declare a range of characters to match in a path segment (e.g., `example.[0-9]` to match on `example.0`, `example.1`, …)
             * - `[!...]` to negate a range of characters to match in a path segment (e.g., `example.[!0-9]` to match on `example.a`, `example.b`, but not `example.0`)
             */
            std::string pattern;

            static document_filter from_json(const nlohmann::json &node) {
                document_filter res;
                data::from_json(node, "language", res.language);
                data::from_json(node, "scheme", res.scheme);
                data::from_json(node, "pattern", res.pattern);
                return res;
            }

            [[nodiscard]] nlohmann::json to_json() const {
                nlohmann::json json;
                data::set_json(json, "language", language);
                data::set_json(json, "scheme", scheme);
                data::set_json(json, "pattern", pattern);
                return json;
            }
        };

        struct position {
            size_t line;
            size_t character;

            static position from_json(const nlohmann::json &node) {
                position res{};
                data::from_json(node, "line", res.line);
                data::from_json(node, "character", res.character);
                return res;
            }

            [[nodiscard]] nlohmann::json to_json() const {
                nlohmann::json json;
                data::set_json(json, "line", line);
                data::set_json(json, "character", character);
                return json;
            }
        };

        struct range {
            position start;
            position end;

            static range from_json(const nlohmann::json &node) {
                range res{};
                data::from_json(node, "start", res.start);
                data::from_json(node, "end", res.end);
                return res;
            }

            [[nodiscard]] nlohmann::json to_json() const {
                nlohmann::json json;
                data::set_json(json, "start", start);
                data::set_json(json, "end", end);
                return json;
            }
        };

        struct text_edit {
            range range{};
            std::string new_text;

            static text_edit from_json(const nlohmann::json &node) {
                text_edit res;
                data::from_json(node, "range", res.range);
                data::from_json(node, "newText", res.new_text);
                return res;
            }

            [[nodiscard]] nlohmann::json to_json() const {
                nlohmann::json json;
                data::set_json(json, "range", range);
                data::set_json(json, "newText", new_text);
                return json;
            }
        };

        struct text_document_identifier {
            uri uri;

            static text_document_identifier from_json(const nlohmann::json &node) {
                text_document_identifier res;
                data::from_json(node, "uri", res.uri);
                return res;
            }

            [[nodiscard]] nlohmann::json to_json() const {
                nlohmann::json json;
                data::set_json(json, "uri", uri);
                return json;
            }
        };

        struct text_document_item {
            /**
             * The text document's URI.
             */
            uri uri;
            /**
             * The text document's language identifier.
             */
            std::string languageId;
            /**
             * The version number of this document (it will increase after each
             * change, including undo/redo).
             */
            size_t version{};
            /**
             * The content of the opened text document.
             */
            std::string text;

            static text_document_item from_json(const nlohmann::json &node) {
                text_document_item res;
                data::from_json(node, "uri", res.uri);
                data::from_json(node, "languageId", res.languageId);
                data::from_json(node, "version", res.version);
                data::from_json(node, "text", res.text);
                return res;
            }

            [[nodiscard]] nlohmann::json to_json() const {
                nlohmann::json json;
                data::set_json(json, "uri", uri);
                data::set_json(json, "languageId", languageId);
                data::set_json(json, "version", version);
                data::set_json(json, "text", text);
                return json;
            }
        };

        struct versioned_text_document_identifier {
            uri uri;
            /**
             * The version number of this document. If a versioned text document identifier
             * is sent from the server to the client and the file is not open in the editor
             * (the server has not received an open notification before) the server can send
             * `null` to indicate that the version is known and the content on disk is the
             * master (as speced with document content ownership).
             *
             * The version number of a document will increase after each change, including
             * undo/redo. The number doesn't need to be consecutive.
             */
            std::optional<size_t> version;

            static versioned_text_document_identifier from_json(const nlohmann::json &node) {
                versioned_text_document_identifier res;
                data::from_json(node, "uri", res.uri);
                data::from_json(node, "version", res.version);
                return res;
            }

            [[nodiscard]] nlohmann::json to_json() const {
                nlohmann::json json;
                data::set_json(json, "uri", uri);
                data::set_json(json, "version", version);
                return json;
            }
        };

        /**
         * A `MarkupContent` literal represents a string value which content is interpreted base on its
         * kind flag. Currently the protocol supports `plaintext` and `markdown` as markup kinds.
         *
         * If the kind is `markdown` then the value can contain fenced code blocks like in GitHub issues.
         * See https://help.github.com/articles/creating-and-highlighting-code-blocks/#syntax-highlighting
         *
         * Here is an example how such a string can be constructed using JavaScript / TypeScript:
         * ```typescript
         * let markdown: MarkdownContent = {
         *  kind: MarkupKind.Markdown,
         *	value: [
         *		'# Header',
         *		'Some text',
         *		'```typescript',
         *		'someCode();',
         *		'```'
         *	].join('\n')
         * };
         * ```
         *
         * *Please Note* that clients might sanitize the return markdown. A client could decide to
         * remove HTML from the markdown to avoid script execution.
         */
        struct markup_content {
            /**
             * The type of the Markup
             */
            markup_kind kind = markup_kind::PlainText;

            /**
             * The content itself
             */
            std::string value;

            static markup_content from_json(const nlohmann::json &node) {
                markup_content res;
                data::from_json(node, "kind", res.kind);
                data::from_json(node, "value", res.value);
                return res;
            }

            [[nodiscard]] nlohmann::json to_json() const {
                nlohmann::json json;
                data::set_json(json, "kind", kind);
                data::set_json(json, "value", value);
                return json;
            }
        };

        struct location {
            uri uri;
            range range{};

            static location from_json(const nlohmann::json &node) {
                location res;
                data::from_json(node, "uri", res.uri);
                data::from_json(node, "range", res.range);
                return res;
            }

            [[nodiscard]] nlohmann::json to_json() const {
                nlohmann::json json;
                data::set_json(json, "uri", uri);
                data::set_json(json, "range", range);
                return json;
            }
        };

        struct diagnostics {
            struct diagnostic_related_information {
                /**
                 * The location of this related diagnostic information.
                 */
                location location;

                /**
                 * The message of this related diagnostic information.
                 */
                std::string message;

                static diagnostic_related_information from_json(const nlohmann::json &node) {
                    diagnostic_related_information res;
                    data::from_json(node, "location", res.location);
                    data::from_json(node, "message", res.message);
                    return res;
                }

                [[nodiscard]] nlohmann::json to_json() const {
                    nlohmann::json json;
                    data::set_json(json, "location", location);
                    data::set_json(json, "message", message);
                    return json;
                }
            };

            /**
             * The range at which the message applies.
             */
            range range{};

            /**
             * The diagnostic's severity. Can be omitted. If omitted it is up to the
             * client to interpret diagnostics as error, warning, info or hint.
             */
            std::optional<diagnostic_severity> severity;

            /**
             * The diagnostic's code, which might appear in the user interface.
             */
            std::optional<std::string> code;

            /**
             * A human-readable string describing the source of this
             * diagnostic, e.g. 'typescript' or 'super lint'.
             */
            std::optional<std::string> source;

            /**
             * The diagnostic's message.
             */
            std::string message;

            /**
             * Additional metadata about the diagnostic.
             *
             * @since 3.15.0
             */
            std::optional<std::vector<diagnostic_tag>> tags;

            /**
             * An array of related diagnostic information, e.g. when symbol-names within
             * a scope collide all definitions can be marked via this property.
             */
            std::optional<std::vector<diagnostic_related_information>> relatedInformation;

            static diagnostics from_json(const nlohmann::json &node) {
                diagnostics res;
                data::from_json(node, "range", res.range);
                data::from_json(node, "severity", res.severity);
                data::from_json(node, "code", res.code);
                data::from_json(node, "source", res.source);
                data::from_json(node, "message", res.message);
                data::from_json(node, "tags", res.tags);
                data::from_json(node, "relatedInformation", res.relatedInformation);
                return res;
            }

            [[nodiscard]] nlohmann::json to_json() const {
                nlohmann::json json;
                data::set_json(json, "range", range);
                data::set_json(json, "severity", severity);
                data::set_json(json, "code", code);
                data::set_json(json, "source", source);
                data::set_json(json, "message", message);
                data::set_json(json, "tags", tags);
                data::set_json(json, "relatedInformation", relatedInformation);
                return json;
            }
        };

        struct folding_range {
            /**
             * The zero-based line number from where the folded range starts.
             */
            size_t startLine{};

            /**
             * The zero-based character offset from where the folded range starts. If not defined, defaults to the length of the start line.
             */
            std::optional<size_t> startCharacter;

            /**
             * The zero-based line number where the folded range ends.
             */
            size_t endLine{};

            /**
             * The zero-based character offset before the folded range ends. If not defined, defaults to the length of the end line.
             */
            std::optional<size_t> endCharacter;

            /**
             * Describes the kind of the folding range such as `comment` or `region`. The kind
             * is used to categorize folding ranges and used by commands like 'Fold all comments'. See
             * [FoldingRangeKind](#FoldingRangeKind) for an enumeration of standardized kinds.
             */
            std::optional<folding_range_kind> kind;

            static struct folding_range from_json(const nlohmann::json &node) {
                struct folding_range res;
                data::from_json(node, "startLine", res.startLine);
                data::from_json(node, "startCharacter", res.startCharacter);
                data::from_json(node, "endLine", res.endLine);
                data::from_json(node, "endCharacter", res.endCharacter);
                data::from_json(node, "kind", res.kind);
                return res;
            }

            [[nodiscard]] nlohmann::json to_json() const {
                nlohmann::json json;
                data::set_json(json, "startLine", startLine);
                data::set_json(json, "startCharacter", startCharacter);
                data::set_json(json, "endLine", endLine);
                data::set_json(json, "endCharacter", endCharacter);
                data::set_json(json, "kind", kind);
                return json;
            }
        };

        struct command {
            /**
             * Title of the command, like `save`.
             */
            std::string title;
            /**
             * The identifier of the actual command handler.
             */
            std::string command_identifier;
            /**
             * Arguments that the command handler should be
             * invoked with.
             */
            std::optional<std::vector<nlohmann::json>> arguments;

            static command from_json(const nlohmann::json &node) {
                command res;
                data::from_json(node, "title", res.title);
                data::from_json(node, "command", res.command_identifier);
                auto arguments_find_res = node.find("arguments");
                if (arguments_find_res != node.end()) {
                    res.arguments = std::vector<nlohmann::json>{};
                    for (auto it: (*arguments_find_res)) {
                        res.arguments->push_back(it);
                    }
                }
                return res;
            }

            [[nodiscard]] nlohmann::json to_json() const {
                nlohmann::json json;
                data::set_json(json, "title", title);
                data::set_json(json, "command", command_identifier);
                if (arguments.has_value()) {
                    json["arguments"] = arguments.value();
                }
                return json;
            }
        };

        struct completion_item {
            /**
             * The label of this completion item. By default
             * also the text that is inserted when selecting
             * this completion.
             */
            std::string label;

            /**
             * The kind of this completion item. Based of the kind
             * an icon is chosen by the editor. The standardized set
             * of available values is defined in `CompletionItemKind`.
             */
            std::optional<size_t> kind;

            /**
             * Tags for this completion item.
             *
             * @since 3.15.0
             */
            std::optional<std::vector<completion_item_tag>> tags;

            /**
             * A human-readable string with additional information
             * about this item, like type or symbol information.
             */
            std::optional<std::string> detail;

            /**
             * A human-readable string that represents a doc-comment.
             */
            std::optional<markup_content> documentation;

            /**
             * Indicates if this item is deprecated.
             *
             * @deprecated Use `tags` instead if supported.
             */
            std::optional<bool> deprecated;

            /**
             * Select this item when showing.
             *
             * *Note* that only one completion item can be selected and that the
             * tool / client decides which item that is. The rule is that the *first*
             * item of those that match best is selected.
             */
            std::optional<bool> preselect;

            /**
             * A string that should be used when comparing this item
             * with other items. When `falsy` the label is used.
             */
            std::optional<std::string> sort_text;

            /**
             * A string that should be used when filtering a set of
             * completion items. When `falsy` the label is used.
             */
            std::optional<std::string> filter_text;

            /**
             * A string that should be inserted into a document when selecting
             * this completion. When `falsy` the label is used.
             *
             * The `insertText` is subject to interpretation by the client side.
             * Some tools might not take the string literally. For example
             * VS Code when code complete is requested in this example `con<cursor position>`
             * and a completion item with an `insertText` of `console` is provided it
             * will only insert `sole`. Therefore it is recommended to use `textEdit` instead
             * since it avoids additional client side interpretation.
             */
            std::optional<std::string> insert_text;

            /**
             * The format of the insert text. The format applies to both the `insertText` property
             * and the `newText` property of a provided `textEdit`. If omitted defaults to
             * `InsertTextFormat.PlainText`.
             */
            std::optional<insert_text_format> insert_text_format;

            /**
             * An edit which is applied to a document when selecting this completion. When an edit is provided the value of
             * `insertText` is ignored.
             *
             * *Note:* The range of the edit must be a single line range and it must contain the position at which completion
             * has been requested.
             */
            std::optional<text_edit> textEdit;

            /**
             * An optional array of additional text edits that are applied when
             * selecting this completion. Edits must not overlap (including the same insert position)
             * with the main edit nor with themselves.
             *
             * Additional text edits should be used to change text unrelated to the current cursor position
             * (for example adding an import statement at the top of the file if the completion item will
             * insert an unqualified type).
             */
            std::optional<std::vector<text_edit>> additionalTextEdits;

            /**
             * An optional set of characters that when pressed while this completion is active will accept it first and
             * then type that character. *Note* that all commit characters should have `length=1` and that superfluous
             * characters will be ignored.
             */
            std::optional<std::vector<std::string>> commitCharacters;

            /**
             * An optional command that is executed *after* inserting this completion. *Note* that
             * additional modifications to the current document should be described with the
             * additionalTextEdits-property.
             */
            std::optional<command> command;

            /**
             * A data entry field that is preserved on a completion item between
             * a completion and a completion resolve request.
             */
            std::optional<nlohmann::json> data;

            static completion_item from_json(const nlohmann::json &node) {
                completion_item res;
                data::from_json(node, "label", res.label);
                data::from_json(node, "kind", res.kind);
                data::from_json(node, "tags", res.tags);
                data::from_json(node, "detail", res.detail);
                data::from_json(node, "documentation", res.documentation);
                data::from_json(node, "deprecated", res.deprecated);
                data::from_json(node, "preselect", res.preselect);
                data::from_json(node, "sortText", res.sort_text);
                data::from_json(node, "filterText", res.filter_text);
                data::from_json(node, "insertText", res.insert_text);
                data::from_json(node, "insertTextFormat", res.insert_text_format);
                data::from_json(node, "textEdit", res.textEdit);
                data::from_json(node, "additionalTextEdits", res.additionalTextEdits);
                data::from_json(node, "commitCharacters", res.commitCharacters);
                data::from_json(node, "command", res.command);
                res.data = node.contains("data") ? node["data"] : nlohmann::json(nullptr);
                return res;
            }

            [[nodiscard]] nlohmann::json to_json() const {
                nlohmann::json json;
                data::set_json(json, "label", label);
                data::set_json(json, "kind", kind);
                data::set_json(json, "tags", tags);
                data::set_json(json, "detail", detail);
                data::set_json(json, "documentation", documentation);
                data::set_json(json, "deprecated", deprecated);
                data::set_json(json, "preselect", preselect);
                data::set_json(json, "sortText", sort_text);
                data::set_json(json, "filterText", filter_text);
                data::set_json(json, "insertText", insert_text);
                data::set_json(json, "insertTextFormat", insert_text_format);
                data::set_json(json, "textEdit", textEdit);
                data::set_json(json, "additionalTextEdits", additionalTextEdits);
                data::set_json(json, "commitCharacters", commitCharacters);
                data::set_json(json, "command", command);
                if (data.has_value()) {
                    json["data"] = *data;
                }
                return json;
            }
        };

        /**
         * Represents a collection of [completion items](#CompletionItem) to be presented
         * in the editor.
         */
        struct completion_list {
            /**
            * This list it not complete. Further typing should result in recomputing
            * this list.
            */
            bool isIncomplete;

            /**
            * The completion items.
            */
            std::vector<completion_item> items;

            static completion_list from_json(const nlohmann::json &node) {
                completion_list res;
                data::from_json(node, "isIncomplete", res.isIncomplete);
                data::from_json(node, "items", res.items);
                return res;
            }

            nlohmann::json to_json() const {
                nlohmann::json json;
                data::set_json(json, "isIncomplete", isIncomplete);
                data::set_json(json, "items", items);
                return json;
            }
        };

        struct initialize_result {
            struct server_capabilities {
                struct text_document_sync_options {
                    struct SaveOptions {
                        /**
                         * The client is supposed to include the content on save.
                         */
                        std::optional<bool> includeText;

                        static SaveOptions from_json(const nlohmann::json &node) {
                            SaveOptions res;
                            data::from_json(node, "includeText", res.includeText);
                            return res;
                        }

                        nlohmann::json to_json() const {
                            nlohmann::json json;
                            data::set_json(json, "includeText", includeText);
                            return json;
                        }
                    };

                    /**
                        * Open and close notifications are sent to the server. If omitted open close notification should not
                        * be sent.
                        */
                    std::optional<bool> openClose;
                    /**
                     * If present will save notifications are sent to the server. If omitted the notification should not be
                     * sent.
                     */
                    std::optional<bool> willSave;
                    /**
                     * If present will save wait until requests are sent to the server. If omitted the request should not be
                     * sent.
                     */
                    std::optional<bool> willSaveWaitUntil;
                    /**
                     * If present save notifications are sent to the server. If omitted the notification should not be
                     * sent.
                     */
                    std::optional<SaveOptions> save;

                    /**
                    * Change notifications are sent to the server. See TextDocumentSyncKind.None, TextDocumentSyncKind.Full
                    * and TextDocumentSyncKind.Incremental. If omitted it defaults to TextDocumentSyncKind.None.
                    */
                    std::optional<text_document_sync_kind> change;

                    static text_document_sync_options from_json(const nlohmann::json &node) {
                        text_document_sync_options res;
                        data::from_json(node, "openClose", res.openClose);
                        data::from_json(node, "willSave", res.willSave);
                        data::from_json(node, "willSaveWaitUntil", res.willSaveWaitUntil);
                        data::from_json(node, "save", res.save);
                        data::from_json(node, "change", res.change);
                        return res;
                    }

                    nlohmann::json to_json() const {
                        nlohmann::json json;
                        data::set_json(json, "openClose", openClose);
                        data::set_json(json, "willSave", willSave);
                        data::set_json(json, "willSaveWaitUntil", willSaveWaitUntil);
                        data::set_json(json, "change", change);
                        return json;
                    }
                };

                struct completion_options {
                    /**
                        * Most tools trigger completion request automatically without explicitly requesting
                        * it using a keyboard shortcut (e.g. Ctrl+Space). Typically they do so when the user
                        * starts to type an identifier. For example if the user types `c` in a JavaScript file
                        * code complete will automatically pop up present `console` besides others as a
                        * completion item. Characters that make up identifiers don't need to be listed here.
                        *
                        * If code complete should automatically be trigger on characters not being valid inside
                        * an identifier (for example `.` in JavaScript) list them in `triggerCharacters`.
                        */
                    std::optional<std::vector<std::string>> triggerCharacters;

                    /**
                        * The list of all possible characters that commit a completion. This field can be used
                        * if clients don't support individual commit characters per completion item. See
                        * `ClientCapabilities.textDocument.completion.completionItem.commitCharactersSupport`.
                        *
                        * If a server provides both `allCommitCharacters` and commit characters on an individual
                        * completion item the ones on the completion item win.
                        *
                        * @since 3.2.0
                        */
                    std::optional<std::vector<std::string>> allCommitCharacters;

                    /**
                        * The server provides support to resolve additional
                        * information for a completion item.
                        */
                    std::optional<bool> resolveProvider;

                    static completion_options from_json(const nlohmann::json &node) {
                        completion_options res;
                        data::from_json(node, "triggerCharacters", res.triggerCharacters);
                        data::from_json(node, "allCommitCharacters", res.allCommitCharacters);
                        data::from_json(node, "resolveProvider", res.resolveProvider);
                        return res;
                    }

                    nlohmann::json to_json() const {
                        nlohmann::json json;
                        data::set_json(json, "triggerCharacters", triggerCharacters);
                        data::set_json(json, "allCommitCharacters", allCommitCharacters);
                        data::set_json(json, "resolveProvider", resolveProvider);
                        return json;
                    }
                };

                struct hover_options {
                    std::optional<bool> workDoneProgress;

                    static hover_options from_json(const nlohmann::json &node) {
                        hover_options res;
                        data::from_json(node, "workDoneProgress", res.workDoneProgress);
                        return res;
                    }

                    nlohmann::json to_json() const {
                        nlohmann::json json;
                        data::set_json(json, "workDoneProgress", workDoneProgress);
                        return json;
                    }
                };

                struct signature_help_options {
                    std::optional<bool> workDoneProgress;
                    /**
                        * The characters that trigger signature help
                        * automatically.
                        */
                    std::optional<std::vector<std::string>> triggerCharacters;

                    /**
                        * List of characters that re-trigger signature help.
                        *
                        * These trigger characters are only active when signature help is already showing. All trigger characters
                        * are also counted as re-trigger characters.
                        *
                        * @since 3.15.0
                        */
                    std::optional<std::vector<std::string>> retriggerCharacters;

                    static signature_help_options from_json(const nlohmann::json &node) {
                        signature_help_options res;
                        data::from_json(node, "workDoneProgress", res.workDoneProgress);
                        data::from_json(node, "triggerCharacters", res.triggerCharacters);
                        data::from_json(node, "retriggerCharacters", res.retriggerCharacters);
                        return res;
                    }

                    nlohmann::json to_json() const {
                        nlohmann::json json;
                        data::set_json(json, "workDoneProgress", workDoneProgress);
                        data::set_json(json, "triggerCharacters", triggerCharacters);
                        data::set_json(json, "retriggerCharacters", retriggerCharacters);
                        return json;
                    }
                };

                struct declaration_registration_options {
                    std::optional<bool> workDoneProgress;
                    /**
                        * The id used to register the request. The id can be used to deregister
                        * the request again. See also Registration#id.
                        */
                    std::optional<std::string> id;
                    /**
                        * A document selector to identify the scope of the registration. If set to null
                        * the document selector provided on the client side will be used.
                        */
                    std::optional<document_filter> documentSelector;

                    static declaration_registration_options from_json(const nlohmann::json &node) {
                        declaration_registration_options res;
                        data::from_json(node, "workDoneProgress", res.workDoneProgress);
                        data::from_json(node, "id", res.id);
                        data::from_json(node, "documentSelector", res.documentSelector);
                        return res;
                    }

                    nlohmann::json to_json() const {
                        nlohmann::json json;
                        data::set_json(json, "workDoneProgress", workDoneProgress);
                        data::set_json(json, "id", id);
                        data::set_json(json, "documentSelector", documentSelector);
                        return json;
                    }
                };

                struct definition_options {
                    std::optional<bool> workDoneProgress;

                    static definition_options from_json(const nlohmann::json &node) {
                        definition_options res;
                        data::from_json(node, "workDoneProgress", res.workDoneProgress);
                        return res;
                    }

                    nlohmann::json to_json() const {
                        nlohmann::json json;
                        data::set_json(json, "workDoneProgress", workDoneProgress);
                        return json;
                    }
                };

                struct type_definition_registration_options {
                    std::optional<bool> workDoneProgress;
                    /**
                        * A document selector to identify the scope of the registration. If set to null
                        * the document selector provided on the client side will be used.
                        */
                    std::optional<document_filter> documentSelector;
                    /**
                        * The id used to register the request. The id can be used to deregister
                        * the request again. See also Registration#id.
                        */
                    std::optional<std::string> id;

                    static type_definition_registration_options from_json(const nlohmann::json &node) {
                        type_definition_registration_options res;
                        data::from_json(node, "workDoneProgress", res.workDoneProgress);
                        data::from_json(node, "documentSelector", res.documentSelector);
                        data::from_json(node, "id", res.id);
                        return res;
                    }

                    nlohmann::json to_json() const {
                        nlohmann::json json;
                        data::set_json(json, "workDoneProgress", workDoneProgress);
                        data::set_json(json, "documentSelector", documentSelector);
                        data::set_json(json, "id", id);
                        return json;
                    }
                };

                struct implementation_registration_options {
                    std::optional<bool> workDoneProgress;
                    /**
                        * A document selector to identify the scope of the registration. If set to null
                        * the document selector provided on the client side will be used.
                        */
                    std::optional<document_filter> documentSelector;
                    /**
                        * The id used to register the request. The id can be used to deregister
                        * the request again. See also Registration#id.
                        */
                    std::optional<std::string> id;

                    static implementation_registration_options from_json(const nlohmann::json &node) {
                        implementation_registration_options res;
                        data::from_json(node, "workDoneProgress", res.workDoneProgress);
                        data::from_json(node, "documentSelector", res.documentSelector);
                        data::from_json(node, "id", res.id);
                        return res;
                    }

                    nlohmann::json to_json() const {
                        nlohmann::json json;
                        data::set_json(json, "workDoneProgress", workDoneProgress);
                        data::set_json(json, "documentSelector", documentSelector);
                        data::set_json(json, "id", id);
                        return json;
                    }
                };

                struct reference_options {
                    std::optional<bool> workDoneProgress;

                    static reference_options from_json(const nlohmann::json &node) {
                        reference_options res;
                        data::from_json(node, "workDoneProgress", res.workDoneProgress);
                        return res;
                    }

                    nlohmann::json to_json() const {
                        nlohmann::json json;
                        data::set_json(json, "workDoneProgress", workDoneProgress);
                        return json;
                    }
                };

                struct document_highlight_options {
                    std::optional<bool> workDoneProgress;

                    static document_highlight_options from_json(const nlohmann::json &node) {
                        document_highlight_options res;
                        data::from_json(node, "workDoneProgress", res.workDoneProgress);
                        return res;
                    }

                    nlohmann::json to_json() const {
                        nlohmann::json json;
                        data::set_json(json, "workDoneProgress", workDoneProgress);
                        return json;
                    }
                };

                struct document_symbol_options {
                    std::optional<bool> workDoneProgress;

                    static document_symbol_options from_json(const nlohmann::json &node) {
                        document_symbol_options res;
                        data::from_json(node, "workDoneProgress", res.workDoneProgress);
                        return res;
                    }

                    nlohmann::json to_json() const {
                        nlohmann::json json;
                        data::set_json(json, "workDoneProgress", workDoneProgress);
                        return json;
                    }
                };

                struct code_action_options {
                    std::optional<bool> workDoneProgress;
                    /**
                        * CodeActionKinds that this server may return.
                        *
                        * The list of kinds may be generic, such as `CodeActionKind.Refactor`, or the server
                        * may list out every specific kind they provide.
                        */
                    std::optional<std::vector<code_action_kind>> codeActionKinds;

                    static code_action_options from_json(const nlohmann::json &node) {
                        code_action_options res;
                        data::from_json(node, "workDoneProgress", res.workDoneProgress);
                        data::from_json(node, "codeActionKinds", res.codeActionKinds);
                        return res;
                    }

                    nlohmann::json to_json() const {
                        nlohmann::json json;
                        data::set_json(json, "workDoneProgress", workDoneProgress);
                        data::set_json(json, "codeActionKinds", codeActionKinds);
                        return json;
                    }
                };

                struct code_lens_options {
                    std::optional<bool> workDoneProgress;
                    /**
                        * Code lens has a resolve provider as well.
                        */
                    std::optional<bool> resolveProvider;

                    static code_lens_options from_json(const nlohmann::json &node) {
                        code_lens_options res;
                        data::from_json(node, "workDoneProgress", res.workDoneProgress);
                        data::from_json(node, "resolveProvider", res.resolveProvider);
                        return res;
                    }

                    nlohmann::json to_json() const {
                        nlohmann::json json;
                        data::set_json(json, "workDoneProgress", workDoneProgress);
                        data::set_json(json, "resolveProvider", resolveProvider);
                        return json;
                    }
                };

                struct document_link_options {
                    std::optional<bool> workDoneProgress;
                    /**
                        * Code lens has a resolve provider as well.
                        */
                    std::optional<bool> resolveProvider;

                    static document_link_options from_json(const nlohmann::json &node) {
                        document_link_options res;
                        data::from_json(node, "workDoneProgress", res.workDoneProgress);
                        data::from_json(node, "resolveProvider", res.resolveProvider);
                        return res;
                    }

                    nlohmann::json to_json() const {
                        nlohmann::json json;
                        data::set_json(json, "workDoneProgress", workDoneProgress);
                        data::set_json(json, "resolveProvider", resolveProvider);
                        return json;
                    }
                };

                struct document_color_registration_options {
                    std::optional<bool> workDoneProgress;
                    /**
                        * A document selector to identify the scope of the registration. If set to null
                        * the document selector provided on the client side will be used.
                        */
                    std::optional<document_filter> documentSelector;
                    /**
                        * The id used to register the request. The id can be used to deregister
                        * the request again. See also Registration#id.
                        */
                    std::optional<std::string> id;

                    static document_color_registration_options from_json(const nlohmann::json &node) {
                        document_color_registration_options res;
                        data::from_json(node, "workDoneProgress", res.workDoneProgress);
                        data::from_json(node, "documentSelector", res.documentSelector);
                        data::from_json(node, "id", res.id);
                        return res;
                    }

                    nlohmann::json to_json() const {
                        nlohmann::json json;
                        data::set_json(json, "workDoneProgress", workDoneProgress);
                        data::set_json(json, "documentSelector", documentSelector);
                        data::set_json(json, "id", id);
                        return json;
                    }
                };

                struct document_formatting_options {
                    std::optional<bool> workDoneProgress;

                    static document_formatting_options from_json(const nlohmann::json &node) {
                        document_formatting_options res;
                        data::from_json(node, "workDoneProgress", res.workDoneProgress);
                        return res;
                    }

                    nlohmann::json to_json() const {
                        nlohmann::json json;
                        data::set_json(json, "workDoneProgress", workDoneProgress);
                        return json;
                    }
                };

                struct document_range_formatting_options {
                    std::optional<bool> workDoneProgress;

                    static document_range_formatting_options from_json(const nlohmann::json &node) {
                        document_range_formatting_options res;
                        data::from_json(node, "workDoneProgress", res.workDoneProgress);
                        return res;
                    }

                    nlohmann::json to_json() const {
                        nlohmann::json json;
                        data::set_json(json, "workDoneProgress", workDoneProgress);
                        return json;
                    }
                };

                struct document_on_type_formatting_options {
                    /**
                        * A character on which formatting should be triggered, like `}`.
                        */
                    std::string firstTriggerCharacter;

                    /**
                        * More trigger characters.
                        */
                    std::optional<std::vector<std::string>> moreTriggerCharacter;

                    static document_on_type_formatting_options from_json(const nlohmann::json &node) {
                        document_on_type_formatting_options res;
                        data::from_json(node, "firstTriggerCharacter", res.firstTriggerCharacter);
                        data::from_json(node, "moreTriggerCharacter", res.moreTriggerCharacter);
                        return res;
                    }

                    nlohmann::json to_json() const {
                        nlohmann::json json;
                        data::set_json(json, "firstTriggerCharacter", firstTriggerCharacter);
                        data::set_json(json, "moreTriggerCharacter", moreTriggerCharacter);
                        return json;
                    }
                };

                struct rename_options {
                    std::optional<bool> workDoneProgress;
                    /**
                        * Renames should be checked and tested before being executed.
                        */
                    std::optional<bool> prepareProvider;

                    static rename_options from_json(const nlohmann::json &node) {
                        rename_options res;
                        data::from_json(node, "workDoneProgress", res.workDoneProgress);
                        data::from_json(node, "prepareProvider", res.prepareProvider);
                        return res;
                    }

                    nlohmann::json to_json() const {
                        nlohmann::json json;
                        data::set_json(json, "workDoneProgress", workDoneProgress);
                        data::set_json(json, "prepareProvider", prepareProvider);
                        return json;
                    }
                };

                struct folding_range_registration_options {
                    std::optional<bool> workDoneProgress;
                    /**
                        * A document selector to identify the scope of the registration. If set to null
                        * the document selector provided on the client side will be used.
                        */
                    std::optional<document_filter> documentSelector;
                    /**
                        * The id used to register the request. The id can be used to deregister
                        * the request again. See also Registration#id.
                        */
                    std::optional<std::string> id;

                    static folding_range_registration_options from_json(const nlohmann::json &node) {
                        folding_range_registration_options res;
                        data::from_json(node, "workDoneProgress", res.workDoneProgress);
                        data::from_json(node, "documentSelector", res.documentSelector);
                        data::from_json(node, "id", res.id);
                        return res;
                    }

                    nlohmann::json to_json() const {
                        nlohmann::json json;
                        data::set_json(json, "workDoneProgress", workDoneProgress);
                        data::set_json(json, "documentSelector", documentSelector);
                        data::set_json(json, "workDoneProgress", workDoneProgress);
                        return json;
                    }
                };

                struct execute_command_options {
                    std::optional<bool> workDoneProgress;
                    /**
                        * The commands to be executed on the server.
                        */
                    std::optional<std::vector<std::string>> commands;

                    static execute_command_options from_json(const nlohmann::json &node) {
                        execute_command_options res;
                        data::from_json(node, "workDoneProgress", res.workDoneProgress);
                        data::from_json(node, "commands", res.commands);
                        return res;
                    }

                    nlohmann::json to_json() const {
                        nlohmann::json json;
                        data::set_json(json, "workDoneProgress", workDoneProgress);
                        data::set_json(json, "commands", commands);
                        return json;
                    }
                };

                struct selection_range_registration_options {
                    std::optional<bool> workDoneProgress;
                    /**
                        * A document selector to identify the scope of the registration. If set to null
                        * the document selector provided on the client side will be used.
                        */
                    std::optional<document_filter> documentSelector;
                    /**
                        * The id used to register the request. The id can be used to deregister
                        * the request again. See also Registration#id.
                        */
                    std::optional<std::string> id;

                    static selection_range_registration_options from_json(const nlohmann::json &node) {
                        selection_range_registration_options res;
                        data::from_json(node, "workDoneProgress", res.workDoneProgress);
                        data::from_json(node, "documentSelector", res.documentSelector);
                        data::from_json(node, "id", res.id);
                        return res;
                    }

                    nlohmann::json to_json() const {
                        nlohmann::json json;
                        data::set_json(json, "workDoneProgress", workDoneProgress);
                        data::set_json(json, "documentSelector", documentSelector);
                        data::set_json(json, "id", id);
                        return json;
                    }
                };

                struct workspace_folders_server_capabilities {
                    /**
                        * The server has support for workspace folders
                        */
                    std::optional<bool> supported;

                    /**
                        * Whether the server wants to receive workspace folder
                        * change notifications.
                        *
                        * If a string is provided, the string is treated as an ID
                        * under which the notification is registered on the client
                        * side. The ID can be used to unregister for these events
                        * using the `client/unregisterCapability` request.
                        *
                        * Implementors note: For simplicity (on my end ...), the option to use a string here was removed.
                        */
                    std::optional<bool> changeNotifications;

                    static workspace_folders_server_capabilities from_json(const nlohmann::json &node) {
                        workspace_folders_server_capabilities res;
                        data::from_json(node, "supported", res.supported);
                        data::from_json(node, "changeNotifications", res.changeNotifications);
                        return res;
                    }

                    nlohmann::json to_json() const {
                        nlohmann::json json;
                        data::set_json(json, "supported", supported);
                        data::set_json(json, "changeNotifications", changeNotifications);
                        return json;
                    }
                };

                struct Workspace {
                    /**
                        * The server supports workspace folder.
                        *
                        * @since 3.6.0
                        */
                    std::optional<workspace_folders_server_capabilities> workspaceFolders;

                    static Workspace from_json(const nlohmann::json &node) {
                        Workspace res;
                        data::from_json(node, "workspaceFolders", res.workspaceFolders);
                        return res;
                    }

                    nlohmann::json to_json() const {
                        nlohmann::json json;
                        data::set_json(json, "workspaceFolders", workspaceFolders);
                        return json;
                    }
                };

                /**
                    * Defines how text documents are synced. Is either a detailed structure defining each notification or
                    * for backwards compatibility the TextDocumentSyncKind number. If omitted it defaults to `TextDocumentSyncKind.None`.
                    *
                    * Implementors note: Technically, this should support `TextDocumentSyncOptions | number` for backwards compatibility ... but we ignore that simply because
                    *                    it already is hard enough to provide this shitfest of a protocol. No need to make it even harder to implement
                    *                    a server.
                    */
                std::optional<text_document_sync_options> textDocumentSync;

                /**
                    * The server provides completion support.
                    */
                std::optional<completion_options> completionProvider;

                /**
                    * The server provides hover support.
                    *
                    * Implementors note: Technically, this should support `boolean | HoverOptions` for backwards compatibility ... but we ignore that simply because
                    *                    it already is hard enough to provide this shitfest of a protocol. No need to make it even harder to implement
                    *                    a server.
                    */
                std::optional<hover_options> hoverProvider;

                /**
                    * The server provides signature help support.
                    */
                std::optional<signature_help_options> signatureHelpProvider;

                /**
                    * The server provides go to declaration support.
                    *
                    * @since 3.14.0
                    *
                    * Implementors note: Technically, this should support `boolean | DeclarationOptions | DeclarationRegistrationOptions` for backwards compatibility ... but we ignore that simply because
                    *                    it already is hard enough to provide this shitfest of a protocol. No need to make it even harder to implement
                    *                    a server.
                    */
                std::optional<declaration_registration_options> declarationProvider;

                /**
                    * The server provides goto definition support.
                    *
                    * Implementors note: Technically, this should support `boolean | DefinitionOptions` for backwards compatibility ... but we ignore that simply because
                    *                    it already is hard enough to provide this shitfest of a protocol. No need to make it even harder to implement
                    *                    a server.
                    */
                std::optional<definition_options> definitionProvider;

                /**
                    * The server provides goto type definition support.
                    *
                    * @since 3.6.0
                    *
                    * Implementors note: Technically, this should support `boolean | TypeDefinitionOptions | TypeDefinitionRegistrationOptions` for backwards compatibility ... but we ignore that simply because
                    *                    it already is hard enough to provide this shitfest of a protocol. No need to make it even harder to implement
                    *                    a server.
                    */
                std::optional<type_definition_registration_options> typeDefinitionProvider;;

                /**
                    * The server provides goto implementation support.
                    *
                    * @since 3.6.0
                    *
                    * Implementors note: Technically, this should support `boolean | ImplementationOptions | ImplementationRegistrationOptions` for backwards compatibility ... but we ignore that simply because
                    *                    it already is hard enough to provide this shitfest of a protocol. No need to make it even harder to implement
                    *                    a server.
                    */
                std::optional<implementation_registration_options> implementationProvider;

                /**
                    * The server provides find references support.
                    *
                    * Implementors note: Technically, this should support `boolean | ReferenceOptions` for backwards compatibility ... but we ignore that simply because
                    *                    it already is hard enough to provide this shitfest of a protocol. No need to make it even harder to implement
                    *                    a server.
                    */
                std::optional<reference_options> referencesProvider;

                /**
                    * The server provides document highlight support.
                    *
                    * Implementors note: Technically, this should support `boolean | DocumentHighlightOptions` for backwards compatibility ... but we ignore that simply because
                    *                    it already is hard enough to provide this shitfest of a protocol. No need to make it even harder to implement
                    *                    a server.
                    */
                std::optional<document_highlight_options> documentHighlightProvider;

                /**
                    * The server provides document symbol support.
                    *
                    * Implementors note: Technically, this should support `boolean | DocumentSymbolOptions` for backwards compatibility ... but we ignore that simply because
                    *                    it already is hard enough to provide this shitfest of a protocol. No need to make it even harder to implement
                    *                    a server.
                    */
                std::optional<document_symbol_options> documentSymbolProvider;

                /**
                    * The server provides code actions. The `CodeActionOptions` return type is only
                    * valid if the client signals code action literal support via the property
                    * `textDocument.codeAction.codeActionLiteralSupport`.
                    *
                    * Implementors note: Technically, this should support `boolean | CodeActionOptions` for backwards compatibility ... but we ignore that simply because
                    *                    it already is hard enough to provide this shitfest of a protocol. No need to make it even harder to implement
                    *                    a server.
                    */
                std::optional<code_action_options> codeActionProvider;

                /**
                    * The server provides code lens.
                    */
                std::optional<code_lens_options> codeLensProvider;

                /**
                    * The server provides document link support.
                    */
                std::optional<document_link_options> documentLinkProvider;

                /**
                    * The server provides color provider support.
                    *
                    * @since 3.6.0
                    *
                    * Implementors note: Technically, this should support `boolean | DocumentColorOptions | DocumentColorRegistrationOptions` for backwards compatibility ... but we ignore that simply because
                    *                    it already is hard enough to provide this shitfest of a protocol. No need to make it even harder to implement
                    *                    a server.
                    */
                std::optional<document_color_registration_options> colorProvider;

                /**
                    * The server provides document formatting.
                    *
                    * Implementors note: Technically, this should support `boolean | DocumentFormattingOptions` for backwards compatibility ... but we ignore that simply because
                    *                    it already is hard enough to provide this shitfest of a protocol. No need to make it even harder to implement
                    *                    a server.
                    */
                std::optional<document_formatting_options> documentFormattingProvider;

                /**
                    * The server provides document range formatting.
                    *
                    * Implementors note: Technically, this should support `boolean | DocumentRangeFormattingOptions` for backwards compatibility ... but we ignore that simply because
                    *                    it already is hard enough to provide this shitfest of a protocol. No need to make it even harder to implement
                    *                    a server.
                    */
                std::optional<document_range_formatting_options> documentRangeFormattingProvider;

                /**
                    * The server provides document formatting on typing.
                    */
                std::optional<document_on_type_formatting_options> documentOnTypeFormattingProvider;

                /**
                    * The server provides rename support. RenameOptions may only be
                    * specified if the client states that it supports
                    * `prepareSupport` in its initial `initialize` request.
                    *
                    * Implementors note: Technically, this should support `boolean | RenameOptions` for backwards compatibility ... but we ignore that simply because
                    *                    it already is hard enough to provide this shitfest of a protocol. No need to make it even harder to implement
                    *                    a server.
                    */
                std::optional<rename_options> renameProvider;

                /**
                    * The server provides folding provider support.
                    *
                    * @since 3.10.0
                    *
                    * Implementors note: Technically, this should support `boolean | FoldingRangeOptions | FoldingRangeRegistrationOptions` for backwards compatibility ... but we ignore that simply because
                    *                    it already is hard enough to provide this shitfest of a protocol. No need to make it even harder to implement
                    *                    a server.
                    */
                std::optional<folding_range_registration_options> foldingRangeProvider;

                /**
                    * The server provides execute command support.
                    */
                std::optional<execute_command_options> executeCommandProvider;

                /**
                * The server provides selection range support.
                *
                * @since 3.15.0
                *
                * Implementors note: Technically, this should support `boolean | SelectionRangeOptions | SelectionRangeRegistrationOptions` for backwards compatibility ... but we ignore that simply because
                *                    it already is hard enough to provide this shitfest of a protocol. No need to make it even harder to implement
                *                    a server.
                */
                std::optional<selection_range_registration_options> selectionRangeProvider;

                /**
                * The server provides workspace symbol support.
                */
                std::optional<bool> workspaceSymbolProvider;

                /**
                * Workspace specific server capabilities
                */
                std::optional<Workspace> workspace;

                /**
                * Experimental server capabilities.
                */
                std::optional<nlohmann::json> experimental;

                static server_capabilities from_json(const nlohmann::json &node) {
                    server_capabilities res;
                    data::from_json(node, "textDocumentSync", res.textDocumentSync);
                    data::from_json(node, "completionProvider", res.completionProvider);
                    data::from_json(node, "hoverProvider", res.hoverProvider);
                    data::from_json(node, "signatureHelpProvider", res.signatureHelpProvider);
                    data::from_json(node, "declarationProvider", res.declarationProvider);
                    data::from_json(node, "definitionProvider", res.definitionProvider);
                    data::from_json(node, "typeDefinitionProvider", res.typeDefinitionProvider);
                    data::from_json(node, "implementationProvider", res.implementationProvider);
                    data::from_json(node, "referencesProvider", res.referencesProvider);
                    data::from_json(node, "documentHighlightProvider", res.documentHighlightProvider);
                    data::from_json(node, "documentSymbolProvider", res.documentSymbolProvider);
                    data::from_json(node, "codeActionProvider", res.codeActionProvider);
                    data::from_json(node, "codeLensProvider", res.codeLensProvider);
                    data::from_json(node, "documentLinkProvider", res.documentLinkProvider);
                    data::from_json(node, "colorProvider", res.colorProvider);
                    data::from_json(node, "documentFormattingProvider", res.documentFormattingProvider);
                    data::from_json(node, "documentRangeFormattingProvider", res.documentRangeFormattingProvider);
                    data::from_json(node, "documentOnTypeFormattingProvider", res.documentOnTypeFormattingProvider);
                    data::from_json(node, "renameProvider", res.renameProvider);
                    data::from_json(node, "foldingRangeProvider", res.foldingRangeProvider);
                    data::from_json(node, "executeCommandProvider", res.executeCommandProvider);
                    data::from_json(node, "selectionRangeProvider", res.selectionRangeProvider);
                    data::from_json(node, "workspaceSymbolProvider", res.workspaceSymbolProvider);
                    data::from_json(node, "workspace", res.workspace);
                    res.experimental = node.contains("experimental") ? node["experimental"] : nlohmann::json(nullptr);
                    return res;
                }

                nlohmann::json to_json() const {
                    nlohmann::json json;
                    data::set_json(json, "textDocumentSync", textDocumentSync);
                    data::set_json(json, "completionProvider", completionProvider);
                    data::set_json(json, "hoverProvider", hoverProvider);
                    data::set_json(json, "signatureHelpProvider", signatureHelpProvider);
                    data::set_json(json, "declarationProvider", declarationProvider);
                    data::set_json(json, "definitionProvider", definitionProvider);
                    data::set_json(json, "typeDefinitionProvider", typeDefinitionProvider);
                    data::set_json(json, "implementationProvider", implementationProvider);
                    data::set_json(json, "referencesProvider", referencesProvider);
                    data::set_json(json, "documentHighlightProvider", documentHighlightProvider);
                    data::set_json(json, "documentSymbolProvider", documentSymbolProvider);
                    data::set_json(json, "codeActionProvider", codeActionProvider);
                    data::set_json(json, "codeLensProvider", codeLensProvider);
                    data::set_json(json, "documentLinkProvider", documentLinkProvider);
                    data::set_json(json, "colorProvider", colorProvider);
                    data::set_json(json, "documentFormattingProvider", documentFormattingProvider);
                    data::set_json(json, "documentRangeFormattingProvider", documentRangeFormattingProvider);
                    data::set_json(json, "documentOnTypeFormattingProvider", documentOnTypeFormattingProvider);
                    data::set_json(json, "renameProvider", renameProvider);
                    data::set_json(json, "foldingRangeProvider", foldingRangeProvider);
                    data::set_json(json, "executeCommandProvider", executeCommandProvider);
                    data::set_json(json, "selectionRangeProvider", selectionRangeProvider);
                    data::set_json(json, "workspaceSymbolProvider", workspaceSymbolProvider);
                    data::set_json(json, "workspace", workspace);
                    if (experimental.has_value()) {
                        json["experimental"] = *experimental;
                    }
                    return json;
                }
            };

            struct server_info {
                std::string name;
                std::optional<std::string> version;

                static server_info from_json(const nlohmann::json &node) {
                    server_info res;
                    data::from_json(node, "name", res.name);
                    data::from_json(node, "version", res.version);
                    return res;
                }

                nlohmann::json to_json() const {
                    nlohmann::json json;
                    data::set_json(json, "name", name);
                    data::set_json(json, "version", version);
                    return json;
                }
            };

            /**
            * The capabilities the language server provides.
            */
            server_capabilities capabilities;

            /**
            * Information about the server.
            *
            * @since 3.15.0
            */
            std::optional<server_info> serverInfo;

            static initialize_result from_json(const nlohmann::json &node) {
                initialize_result res;
                data::from_json(node, "capabilities", res.capabilities);
                data::from_json(node, "serverInfo", res.serverInfo);
                return res;
            }

            [[nodiscard]] nlohmann::json to_json() const {
                nlohmann::json json;
                data::set_json(json, "capabilities", capabilities);
                data::set_json(json, "serverInfo", serverInfo);
                return json;
            }
        };

        struct initialize_params {
            struct client_info {
                std::string name;
                std::string version;

                static client_info from_json(const nlohmann::json &node) {
                    return {
                            node["name"], node["version"]};
                }

                nlohmann::json to_json() const {
                    return {
                            {"name",    name},
                            {"version", version}};
                }
            };

            struct workspace_edit_client_capabilities {
                /*
                    The client supports versioned document changes in `WorkspaceEdit`s
                */
                std::optional<bool> documentChanges;
                /*
                    The resource operations the client supports. Clients should at least
                    support 'create', 'rename' and 'delete' files and folders.

                    @since 3.13.0
                */
                resource_operations resourceOperations;
                /*
                    The failure handling strategy of a client if applying the workspace edit
                    fails.

                    @since 3.13.0
                */
                failure_handling failureHandling;

                static workspace_edit_client_capabilities from_json(const nlohmann::json &node) {
                    workspace_edit_client_capabilities cap;
                    data::from_json(node, "documentChanges", cap.documentChanges);
                    data::from_json(node, "resourceOperations", cap.resourceOperations);
                    data::from_json(node, "failureHandling", cap.failureHandling);
                    return cap;
                }

                nlohmann::json to_json() const {
                    nlohmann::json json;
                    data::set_json(json, "documentChanges", documentChanges);
                    data::set_json(json, "resourceOperations", resourceOperations);
                    data::set_json(json, "failureHandling", failureHandling);
                    return json;
                }
            };

            struct did_change_configuration_client_capabilities {
                /*
                    Did change configuration notification supports dynamic registration.
                */
                std::optional<bool> dynamicRegistration;

                static did_change_configuration_client_capabilities from_json(const nlohmann::json &node) {
                    did_change_configuration_client_capabilities res;
                    data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                    return res;
                }

                nlohmann::json to_json() const {
                    nlohmann::json json;
                    data::set_json(json, "dynamicRegistration", dynamicRegistration);
                    return json;
                }
            };

            struct did_change_watched_files_client_capabilities {
                /*
                    Did change watched files notification supports dynamic registration. Please note
                    that the current protocol doesn't support static configuration for file changes
                    from the server side.
                */
                std::optional<bool> dynamicRegistration;

                static did_change_watched_files_client_capabilities from_json(const nlohmann::json &node) {
                    did_change_watched_files_client_capabilities res;
                    data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                    return res;
                }

                nlohmann::json to_json() const {
                    nlohmann::json json;
                    data::set_json(json, "dynamicRegistration", dynamicRegistration);
                    return json;
                }
            };

            struct workspace_symbol_client_capabilities {
                struct SymbolKind {
                    std::optional<std::vector<symbol_kind>> valueSet;

                    static SymbolKind from_json(const nlohmann::json &node) {
                        SymbolKind res;
                        data::from_json(node, "valueSet", res.valueSet);
                        return res;
                    }

                    nlohmann::json to_json() const {
                        nlohmann::json json;
                        data::set_json(json, "valueSet", valueSet);
                        return json;
                    }
                };

                /*
                    Symbol request supports dynamic registration.
                */
                std::optional<bool> dynamicRegistration;
                /*
                    Specific capabilities for the `SymbolKind` in the `workspace/symbol` request.
                */
                std::optional<SymbolKind> symbolKind;

                static workspace_symbol_client_capabilities from_json(const nlohmann::json &node) {
                    workspace_symbol_client_capabilities res;
                    data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                    data::from_json(node, "symbolKind", res.symbolKind);
                    return res;
                }

                nlohmann::json to_json() const {
                    nlohmann::json json;
                    data::set_json(json, "dynamicRegistration", dynamicRegistration);
                    data::set_json(json, "symbolKind", symbolKind);
                    return json;
                }
            };

            struct execute_command_client_capabilities {
                /*
                    Execute command supports dynamic registration.
                */
                std::optional<bool> dynamicRegistration;

                static execute_command_client_capabilities from_json(const nlohmann::json &node) {
                    execute_command_client_capabilities res;
                    data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                    return res;
                }

                nlohmann::json to_json() const {
                    nlohmann::json json;
                    data::set_json(json, "dynamicRegistration", dynamicRegistration);
                    return json;
                }
            };

            struct text_document_sync_client_capabilities {
                /*
                    Whether text document synchronization supports dynamic registration.
                */
                std::optional<bool> dynamicRegistration;

                /*
                    The client supports sending will save notifications.
                */
                std::optional<bool> willSave;

                /*
                    The client supports sending a will save request and
                    waits for a response providing text edits which will
                    be applied to the document before it is saved.
                */
                std::optional<bool> willSaveWaitUntil;

                /*
                    The client supports did save notifications.
                */
                std::optional<bool> didSave;

                static text_document_sync_client_capabilities from_json(const nlohmann::json &node) {
                    text_document_sync_client_capabilities res;
                    data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                    data::from_json(node, "willSave", res.willSave);
                    data::from_json(node, "willSaveWaitUntil", res.willSaveWaitUntil);
                    data::from_json(node, "didSave", res.didSave);
                    return res;
                }

                nlohmann::json to_json() const {
                    nlohmann::json json;
                    data::set_json(json, "dynamicRegistration", dynamicRegistration);
                    data::set_json(json, "willSave", willSave);
                    data::set_json(json, "willSaveWaitUntil", willSaveWaitUntil);
                    data::set_json(json, "didSave", didSave);
                    return json;
                }
            };

            struct completion_client_capabilities {
                struct CompletionItem {
                    struct TagSupport {
                        /**
                            * The tags supported by the client.
                            */
                        std::optional<std::vector<completion_item_tag>> valueSet;

                        static TagSupport from_json(const nlohmann::json &node) {
                            TagSupport res;
                            data::from_json(node, "valueSet", res.valueSet);
                            return res;
                        }

                        nlohmann::json to_json() const {
                            nlohmann::json json;
                            data::set_json(json, "valueSet", valueSet);
                            return json;
                        }
                    };

                    /**
                        * Client supports snippets as insert text.
                        *
                        * A snippet can define tab stops and placeholders with `$1`, `$2`
                        * and `${3:foo}`. `$0` defines the final tab stop, it defaults to
                        * the end of the snippet. Placeholders with equal identifiers are linked,
                        * that is typing in one will update others too.
                        */
                    std::optional<bool> snippetSupport;

                    /**
                        * Client supports commit characters on a completion item.
                        */
                    std::optional<bool> commitCharactersSupport;

                    /**
                    * Client supports the follow content formats for the documentation
                    * property. The order describes the preferred format of the client.
                    */
                    std::optional<std::vector<markup_kind>> documentationFormat;

                    /**
                        * Client supports the deprecated property on a completion item.
                        */
                    std::optional<bool> deprecatedSupport;

                    /**
                        * Client supports the preselect property on a completion item.
                        */
                    std::optional<bool> preselectSupport;

                    /**
                        * Client supports the tag property on a completion item. Clients supporting
                        * tags have to handle unknown tags gracefully. Clients especially need to
                        * preserve unknown tags when sending a completion item back to the server in
                        * a resolve call.
                        *
                        * @since 3.15.0
                        */
                    std::optional<TagSupport> tagSupport;

                    static CompletionItem from_json(const nlohmann::json &node) {
                        CompletionItem res;
                        data::from_json(node, "snippetSupport", res.snippetSupport);
                        data::from_json(node, "commitCharactersSupport", res.commitCharactersSupport);
                        data::from_json(node, "documentationFormat", res.documentationFormat);
                        data::from_json(node, "deprecatedSupport", res.deprecatedSupport);
                        data::from_json(node, "preselectSupport", res.preselectSupport);
                        data::from_json(node, "tagSupport", res.tagSupport);
                        return res;
                    }

                    nlohmann::json to_json() const {
                        nlohmann::json json;
                        data::set_json(json, "snippetSupport", snippetSupport);
                        data::set_json(json, "commitCharactersSupport", commitCharactersSupport);
                        data::set_json(json, "documentationFormat", documentationFormat);
                        data::set_json(json, "deprecatedSupport", deprecatedSupport);
                        data::set_json(json, "preselectSupport", preselectSupport);
                        data::set_json(json, "tagSupport", tagSupport);
                        return json;
                    }
                };

                struct CompletionItemKind {
                    /*
                        The completion item kind values the client supports. When this
                        property exists the client also guarantees that it will
                        handle values outside its set gracefully and falls back
                        to a default value when unknown.

                        If this property is not present the client only supports
                        the completion items kinds from `Text` to `Reference` as defined in
                        the initial version of the protocol.
                    */
                    std::optional<std::vector<completion_item_kind>> valueSet;

                    static CompletionItemKind from_json(const nlohmann::json &node) {
                        CompletionItemKind res;
                        data::from_json(node, "valueSet", res.valueSet);
                        return res;
                    }

                    nlohmann::json to_json() const {
                        nlohmann::json json;
                        data::set_json(json, "valueSet", valueSet);
                        return json;
                    }
                };

                /*
                    The client supports the following `CompletionItem` specific
                    capabilities.
                */
                std::optional<bool> dynamicRegistration;
                /*
                    The client supports the following `CompletionItem` specific
                    capabilities.
                */
                std::optional<CompletionItem> completionItem;
                /*
                    The client supports the following `CompletionItem` specific
                    capabilities.
                */
                std::optional<CompletionItemKind> completionItemKind;
                /*
                    The client supports to send additional context information for a
                    `textDocument/completion` request.
                */
                std::optional<bool> contextSupport;

                static completion_client_capabilities from_json(const nlohmann::json &node) {
                    completion_client_capabilities res;
                    data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                    data::from_json(node, "completionItem", res.completionItem);
                    data::from_json(node, "completionItemKind", res.completionItemKind);
                    data::from_json(node, "contextSupport", res.contextSupport);
                    return res;
                }

                nlohmann::json to_json() const {
                    nlohmann::json json;
                    data::set_json(json, "dynamicRegistration", dynamicRegistration);
                    data::set_json(json, "completionItem", completionItem);
                    data::set_json(json, "completionItemKind", completionItemKind);
                    data::set_json(json, "contextSupport", contextSupport);
                    return json;
                }
            };

            struct hover_client_capabilities {
                /**
                    * Whether hover supports dynamic registration.
                    */
                std::optional<bool> dynamicRegistration;

                /**
                    * Client supports the follow content formats for the content
                    * property. The order describes the preferred format of the client.
                    */
                std::optional<std::vector<markup_kind>> contentFormat;

                static hover_client_capabilities from_json(const nlohmann::json &node) {
                    hover_client_capabilities res;
                    data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                    data::from_json(node, "contentFormat", res.contentFormat);
                    return res;
                }

                nlohmann::json to_json() const {
                    nlohmann::json json;
                    data::set_json(json, "dynamicRegistration", dynamicRegistration);
                    data::set_json(json, "contentFormat", contentFormat);
                    return json;
                }
            };

            struct signature_help_client_capabilities {
                struct SignatureInformation {
                    struct ParameterInformation {
                        /**
                            * The client supports processing label offsets instead of a
                            * simple label string.
                            *
                            * @since 3.14.0
                            */
                        std::optional<bool> labelOffsetSupport;

                        static ParameterInformation from_json(const nlohmann::json &node) {
                            ParameterInformation res;
                            data::from_json(node, "labelOffsetSupport", res.labelOffsetSupport);
                            return res;
                        }

                        nlohmann::json to_json() const {
                            nlohmann::json json;
                            data::set_json(json, "labelOffsetSupport", labelOffsetSupport);
                            return json;
                        }
                    };

                    /**
                        * Client supports the follow content formats for the documentation
                        * property. The order describes the preferred format of the client.
                        */
                    std::optional<std::vector<markup_kind>> documentationFormat;

                    /**
                        * Client capabilities specific to parameter information.
                        */
                    std::optional<ParameterInformation> parameterInformation;

                    static SignatureInformation from_json(const nlohmann::json &node) {
                        SignatureInformation res;
                        data::from_json(node, "documentationFormat", res.documentationFormat);
                        data::from_json(node, "parameterInformation", res.parameterInformation);
                        return res;
                    }

                    nlohmann::json to_json() const {
                        nlohmann::json json;
                        data::set_json(json, "documentationFormat", documentationFormat);
                        data::set_json(json, "parameterInformation", parameterInformation);
                        return json;
                    }
                };

                /**
                    * Whether signature help supports dynamic registration.
                    */
                std::optional<bool> dynamicRegistration;

                /**
                    * The client supports the following `SignatureInformation`
                    * specific properties.
                    */
                std::optional<SignatureInformation> signatureInformation;

                /**
                    * The client supports to send additional context information for a
                    * `textDocument/signatureHelp` request. A client that opts into
                    * contextSupport will also support the `retriggerCharacters` on
                    * `SignatureHelpOptions`.
                    *
                    * @since 3.15.0
                    */
                std::optional<bool> contextSupport;

                static signature_help_client_capabilities from_json(const nlohmann::json &node) {
                    signature_help_client_capabilities res;
                    data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                    data::from_json(node, "signatureInformation", res.signatureInformation);
                    data::from_json(node, "contextSupport", res.contextSupport);
                    return res;
                }

                nlohmann::json to_json() const {
                    nlohmann::json json;
                    data::set_json(json, "dynamicRegistration", dynamicRegistration);
                    data::set_json(json, "signatureInformation", signatureInformation);
                    data::set_json(json, "contextSupport", contextSupport);
                    return json;
                }
            };

            struct declaration_client_capabilities {
                /**
                    * Whether declaration supports dynamic registration. If this is set to `true`
                    * the client supports the new `DeclarationRegistrationOptions` return value
                    * for the corresponding server capability as well.
                    */
                std::optional<bool> dynamicRegistration;

                /**
                    * The client supports additional metadata in the form of declaration links.
                    */
                std::optional<bool> linkSupport;

                static declaration_client_capabilities from_json(const nlohmann::json &node) {
                    declaration_client_capabilities res;
                    data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                    data::from_json(node, "linkSupport", res.linkSupport);
                    return res;
                }

                nlohmann::json to_json() const {
                    nlohmann::json json;
                    data::set_json(json, "dynamicRegistration", dynamicRegistration);
                    data::set_json(json, "linkSupport", linkSupport);
                    return json;
                }
            };

            struct definition_client_capabilities {
                /**
                    * Whether definition supports dynamic registration.
                    */
                std::optional<bool> dynamicRegistration;

                /**
                    * The client supports additional metadata in the form of definition links.
                    *
                    * @since 3.14.0
                    */
                std::optional<bool> linkSupport;

                static definition_client_capabilities from_json(const nlohmann::json &node) {
                    definition_client_capabilities res;
                    data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                    data::from_json(node, "linkSupport", res.linkSupport);
                    return res;
                }

                nlohmann::json to_json() const {
                    nlohmann::json json;
                    data::set_json(json, "dynamicRegistration", dynamicRegistration);
                    data::set_json(json, "linkSupport", linkSupport);
                    return json;
                }
            };

            struct type_definition_client_capabilities {
                /**
                    * Whether implementation supports dynamic registration. If this is set to `true`
                    * the client supports the new `TypeDefinitionRegistrationOptions` return value
                    * for the corresponding server capability as well.
                    */
                std::optional<bool> dynamicRegistration;

                /**
                    * The client supports additional metadata in the form of definition links.
                    *
                    * @since 3.14.0
                    */
                std::optional<bool> linkSupport;

                static type_definition_client_capabilities from_json(const nlohmann::json &node) {
                    type_definition_client_capabilities res;
                    data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                    data::from_json(node, "linkSupport", res.linkSupport);
                    return res;
                }

                nlohmann::json to_json() const {
                    nlohmann::json json;
                    data::set_json(json, "dynamicRegistration", dynamicRegistration);
                    data::set_json(json, "linkSupport", linkSupport);
                    return json;
                }
            };

            struct implementation_client_capabilities {
                /**
                    * Whether implementation supports dynamic registration. If this is set to `true`
                    * the client supports the new `ImplementationRegistrationOptions` return value
                    * for the corresponding server capability as well.
                    */
                std::optional<bool> dynamicRegistration;

                /**
                    * The client supports additional metadata in the form of definition links.
                    *
                    * @since 3.14.0
                    */
                std::optional<bool> linkSupport;

                static implementation_client_capabilities from_json(const nlohmann::json &node) {
                    implementation_client_capabilities res;
                    data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                    data::from_json(node, "linkSupport", res.linkSupport);
                    return res;
                }

                nlohmann::json to_json() const {
                    nlohmann::json json;
                    data::set_json(json, "dynamicRegistration", dynamicRegistration);
                    data::set_json(json, "linkSupport", linkSupport);
                    return json;
                }
            };

            struct reference_client_capabilities {
                /**
                    * Whether references supports dynamic registration.
                    */
                std::optional<bool> dynamicRegistration;

                static reference_client_capabilities from_json(const nlohmann::json &node) {
                    reference_client_capabilities res;
                    data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                    return res;
                }

                nlohmann::json to_json() const {
                    nlohmann::json json;
                    data::set_json(json, "dynamicRegistration", dynamicRegistration);
                    return json;
                }
            };

            struct document_highlight_client_capabilities {
                /**
                    * Whether document highlight supports dynamic registration.
                    */
                std::optional<bool> dynamicRegistration;

                static document_highlight_client_capabilities from_json(const nlohmann::json &node) {
                    document_highlight_client_capabilities res;
                    data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                    return res;
                }

                nlohmann::json to_json() const {
                    nlohmann::json json;
                    data::set_json(json, "dynamicRegistration", dynamicRegistration);
                    return json;
                }
            };

            struct document_symbol_client_capabilities {
                struct SymbolKind {
                    /**
                        * The symbol kind values the client supports. When this
                        * property exists the client also guarantees that it will
                        * handle values outside its set gracefully and falls back
                        * to a default value when unknown.
                        *
                        * If this property is not present the client only supports
                        * the symbol kinds from `File` to `Array` as defined in
                        * the initial version of the protocol.
                        */
                    std::optional<std::vector<symbol_kind>> valueSet;

                    static SymbolKind from_json(const nlohmann::json &node) {
                        SymbolKind res;
                        data::from_json(node, "valueSet", res.valueSet);
                        return res;
                    }

                    nlohmann::json to_json() const {
                        nlohmann::json json;
                        data::set_json(json, "valueSet", valueSet);
                        return json;
                    }
                };

                /**
                    * Whether document symbol supports dynamic registration.
                    */
                std::optional<bool> dynamicRegistration;

                /**
                    * Specific capabilities for the `SymbolKind` in the `textDocument/documentSymbol` request.
                    */
                std::optional<SymbolKind> symbolKind;

                /**
                    * The client supports hierarchical document symbols.
                    */
                std::optional<bool> hierarchicalDocumentSymbolSupport;

                static document_symbol_client_capabilities from_json(const nlohmann::json &node) {
                    document_symbol_client_capabilities res;
                    data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                    data::from_json(node, "symbolKind", res.symbolKind);
                    data::from_json(node, "hierarchicalDocumentSymbolSupport", res.hierarchicalDocumentSymbolSupport);
                    return res;
                }

                nlohmann::json to_json() const {
                    nlohmann::json json;
                    data::set_json(json, "dynamicRegistration", dynamicRegistration);
                    data::set_json(json, "symbolKind", symbolKind);
                    data::set_json(json, "hierarchicalDocumentSymbolSupport", hierarchicalDocumentSymbolSupport);
                    return json;
                }
            };

            struct code_action_client_capabilities {
                struct CodeActionLiteralSupport {
                    /**
                        * The code action kind is supported with the following value
                        * set.
                        */
                    struct {
                        /**
                            * The code action kind values the client supports. When this
                            * property exists the client also guarantees that it will
                            * handle values outside its set gracefully and falls back
                            * to a default value when unknown.
                            */
                        std::vector<code_action_kind> valueSet;
                    } codeActionKind;

                    static CodeActionLiteralSupport from_json(const nlohmann::json &node) {
                        CodeActionLiteralSupport res;
                        data::from_json(node["codeActionKind"], "valueSet", res.codeActionKind.valueSet);
                        return res;
                    }

                    nlohmann::json to_json() const {
                        nlohmann::json json;
                        json["codeActionKind"] = {"valueSet", data::to_json(codeActionKind.valueSet)};
                        return json;
                    }
                };

                /**
                    * Whether code action supports dynamic registration.
                    */
                std::optional<bool> dynamicRegistration;

                /**
                    * The client supports code action literals as a valid
                    * response of the `textDocument/codeAction` request.
                    *
                    * @since 3.8.0
                    */
                std::optional<CodeActionLiteralSupport> codeActionLiteralSupport;

                /**
                    * Whether code action supports the `isPreferred` property.
                    * @since 3.15.0
                    */
                std::optional<bool> isPreferredSupport;

                static code_action_client_capabilities from_json(const nlohmann::json &node) {
                    code_action_client_capabilities res;
                    data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                    data::from_json(node, "codeActionLiteralSupport", res.codeActionLiteralSupport);
                    data::from_json(node, "isPreferredSupport", res.isPreferredSupport);
                    return res;
                }

                nlohmann::json to_json() const {
                    nlohmann::json json;
                    data::set_json(json, "dynamicRegistration", dynamicRegistration);
                    data::set_json(json, "codeActionLiteralSupport", codeActionLiteralSupport);
                    data::set_json(json, "isPreferredSupport", isPreferredSupport);
                    return json;
                }
            };

            struct code_lens_client_capabilities {
                /**
                    * Whether code lens supports dynamic registration.
                    */
                std::optional<bool> dynamicRegistration;

                static code_lens_client_capabilities from_json(const nlohmann::json &node) {
                    code_lens_client_capabilities res;
                    data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                    return res;
                }

                nlohmann::json to_json() const {
                    nlohmann::json json;
                    data::set_json(json, "dynamicRegistration", dynamicRegistration);
                    return json;
                }
            };

            struct document_link_client_capabilities {
                /**
                    * Whether document link supports dynamic registration.
                    */
                std::optional<bool> dynamicRegistration;

                /**
                    * Whether the client supports the `tooltip` property on `DocumentLink`.
                    *
                    * @since 3.15.0
                    */
                std::optional<bool> tooltipSupport;

                static document_link_client_capabilities from_json(const nlohmann::json &node) {
                    document_link_client_capabilities res;
                    data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                    data::from_json(node, "tooltipSupport", res.tooltipSupport);
                    return res;
                }

                nlohmann::json to_json() const {
                    nlohmann::json json;
                    data::set_json(json, "dynamicRegistration", dynamicRegistration);
                    data::set_json(json, "tooltipSupport", tooltipSupport);
                    return json;
                }
            };

            struct document_color_client_capabilities {
                /**
                    * Whether document color supports dynamic registration.
                    */
                std::optional<bool> dynamicRegistration;

                static document_color_client_capabilities from_json(const nlohmann::json &node) {
                    document_color_client_capabilities res;
                    data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                    return res;
                }

                nlohmann::json to_json() const {
                    nlohmann::json json;
                    data::set_json(json, "dynamicRegistration", dynamicRegistration);
                    return json;
                }
            };

            struct document_formatting_client_capabilities {
                /**
                    * Whether formatting supports dynamic registration.
                    */
                std::optional<bool> dynamicRegistration;

                static document_formatting_client_capabilities from_json(const nlohmann::json &node) {
                    document_formatting_client_capabilities res;
                    data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                    return res;
                }

                nlohmann::json to_json() const {
                    nlohmann::json json;
                    data::set_json(json, "dynamicRegistration", dynamicRegistration);
                    return json;
                }
            };

            struct document_range_formatting_client_capabilities {
                /**
                    * Whether formatting supports dynamic registration.
                    */
                std::optional<bool> dynamicRegistration;

                static document_range_formatting_client_capabilities from_json(const nlohmann::json &node) {
                    document_range_formatting_client_capabilities res;
                    data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                    return res;
                }

                nlohmann::json to_json() const {
                    nlohmann::json json;
                    data::set_json(json, "dynamicRegistration", dynamicRegistration);
                    return json;
                }
            };

            struct document_on_type_formatting_client_capabilities {
                /**
                    * Whether on type formatting supports dynamic registration.
                    */
                std::optional<bool> dynamicRegistration;

                static document_on_type_formatting_client_capabilities from_json(const nlohmann::json &node) {
                    document_on_type_formatting_client_capabilities res;
                    data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                    return res;
                }

                nlohmann::json to_json() const {
                    nlohmann::json json;
                    data::set_json(json, "dynamicRegistration", dynamicRegistration);
                    return json;
                }
            };

            struct rename_client_capabilities {
                /**
                    * Whether rename supports dynamic registration.
                    */
                std::optional<bool> dynamicRegistration;

                /**
                    * Client supports testing for validity of rename operations
                    * before execution.
                    *
                    * @since version 3.12.0
                    */
                std::optional<bool> prepareSupport;

                static rename_client_capabilities from_json(const nlohmann::json &node) {
                    rename_client_capabilities res;
                    data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                    data::from_json(node, "prepareSupport", res.prepareSupport);
                    return res;
                }

                nlohmann::json to_json() const {
                    nlohmann::json json;
                    data::set_json(json, "dynamicRegistration", dynamicRegistration);
                    data::set_json(json, "prepareSupport", prepareSupport);
                    return json;
                }
            };

            struct publish_diagnostics_client_capabilities {
                struct TagSupport {
                    /**
                        * The tags supported by the client.
                        */
                    std::optional<std::vector<diagnostic_tag>> valueSet;

                    static TagSupport from_json(const nlohmann::json &node) {
                        TagSupport res;
                        data::from_json(node, "valueSet", res.valueSet);
                        return res;
                    }

                    nlohmann::json to_json() const {
                        nlohmann::json json;
                        data::set_json(json, "valueSet", valueSet);
                        return json;
                    }
                };

                /**
                    * Whether the clients accepts diagnostics with related information.
                    */
                std::optional<bool> relatedInformation;

                /**
                    * Client supports the tag property to provide meta data about a diagnostic.
                    * Clients supporting tags have to handle unknown tags gracefully.
                    *
                    * @since 3.15.0
                    */
                std::optional<TagSupport> tagSupport;

                /**
                    * Whether the client interprets the version property of the
                    * `textDocument/publishDiagnostics` notification's parameter.
                    *
                    * @since 3.15.0
                    */
                std::optional<bool> versionSupport;

                static publish_diagnostics_client_capabilities from_json(const nlohmann::json &node) {
                    publish_diagnostics_client_capabilities res;
                    data::from_json(node, "relatedInformation", res.relatedInformation);
                    data::from_json(node, "tagSupport", res.tagSupport);
                    data::from_json(node, "versionSupport", res.versionSupport);
                    return res;
                }

                nlohmann::json to_json() const {
                    nlohmann::json json;
                    data::set_json(json, "relatedInformation", relatedInformation);
                    data::set_json(json, "tagSupport", tagSupport);
                    data::set_json(json, "versionSupport", versionSupport);
                    return json;
                }
            };

            struct folding_range_client_capabilities {
                /**
                    * Whether implementation supports dynamic registration for folding range providers. If this is set to `true`
                    * the client supports the new `FoldingRangeRegistrationOptions` return value for the corresponding server
                    * capability as well.
                    */
                std::optional<bool> dynamicRegistration;
                /**
                    * The maximum number of folding ranges that the client prefers to receive per document. The value serves as a
                    * hint, servers are free to follow the limit.
                    */
                std::optional<size_t> rangeLimit;
                /**
                    * If set, the client signals that it only supports folding complete lines. If set, client will
                    * ignore specified `startCharacter` and `endCharacter` properties in a FoldingRange.
                    */
                std::optional<bool> lineFoldingOnly;

                static folding_range_client_capabilities from_json(const nlohmann::json &node) {
                    folding_range_client_capabilities res;
                    data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                    data::from_json(node, "rangeLimit", res.rangeLimit);
                    data::from_json(node, "lineFoldingOnly", res.lineFoldingOnly);
                    return res;
                }

                nlohmann::json to_json() const {
                    nlohmann::json json;
                    data::set_json(json, "dynamicRegistration", dynamicRegistration);
                    data::set_json(json, "rangeLimit", rangeLimit);
                    data::set_json(json, "lineFoldingOnly", lineFoldingOnly);
                    return json;
                }
            };

            struct selection_range_client_capabilities {
                /**
                    * Whether implementation supports dynamic registration for selection range providers. If this is set to `true`
                    * the client supports the new `SelectionRangeRegistrationOptions` return value for the corresponding server
                    * capability as well.
                    */
                std::optional<bool> dynamicRegistration;

                static selection_range_client_capabilities from_json(const nlohmann::json &node) {
                    selection_range_client_capabilities res;
                    data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                    return res;
                }

                nlohmann::json to_json() const {
                    nlohmann::json json;
                    data::set_json(json, "dynamicRegistration", dynamicRegistration);
                    return json;
                }
            };

            /*
                Text document specific client capabilities.
            */
            struct text_document_client_capabilities {
                std::optional<text_document_sync_client_capabilities> synchronization;

                /**
                    Capabilities specific to the `textDocument/completion` request.
                */
                std::optional<completion_client_capabilities> completion;

                /**
                    Capabilities specific to the `textDocument/hover` request.
                */
                std::optional<hover_client_capabilities> hover;

                /**
                    Capabilities specific to the `textDocument/signatureHelp` request.
                */
                std::optional<signature_help_client_capabilities> signatureHelp;

                /**
                    Capabilities specific to the `textDocument/declaration` request.

                    @since 3.14.0
                */
                std::optional<declaration_client_capabilities> declaration;

                /**
                    Capabilities specific to the `textDocument/definition` request.
                */
                std::optional<definition_client_capabilities> definition;

                /**
                    Capabilities specific to the `textDocument/typeDefinition` request.

                    @since 3.6.0
                */
                std::optional<type_definition_client_capabilities> typeDefinition;

                /**
                    Capabilities specific to the `textDocument/implementation` request.

                    @since 3.6.0
                */
                std::optional<implementation_client_capabilities> implementation;

                /**
                    Capabilities specific to the `textDocument/references` request.
                */
                std::optional<reference_client_capabilities> references;

                /**
                    Capabilities specific to the `textDocument/documentHighlight` request.
                */
                std::optional<document_highlight_client_capabilities> documentHighlight;

                /**
                    Capabilities specific to the `textDocument/documentSymbol` request.
                */
                std::optional<document_symbol_client_capabilities> documentSymbol;

                /**
                    Capabilities specific to the `textDocument/codeAction` request.
                */
                std::optional<code_action_client_capabilities> codeAction;

                /**
                    Capabilities specific to the `textDocument/codeLens` request.
                */
                std::optional<code_lens_client_capabilities> codeLens;

                /**
                    Capabilities specific to the `textDocument/documentLink` request.
                */
                std::optional<document_link_client_capabilities> documentLink;

                /**
                    Capabilities specific to the `textDocument/documentColor` and the
                    `textDocument/colorPresentation` request.

                    @since 3.6.0
                */
                std::optional<document_color_client_capabilities> colorProvider;

                /**
                    Capabilities specific to the `textDocument/formatting` request.
                */
                std::optional<document_formatting_client_capabilities> formatting;

                /**
                    Capabilities specific to the `textDocument/rangeFormatting` request.
                */
                std::optional<document_range_formatting_client_capabilities> rangeFormatting;

                /** request.
                    Capabilities specific to the `textDocument/onTypeFormatting` request.
                */
                std::optional<document_on_type_formatting_client_capabilities> onTypeFormatting;

                /**
                    Capabilities specific to the `textDocument/rename` request.
                */
                std::optional<rename_client_capabilities> rename;

                /**
                    Capabilities specific to the `textDocument/publishDiagnostics` notification.
                */
                std::optional<publish_diagnostics_client_capabilities> publishDiagnostics;

                /**
                    Capabilities specific to the `textDocument/foldingRange` request.

                    @since 3.10.0
                */
                std::optional<folding_range_client_capabilities> foldingRange;

                /**
                    Capabilities specific to the `textDocument/selectionRange` request.

                    @since 3.15.0
                */
                std::optional<selection_range_client_capabilities> selectionRange;

                static text_document_client_capabilities from_json(const nlohmann::json &node) {
                    text_document_client_capabilities res;
                    data::from_json(node, "synchronization", res.synchronization);
                    data::from_json(node, "completion", res.completion);
                    data::from_json(node, "hover", res.hover);
                    data::from_json(node, "signatureHelp", res.signatureHelp);
                    data::from_json(node, "declaration", res.declaration);
                    data::from_json(node, "definition", res.definition);
                    data::from_json(node, "typeDefinition", res.typeDefinition);
                    data::from_json(node, "implementation", res.implementation);
                    data::from_json(node, "references", res.references);
                    data::from_json(node, "documentHighlight", res.documentHighlight);
                    data::from_json(node, "documentSymbol", res.documentSymbol);
                    data::from_json(node, "codeAction", res.codeAction);
                    data::from_json(node, "codeLens", res.codeLens);
                    data::from_json(node, "documentLink", res.documentLink);
                    data::from_json(node, "colorProvider", res.colorProvider);
                    data::from_json(node, "formatting", res.formatting);
                    data::from_json(node, "rangeFormatting", res.rangeFormatting);
                    data::from_json(node, "onTypeFormatting", res.onTypeFormatting);
                    data::from_json(node, "rename", res.rename);
                    data::from_json(node, "publishDiagnostics", res.publishDiagnostics);
                    data::from_json(node, "foldingRange", res.foldingRange);
                    data::from_json(node, "selectionRange", res.selectionRange);
                    return res;
                }

                nlohmann::json to_json() const {
                    nlohmann::json json;
                    data::set_json(json, "synchronization", synchronization);
                    data::set_json(json, "completion", completion);
                    data::set_json(json, "hover", hover);
                    data::set_json(json, "signatureHelp", signatureHelp);
                    data::set_json(json, "declaration", declaration);
                    data::set_json(json, "definition", definition);
                    data::set_json(json, "typeDefinition", typeDefinition);
                    data::set_json(json, "implementation", implementation);
                    data::set_json(json, "references", references);
                    data::set_json(json, "documentHighlight", documentHighlight);
                    data::set_json(json, "documentSymbol", documentSymbol);
                    data::set_json(json, "codeAction", codeAction);
                    data::set_json(json, "codeLens", codeLens);
                    data::set_json(json, "documentLink", documentLink);
                    data::set_json(json, "colorProvider", colorProvider);
                    data::set_json(json, "formatting", formatting);
                    data::set_json(json, "rangeFormatting", rangeFormatting);
                    data::set_json(json, "onTypeFormatting", onTypeFormatting);
                    data::set_json(json, "rename", rename);
                    data::set_json(json, "publishDiagnostics", publishDiagnostics);
                    data::set_json(json, "foldingRange", foldingRange);
                    data::set_json(json, "selectionRange", selectionRange);
                    return json;
                }
            };

            struct client_capabilities {
                struct Workspace {
                    /*
                        The client supports applying batch edits
                        to the workspace by supporting the request
                        'workspace/applyEdit'
                    */
                    std::optional<bool> applyEdit;
                    /*
                    Capabilities specific to `WorkspaceEdit`s
                    */
                    std::optional<workspace_edit_client_capabilities> workspaceEdit;
                    /*
                        Capabilities specific to the `workspace/didChangeConfiguration` notification.
                    */
                    std::optional<did_change_configuration_client_capabilities> didChangeConfiguration;
                    /*
                        Capabilities specific to the `workspace/didChangeWatchedFiles` notification.
                    */
                    std::optional<did_change_watched_files_client_capabilities> didChangeWatchedFiles;
                    /*
                        Capabilities specific to the `workspace/symbol` request.
                    */
                    std::optional<workspace_symbol_client_capabilities> symbol;
                    /*
                        Capabilities specific to the `workspace/executeCommand` request.
                    */
                    std::optional<execute_command_client_capabilities> executeCommand;
                    /*
                        The client has support for workspace folders.

                        Since 3.6.0
                    */
                    std::optional<bool> workspaceFolders;
                    /*
                        The client supports `workspace/configuration` requests.

                        Since 3.6.0
                    */
                    std::optional<bool> configuration;

                    static Workspace from_json(const nlohmann::json &node) {
                        Workspace res;
                        data::from_json(node, "applyEdit", res.applyEdit);
                        data::from_json(node, "workspaceEdit", res.workspaceEdit);
                        data::from_json(node, "didChangeConfiguration", res.didChangeConfiguration);
                        data::from_json(node, "didChangeWatchedFiles", res.didChangeWatchedFiles);
                        data::from_json(node, "symbol", res.symbol);
                        data::from_json(node, "executeCommand", res.executeCommand);
                        data::from_json(node, "workspaceFolders", res.workspaceFolders);
                        data::from_json(node, "configuration", res.configuration);
                        return res;
                    }

                    nlohmann::json to_json() const {
                        nlohmann::json json;
                        data::set_json(json, "applyEdit", applyEdit);
                        data::set_json(json, "workspaceEdit", workspaceEdit);
                        data::set_json(json, "didChangeConfiguration", didChangeConfiguration);
                        data::set_json(json, "didChangeWatchedFiles", didChangeWatchedFiles);
                        data::set_json(json, "symbol", symbol);
                        data::set_json(json, "executeCommand", executeCommand);
                        data::set_json(json, "workspaceFolders", workspaceFolders);
                        data::set_json(json, "configuration", configuration);
                        return json;
                    }
                };

                struct Window {
                    /*
                        Whether client supports handling progress notifications. If set servers are allowed to
                        report in `workDoneProgress` property in the request specific server capabilities.

                        Since 3.15.0
                    */
                    std::optional<bool> workDoneProgress;

                    static Window from_json(const nlohmann::json &node) {
                        Window res;
                        data::from_json(node, "workDoneProgress", res.workDoneProgress);
                        return res;
                    }

                    nlohmann::json to_json() const {
                        nlohmann::json json;
                        data::set_json(json, "workDoneProgress", workDoneProgress);
                        return json;
                    }
                };

                /*
                    Workspace specific client capabilities.
                */
                std::optional<Workspace> workspace;
                /*
                    Text document specific client capabilities.
                */
                std::optional<text_document_client_capabilities> textDocument;
                /*
                    Experimental client capabilities.
                */
                std::optional<nlohmann::json> experimental;

                static client_capabilities from_json(const nlohmann::json &node) {
                    client_capabilities res;
                    data::from_json(node, "workspace", res.workspace);
                    data::from_json(node, "textDocument", res.textDocument);
                    res.experimental = node.contains("experimental") ? node["experimental"] : nlohmann::json(nullptr);
                    return res;
                }

                nlohmann::json to_json() const {
                    nlohmann::json json;
                    data::set_json(json, "workspace", workspace);
                    data::set_json(json, "textDocument", textDocument);
                    if (experimental.has_value()) {
                        json["experimental"] = *experimental;
                    }
                    return json;
                }
            };

            struct workspace_folder {
                /*
                    The associated URI for this workspace folder.
                */
                uri uri;
                /*
                    The name of the workspace folder. Used to refer to this
                    workspace folder in the user interface.
                */
                std::string name;

                static workspace_folder from_json(const nlohmann::json &node) {
                    workspace_folder res;
                    data::from_json(node, "uri", res.uri);
                    data::from_json(node, "name", res.name);
                    return res;
                }

                nlohmann::json to_json() const {
                    nlohmann::json json;
                    data::set_json(json, "uri", uri);
                    data::set_json(json, "name", name);
                    return json;
                }
            };

            /*
                The process Id of the parent process that started
                the server. Is null if the process has not been started by another process.
                If the parent process is not alive then the server should exit (see exit notification) its process.
            */
            std::optional<uint64_t> processId;
            /*
                Information about the client

                @since 3.15.0
            */
            client_info clientInfo;
            /*
                The rootPath of the workspace. Is null
                if no folder is open.

                @deprecated in favour of rootUri.
            */
            std::optional<std::string> rootPath;
            /*
                The rootUri of the workspace. Is null if no
                folder is open. If both `rootPath` and `rootUri` are set
                `rootUri` wins.
            */
            std::optional<uri> rootUri;
            /*
                User provided initialization options.
            */
            nlohmann::json initializationOptions;
            /*
                The capabilities provided by the client (editor or tool)
            */
            client_capabilities capabilities;
            /*
                The initial trace setting. If omitted trace is disabled ('off').
            */
            trace_mode trace;
            /**
                The workspace folders configured in the client when the server starts.
                This property is only available if the client supports workspace folders.
                It can be `null` if the client supports workspace folders but none are
                configured.

                @since 3.6.0
            */
            std::optional<std::vector<workspace_folder>> workspace_folders;

            static initialize_params from_json(const nlohmann::json &node) {
                initialize_params res;
                data::from_json(node, "processId", res.processId);
                data::from_json(node, "clientInfo", res.clientInfo);
                data::from_json(node, "rootPath", res.rootPath);
                data::from_json(node, "rootUri", res.rootUri);
                res.initializationOptions = node.contains("initializationOptions")
                                            ? node["initializationOptions"]
                                            : nlohmann::json(nullptr);
                data::from_json(node, "capabilities", res.capabilities);
                data::from_json(node, "trace", res.trace);
                data::from_json(node, "workspaceFolders", res.workspace_folders);
                return res;
            }

            nlohmann::json to_json() const {
                nlohmann::json json;
                data::set_json(json, "processId", processId);
                data::set_json(json, "clientInfo", clientInfo);
                data::set_json(json, "rootPath", rootPath);
                data::set_json(json, "rootUri", rootUri);
                json["initializationOptions"] = initializationOptions;
                data::set_json(json, "capabilities", capabilities);
                data::set_json(json, "trace", trace);
                data::set_json(json, "workspaceFolders", workspace_folders);
                return json;
            }
        };

        struct did_open_text_document_params {
            /**
                * The document that was opened.
                */
            text_document_item textDocument;

            static did_open_text_document_params from_json(const nlohmann::json &node) {
                did_open_text_document_params res;
                data::from_json(node, "textDocument", res.textDocument);
                return res;
            }

            nlohmann::json to_json() const {
                nlohmann::json json;
                data::set_json(json, "textDocument", textDocument);
                return json;
            }
        };

        struct did_change_text_document_params {
            struct text_document_content_change_event {
                /**
                    * The range of the document that changed.
                    */
                std::optional<range> range;

                /**
                    * The optional length of the range that got replaced.
                    *
                    * @deprecated use range instead.
                    */
                std::optional<size_t> rangeLength;

                /**
                    * The new text for the provided range.
                    */
                std::string text;

                static text_document_content_change_event from_json(const nlohmann::json &node) {
                    text_document_content_change_event res;
                    data::from_json(node, "range", res.range);
                    data::from_json(node, "rangeLength", res.rangeLength);
                    data::from_json(node, "text", res.text);
                    return res;
                }

                nlohmann::json to_json() const {
                    nlohmann::json json;
                    data::set_json(json, "range", range);
                    data::set_json(json, "rangeLength", rangeLength);
                    data::set_json(json, "text", text);
                    return json;
                }
            };

            /**
                * The document that did change. The version number points
                * to the version after all provided content changes have
                * been applied.
                */
            versioned_text_document_identifier textDocument;

            /**
                * The actual content changes. The content changes describe single state changes
                * to the document. So if there are two content changes c1 (at array index 0) and
                * c2 (at array index 1) for a document in state S then c1 moves the document from
                * S to S' and c2 from S' to S''. So c1 is computed on the state S and c2 is computed
                * on the state S'.
                *
                * To mirror the content of a document using change events use the following approach:
                * - start with the same initial content
                * - apply the 'textDocument/didChange' notifications in the order you recevie them.
                * - apply the `TextDocumentContentChangeEvent`s in a single notification in the order
                *   you receive them.
                */
            std::vector<text_document_content_change_event> contentChanges;

            static did_change_text_document_params from_json(const nlohmann::json &node) {
                did_change_text_document_params res;
                data::from_json(node, "textDocument", res.textDocument);
                data::from_json(node, "contentChanges", res.contentChanges);
                return res;
            }

            nlohmann::json to_json() const {
                nlohmann::json json;
                data::set_json(json, "textDocument", textDocument);
                data::set_json(json, "contentChanges", contentChanges);
                return json;
            }
        };

        struct will_save_text_document_params {
            /**
                * The document that will be saved.
                */
            text_document_identifier textDocument;

            /**
                * The 'TextDocumentSaveReason'.
                */
            text_document_save_reason reason;

            static will_save_text_document_params from_json(const nlohmann::json &node) {
                will_save_text_document_params res;
                data::from_json(node, "textDocument", res.textDocument);
                data::from_json(node, "reason", res.reason);
                return res;
            }

            nlohmann::json to_json() const {
                nlohmann::json json;
                data::set_json(json, "textDocument", textDocument);
                data::set_json(json, "reason", reason);
                return json;
            }
        };

        struct did_save_text_document_params {
            /**
                * The document that was saved.
                */
            text_document_identifier textDocument;

            /**
                * Optional the content when saved. Depends on the includeText value
                * when the save notification was requested.
                */
            std::optional<std::string> text;

            static did_save_text_document_params from_json(const nlohmann::json &node) {
                did_save_text_document_params res;
                data::from_json(node, "textDocument", res.textDocument);
                data::from_json(node, "text", res.text);
                return res;
            }

            nlohmann::json to_json() const {
                nlohmann::json json;
                data::set_json(json, "textDocument", textDocument);
                data::set_json(json, "text", text);
                return json;
            }
        };

        struct did_close_text_document_params {
            /**
                * The document that was closed.
                */
            text_document_identifier textDocument;

            static did_close_text_document_params from_json(const nlohmann::json &node) {
                did_close_text_document_params res;
                data::from_json(node, "textDocument", res.textDocument);
                return res;
            }

            nlohmann::json to_json() const {
                nlohmann::json json;
                data::set_json(json, "textDocument", textDocument);
                return json;
            }
        };

        struct completion_params {
            struct CompletionContext {
                /**
                    * How the completion was triggered.
                    */
                completion_trigger_kind triggerKind;

                /**
                    * The trigger character (a single character) that has trigger code complete.
                    * Is undefined if `triggerKind !== CompletionTriggerKind.TriggerCharacter`
                    */
                std::optional<std::string> triggerCharacter;

                static CompletionContext from_json(const nlohmann::json &node) {
                    CompletionContext res;
                    data::from_json(node, "triggerKind", res.triggerKind);
                    data::from_json(node, "triggerCharacter", res.triggerCharacter);
                    return res;
                }

                nlohmann::json to_json() const {
                    nlohmann::json json;
                    data::set_json(json, "triggerKind", triggerKind);
                    data::set_json(json, "triggerCharacter", triggerCharacter);
                    return json;
                }
            };

            /**
            * An optional token that a server can use to report partial results (e.g. streaming) to
            * the client.
            */
            std::optional<std::string> partialResultToken;
            /**
            * An optional token that a server can use to report work done progress.
            */
            std::optional<std::string> workDoneToken;
            /**
            * The text document.
            */
            text_document_identifier textDocument;

            /**
            * The position inside the text document.
            */
            position position;
            /**
            * The completion context. This is only available if the client specifies
            * to send this using `ClientCapabilities.textDocument.completion.contextSupport === true`
            */
            std::optional<CompletionContext> context;

            static completion_params from_json(const nlohmann::json &node) {
                completion_params res;
                data::from_json(node, "partialResultToken", res.partialResultToken);
                data::from_json(node, "workDoneToken", res.workDoneToken);
                data::from_json(node, "textDocument", res.textDocument);
                data::from_json(node, "position", res.position);
                data::from_json(node, "context", res.context);
                return res;
            }

            nlohmann::json to_json() const {
                nlohmann::json json;
                data::set_json(json, "partialResultToken", partialResultToken);
                data::set_json(json, "workDoneToken", workDoneToken);
                data::set_json(json, "textDocument", textDocument);
                data::set_json(json, "position", position);
                data::set_json(json, "context", context);
                return json;
            }
        };

        struct references_params {
            struct ReferenceContext {
                /**
                 * Include the declaration of the current symbol.
                 */
                bool includeDeclaration;


                static ReferenceContext from_json(const nlohmann::json &node) {
                    ReferenceContext res;
                    data::from_json(node, "includeDeclaration", res.includeDeclaration);
                    return res;
                }

                nlohmann::json to_json() const {
                    nlohmann::json json;
                    data::set_json(json, "includeDeclaration", includeDeclaration);
                    return json;
                }
            };

            /**
            * An optional token that a server can use to report partial results (e.g. streaming) to
            * the client.
            */
            std::optional<std::string> partialResultToken;
            /**
            * An optional token that a server can use to report work done progress.
            */
            std::optional<std::string> workDoneToken;
            /**
            * The text document.
            */
            text_document_identifier textDocument;

            /**
            * The position inside the text document.
            */
            position position;

            ReferenceContext context;

            static references_params from_json(const nlohmann::json &node) {
                references_params res;
                data::from_json(node, "partialResultToken", res.partialResultToken);
                data::from_json(node, "workDoneToken", res.workDoneToken);
                data::from_json(node, "textDocument", res.textDocument);
                data::from_json(node, "position", res.position);
                data::from_json(node, "context", res.context);
                return res;
            }

            nlohmann::json to_json() const {
                nlohmann::json json;
                data::set_json(json, "partialResultToken", partialResultToken);
                data::set_json(json, "workDoneToken", workDoneToken);
                data::set_json(json, "textDocument", textDocument);
                data::set_json(json, "position", position);
                data::set_json(json, "context", context);
                return json;
            }
        };

        struct publish_diagnostics_params {
            /**
             * The URI for which diagnostic information is reported.
             */
            uri uri;

            /**
             * Optional the version number of the document the diagnostics are published for.
             *
             * @since 3.15.0
             */
            std::optional<size_t> version;

            /**
             * An array of diagnostic information items.
             */
            std::vector<diagnostics> diagnostics;

            static publish_diagnostics_params from_json(const nlohmann::json &node) {
                publish_diagnostics_params res;
                data::from_json(node, "uri", res.uri);
                data::from_json(node, "version", res.version);
                data::from_json(node, "diagnostics", res.diagnostics);
                return res;
            }

            nlohmann::json to_json() const {
                nlohmann::json json;
                data::set_json(json, "uri", uri);
                data::set_json(json, "version", version);
                data::set_json(json, "diagnostics", diagnostics);
                return json;
            }
        };

        struct folding_range_params {
            /**
             * An optional token that a server can use to report partial results (e.g. streaming) to
             * the client.
             */
            std::optional<std::string> partialResultToken;
            /**
             * An optional token that a server can use to report work done progress.
             */
            std::optional<std::string> workDoneToken;
            /**
             * The text document.
             */
            text_document_identifier textDocument;

            static folding_range_params from_json(const nlohmann::json &node) {
                folding_range_params res;
                data::from_json(node, "partialResultToken", res.partialResultToken);
                data::from_json(node, "workDoneToken", res.workDoneToken);
                data::from_json(node, "textDocument", res.textDocument);
                return res;
            }

            nlohmann::json to_json() const {
                nlohmann::json json;
                data::set_json(json, "partialResultToken", partialResultToken);
                data::set_json(json, "workDoneToken", workDoneToken);
                data::set_json(json, "textDocument", textDocument);
                return json;
            }
        };

        /**
         * Represents a color in RGBA space.
         */
        struct color {

            /**
             * The red component of this color in the range [0-1].
             */
            float red;

            /**
             * The green component of this color in the range [0-1].
             */
            float green;

            /**
             * The blue component of this color in the range [0-1].
             */
            float blue;

            /**
             * The alpha component of this color in the range [0-1].
             */
            float alpha;

            static color from_json(const nlohmann::json &node) {
                color res;
                data::from_json(node, "red", res.red);
                data::from_json(node, "green", res.green);
                data::from_json(node, "blue", res.blue);
                data::from_json(node, "alpha", res.alpha);
                return res;
            }

            nlohmann::json to_json() const {
                nlohmann::json json;
                data::set_json(json, "red", red);
                data::set_json(json, "green", green);
                data::set_json(json, "blue", blue);
                data::set_json(json, "alpha", alpha);
                return json;
            }
        };

        struct color_information {
            /**
             * The range in the document where this color appears.
             */
            range range;

            /**
             * The actual color value for this color range.
             */
            color color;

            static color_information from_json(const nlohmann::json &node) {
                color_information res;
                data::from_json(node, "range", res.range);
                data::from_json(node, "color", res.color);
                return res;
            }

            nlohmann::json to_json() const {
                nlohmann::json json;
                data::set_json(json, "range", range);
                data::set_json(json, "color", color);
                return json;
            }
        };

        struct document_color_params {
            /**
             * An optional token that a server can use to report partial results (e.g. streaming) to
             * the client.
             */
            std::optional<std::string> partialResultToken;
            /**
             * An optional token that a server can use to report work done progress.
             */
            std::optional<std::string> workDoneToken;
            /**
             * The text document.
             */
            text_document_identifier textDocument;

            static document_color_params from_json(const nlohmann::json &node) {
                document_color_params res;
                data::from_json(node, "partialResultToken", res.partialResultToken);
                data::from_json(node, "workDoneToken", res.workDoneToken);
                data::from_json(node, "textDocument", res.textDocument);
                return res;
            }

            nlohmann::json to_json() const {
                nlohmann::json json;
                data::set_json(json, "partialResultToken", partialResultToken);
                data::set_json(json, "workDoneToken", workDoneToken);
                data::set_json(json, "textDocument", textDocument);
                return json;
            }
        };

        struct did_change_configuration_params {
            /**
             * The actual changed settings
             */
            std::optional<nlohmann::json> settings;

            static did_change_configuration_params from_json(const nlohmann::json &node) {
                did_change_configuration_params res;
                res.settings = node.contains("settings") ? node["settings"] : nlohmann::json(nullptr);
                return res;
            }

            nlohmann::json to_json() const {
                nlohmann::json json;
                if (settings.has_value()) {
                    json["settings"] = *settings;
                }
                return json;
            }
        };

        struct color_presentation {
            /**
             * The label of this color presentation. It will be shown on the color
             * picker header. By default this is also the text that is inserted when selecting
             * this color presentation.
             */
            std::string label;
            /**
             * An [edit](#TextEdit) which is applied to a document when selecting
             * this presentation for the color.  When `falsy` the [label](#ColorPresentation.label)
             * is used.
             */
            std::optional<text_edit> textEdit;
            /**
             * An optional array of additional [text edits](#TextEdit) that are applied when
             * selecting this color presentation. Edits must not overlap with the main [edit](#ColorPresentation.textEdit) nor with themselves.
             */
            std::optional<std::vector<text_edit>> additionalTextEdits;

            static color_presentation from_json(const nlohmann::json &node) {
                color_presentation res;
                data::from_json(node, "label", res.label);
                data::from_json(node, "textEdit", res.textEdit);
                data::from_json(node, "additionalTextEdits", res.additionalTextEdits);
                return res;
            }

            nlohmann::json to_json() const {
                nlohmann::json json;
                data::set_json(json, "label", label);
                data::set_json(json, "textEdit", textEdit);
                data::set_json(json, "additionalTextEdits", additionalTextEdits);
                return json;
            }
        };

        struct color_presentation_params {
            /**
             * An optional token that a server can use to report partial results (e.g. streaming) to
             * the client.
             */
            std::optional<std::string> partialResultToken;
            /**
             * An optional token that a server can use to report work done progress.
             */
            std::optional<std::string> workDoneToken;
            /**
             * The text document.
             */
            text_document_identifier textDocument;

            /**
             * The color information to request presentations for.
             */
            color color;

            /**
             * The range where the color would be inserted. Serves as a context.
             */
            range range;

            static color_presentation_params from_json(const nlohmann::json &node) {
                color_presentation_params res;
                data::from_json(node, "partialResultToken", res.partialResultToken);
                data::from_json(node, "workDoneToken", res.workDoneToken);
                data::from_json(node, "textDocument", res.textDocument);
                data::from_json(node, "color", res.color);
                data::from_json(node, "range", res.range);
                return res;
            }

            nlohmann::json to_json() const {
                nlohmann::json json;
                data::set_json(json, "partialResultToken", partialResultToken);
                data::set_json(json, "workDoneToken", workDoneToken);
                data::set_json(json, "textDocument", textDocument);
                data::set_json(json, "color", color);
                data::set_json(json, "range", range);
                return json;
            }
        };

        struct log_message_params {
            /**
             * The message type. See {@link MessageType}
             */
            message_type type;

            /**
             * The actual message
             */
            std::string message;

            static log_message_params from_json(const nlohmann::json &node) {
                log_message_params res;
                data::from_json(node, "type", res.type);
                data::from_json(node, "message", res.message);
                return res;
            }

            nlohmann::json to_json() const {
                nlohmann::json json;
                data::set_json(json, "type", type);
                data::set_json(json, "message", message);
                return json;
            }
        };
    }


    class server {

    private:
        bool m_die;
    public:
        jsonrpc rpc;

        server() : rpc(std::cin, std::cout, jsonrpc::detach, jsonrpc::skip), m_die(false) {
            rpc.register_method(
                    "initialize", [&](jsonrpc &rpc, const jsonrpc::rpcmessage &msg) {
                        try {
                            auto params = data::initialize_params::from_json(msg.params.value());
                            auto res = on_initialize(params);
                            rpc.send({msg.id, res.to_json()});
                            after_initialize(params);
                        }
                        catch (const std::exception &e) {
                            std::stringstream sstream;
                            sstream << "rpc call 'initialize' failed with: '" << e.what() << "'.";
                            window_logMessage(::lsp::data::message_type::Log, sstream.str());
                        }
                    });
            rpc.register_method(
                    "shutdown", [&](jsonrpc &rpc, const jsonrpc::rpcmessage &msg) {
                        kill();
                        on_shutdown();
                    });
            rpc.register_method(
                    "textDocument/didOpen", [&](jsonrpc &rpc, const jsonrpc::rpcmessage &msg) {
                        try {
                            auto params = data::did_open_text_document_params::from_json(msg.params.value());
                            on_textDocument_didOpen(params);
                        }
                        catch (const std::exception &e) {
                            std::stringstream sstream;
                            sstream << "rpc call 'textDocument/didOpen' failed with: '" << e.what() << "'.";
                            window_logMessage(::lsp::data::message_type::Log, sstream.str());
                        }
                    });
            rpc.register_method(
                    "textDocument/didChange", [&](jsonrpc &rpc, const jsonrpc::rpcmessage &msg) {
                        try {
                            auto params = data::did_change_text_document_params::from_json(msg.params.value());
                            on_textDocument_didChange(params);
                        }
                        catch (const std::exception &e) {
                            std::stringstream sstream;
                            sstream << "rpc call 'textDocument/didChange' failed with: '" << e.what() << "'.";
                            window_logMessage(::lsp::data::message_type::Log, sstream.str());
                        }
                    });
            rpc.register_method(
                    "textDocument/willSave", [&](jsonrpc &rpc, const jsonrpc::rpcmessage &msg) {
                        try {
                            auto params = data::will_save_text_document_params::from_json(msg.params.value());
                            on_textDocument_willSave(params);
                        }
                        catch (const std::exception &e) {
                            std::stringstream sstream;
                            sstream << "rpc call 'textDocument/willSave' failed with: '" << e.what() << "'.";
                            window_logMessage(::lsp::data::message_type::Log, sstream.str());
                        }
                    });
            rpc.register_method(
                    "textDocument/willSaveWaitUntil", [&](jsonrpc &rpc, const jsonrpc::rpcmessage &msg) {
                        try {
                            auto params = data::will_save_text_document_params::from_json(msg.params.value());
                            auto res = on_textDocument_willSaveWaitUntil(params);
                            rpc.send({msg.id, res.has_value() ? res->to_json() : nlohmann::json(nullptr)});
                        }
                        catch (const std::exception &e) {
                            std::stringstream sstream;
                            sstream << "rpc call 'textDocument/willSaveWaitUntil' failed with: '" << e.what() << "'.";
                            window_logMessage(::lsp::data::message_type::Log, sstream.str());
                        }
                    });
            rpc.register_method(
                    "textDocument/didSave", [&](jsonrpc &rpc, const jsonrpc::rpcmessage &msg) {
                        try {
                            auto params = data::did_save_text_document_params::from_json(msg.params.value());
                            on_textDocument_didSave(params);
                        }
                        catch (const std::exception &e) {
                            std::stringstream sstream;
                            sstream << "rpc call 'textDocument/didSave' failed with: '" << e.what() << "'.";
                            window_logMessage(::lsp::data::message_type::Log, sstream.str());
                        }
                    });
            rpc.register_method(
                    "textDocument/didClose", [&](jsonrpc &rpc, const jsonrpc::rpcmessage &msg) {
                        try {
                            auto params = data::did_close_text_document_params::from_json(msg.params.value());
                            on_textDocument_didClose(params);
                        }
                        catch (const std::exception &e) {
                            std::stringstream sstream;
                            sstream << "rpc call 'textDocument/didClose' failed with: '" << e.what() << "'.";
                            window_logMessage(::lsp::data::message_type::Log, sstream.str());
                        }
                    });
            rpc.register_method(
                    "textDocument/completion", [&](jsonrpc &rpc, const jsonrpc::rpcmessage &msg) {
                        try {
                            auto params = data::completion_params::from_json(msg.params.value());
                            auto res = on_textDocument_completion(params);
                            rpc.send({msg.id, res.has_value() ? res->to_json() : nlohmann::json(nullptr)});
                        }
                        catch (const std::exception &e) {
                            std::stringstream sstream;
                            sstream << "rpc call 'textDocument/completion' failed with: '" << e.what() << "'.";
                            window_logMessage(::lsp::data::message_type::Log, sstream.str());
                        }
                    });
            rpc.register_method(
                    "textDocument/foldingRange", [&](jsonrpc &rpc, const jsonrpc::rpcmessage &msg) {
                        try {
                            auto params = data::folding_range_params::from_json(msg.params.value());
                            auto res = on_textDocument_foldingRange(params);
                            rpc.send({msg.id, res.has_value() ? to_json(*res) : nlohmann::json(nullptr)});
                        }
                        catch (const std::exception &e) {
                            std::stringstream sstream;
                            sstream << "rpc call 'textDocument/foldingRange' failed with: '" << e.what() << "'.";
                            window_logMessage(::lsp::data::message_type::Log, sstream.str());
                        }
                    });
            rpc.register_method(
                    "textDocument/documentColor", [&](jsonrpc &rpc, const jsonrpc::rpcmessage &msg) {
                        try {
                            auto params = data::document_color_params::from_json(msg.params.value());
                            auto res = on_textDocument_documentColor(params);
                            rpc.send({msg.id, to_json(res)});
                        }
                        catch (const std::exception &e) {
                            std::stringstream sstream;
                            sstream << "rpc call 'textDocument/documentColor' failed with: '" << e.what() << "'.";
                            window_logMessage(::lsp::data::message_type::Log, sstream.str());
                        }
                    });
            rpc.register_method(
                    "textDocument/references", [&](jsonrpc &rpc, const jsonrpc::rpcmessage &msg) {
                        try {
                            auto params = data::references_params::from_json(msg.params.value());
                            auto res = on_textDocument_references(params);
                            rpc.send({msg.id, to_json(res)});
                        }
                        catch (const std::exception &e) {
                            std::stringstream sstream;
                            sstream << "rpc call 'workspace/didChangeConfiguration' failed with: '" << e.what() << "'.";
                            window_logMessage(::lsp::data::message_type::Log, sstream.str());
                        }
                    });
            rpc.register_method(
                    "textDocument/colorPresentation", [&](jsonrpc &rpc, const jsonrpc::rpcmessage &msg) {
                        try {
                            auto params = data::color_presentation_params::from_json(msg.params.value());
                            auto res = on_textDocument_colorPresentation(params);
                            rpc.send({msg.id, to_json(res)});
                        }
                        catch (const std::exception &e) {
                            std::stringstream sstream;
                            sstream << "rpc call 'textDocument/colorPresentation' failed with: '" << e.what() << "'.";
                            window_logMessage(::lsp::data::message_type::Log, sstream.str());
                        }
                    });
            rpc.register_method(
                    "workspace/didChangeConfiguration", [&](jsonrpc &rpc, const jsonrpc::rpcmessage &msg) {
                        try {
                            auto params = data::did_change_configuration_params::from_json(msg.params.value());
                            on_workspace_didChangeConfiguration(params);
                        }
                        catch (const std::exception &e) {
                            std::stringstream sstream;
                            sstream << "rpc call 'workspace/didChangeConfiguration' failed with: '" << e.what() << "'.";
                            window_logMessage(::lsp::data::message_type::Log, sstream.str());
                        }
                    });
        }

        void listen() {
            while (!m_die) {
                if (!rpc.handle_single_message()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    continue;
                }
            }
        }

        void kill() {
            m_die = true;
        }

        // Methods that must be overriden by clients
    protected:
        virtual lsp::data::initialize_result on_initialize(const lsp::data::initialize_params &params) = 0;

        virtual void after_initialize(const lsp::data::initialize_params &params) {
        }

        virtual void on_shutdown() = 0;


        // Methods that can be overriden by implementing clients
    protected:
        virtual void on_textDocument_didOpen(const lsp::data::did_open_text_document_params &params) {
        }

        virtual void on_textDocument_didChange(const lsp::data::did_change_text_document_params &params) {
        }

        virtual void on_textDocument_willSave(const lsp::data::will_save_text_document_params &params) {
        }

        virtual std::optional<lsp::data::text_edit>
        on_textDocument_willSaveWaitUntil(const lsp::data::will_save_text_document_params &params) {
            return {};
        }

        virtual void on_textDocument_didSave(const lsp::data::did_save_text_document_params &params) {
        }

        virtual void on_textDocument_didClose(const lsp::data::did_close_text_document_params &params) {
        }

        virtual std::optional<lsp::data::completion_list>
        on_textDocument_completion(const lsp::data::completion_params &params) {
            return {};
        }

        virtual std::optional<std::vector<lsp::data::folding_range>>
        on_textDocument_foldingRange(const lsp::data::folding_range_params &params) {
            return {};
        }

        virtual std::vector<lsp::data::color_information>
        on_textDocument_documentColor(const lsp::data::document_color_params &params) {
            return {};
        }

        virtual std::vector<lsp::data::color_presentation>
        on_textDocument_colorPresentation(const lsp::data::color_presentation_params &params) {
            return {};
        }

        virtual std::optional<std::vector<lsp::data::location>> on_textDocument_references(const lsp::data::references_params &params) {
            return {};
        }

        virtual void on_workspace_didChangeConfiguration(const lsp::data::did_change_configuration_params &params) {
        }

    public:
        void textDocument_publishDiagnostics(const lsp::data::publish_diagnostics_params &params) {
            rpc.send({{}, "textDocument/publishDiagnostics", params.to_json()});
        }

        void window_logMessage(lsp::data::message_type type, std::string message) {
            rpc.send({{}, "window/logMessage", lsp::data::log_message_params{type, message}.to_json()});
        }

        void window_logMessage(const lsp::data::log_message_params &params) {
            rpc.send({{}, "window/logMessage", params.to_json()});
        }
    };
}

#endif // SQFVM_LANGUAGE_SERVER_LSP_LSPSERVER_HPP