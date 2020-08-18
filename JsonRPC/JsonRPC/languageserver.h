#pragma once
#include "jsonrpc.h"

#include <optional>
#include <string>
#include <nlohmann/json.hpp>
#include <vector>
#include <variant>

namespace lsp
{
    namespace data
    {
        template<typename T>
        void from_json(const nlohmann::json& node, const char* key, T& t)
        {
            t = T::from_json(node[key]);
        }
        template<typename T>
        void from_json(const nlohmann::json& node, const char* key, std::vector<T>& t)
        {
            t = std::vector();
            for (auto subnode : node[key])
            {
                t.push_back(T::from_json(subnode));
            }
        }
        template<typename T>
        void from_json(const nlohmann::json& node, const char* key, std::optional<T>& opt)
        {
            if (node.contains(key))
            {
                opt = T::from_json(node);
            }
            else
            {
                opt = {};
            }
        }
        template<typename T>
        nlohmann::json to_json(const T& t)
        {
            return t.to_json();
        }
        template<typename T>
        nlohmann::json to_json(std::vector<T>& ts)
        {
            nlohmann::json json;
            for (auto t : ts)
            {
                json.push_back(T::from_json(subnode));
            }
            return json;
        }
        template<typename T>
        nlohmann::json to_json(std::optional<T>& opt)
        {
            if (opt.has_value())
            {
                return to_json(*opt);
            }
            return nlohmann::json(nullptr);
        }


        enum class resource_operations
        {
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
        resource_operations operator | (resource_operations lhs, resource_operations rhs)
        {
            return static_cast<resource_operations> (
                static_cast<std::underlying_type<resource_operations>::type>(lhs) |
                static_cast<std::underlying_type<resource_operations>::type>(rhs)
                );
        }
        resource_operations operator & (resource_operations lhs, resource_operations rhs)
        {
            return static_cast<resource_operations> (
                static_cast<std::underlying_type<resource_operations>::type>(lhs) &
                static_cast<std::underlying_type<resource_operations>::type>(rhs)
                );
        }
        template<>
        void from_json<resource_operations>(const nlohmann::json& node, const char* key, resource_operations& t)
        {
            t = resource_operations::Empty;
            for (auto resourceOperationJson : node[key])
            {
                std::string resourceOperationString = resourceOperationJson;
                if (resourceOperationString == "create") { t = t | resource_operations::Create; }
                else if (resourceOperationString == "rename") { t = t | resource_operations::Rename; }
                else if (resourceOperationString == "delete") { t = t | resource_operations::Delete; }
            }
        }
        template<>
        nlohmann::json to_json<resource_operations>(const resource_operations& t)
        {
            nlohmann::json arr;
            if ((t & resource_operations::Create) == resource_operations::Create) { arr.push_back("create"); }
            if ((t & resource_operations::Delete) == resource_operations::Delete) { arr.push_back("delete"); }
            if ((t & resource_operations::Rename) == resource_operations::Rename) { arr.push_back("rename"); }
            return arr;
        }

        enum class failure_handling
        {
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
        template<>
        void from_json<failure_handling>(const nlohmann::json& node, const char* key, failure_handling& t)
        {

            t = failure_handling::empty;
            std::string actual = node[key];
            if (actual == "abort") { t = failure_handling::abort; return; }
            if (actual == "transactional") { t = failure_handling::transactional; return; }
            if (actual == "textOnlyTransactional") { t = failure_handling::textOnlyTransactional; return; }
            if (actual == "undo") { t = failure_handling::undo; return; }
        }
        template<>
        nlohmann::json to_json<failure_handling>(const failure_handling& t)
        {
            switch (t)
            {
            case failure_handling::abort: return "abort";
            case failure_handling::transactional: return "transactional";
            case failure_handling::textOnlyTransactional: return "textOnlyTransactional";
            case failure_handling::undo: return "undo";
            default: return {};
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
        void from_json<symbol_kind>(const nlohmann::json& node, const char* key, symbol_kind& t)
        {
            t = static_cast<symbol_kind>(node[key].get<int>());
        }
        template<>
        nlohmann::json to_json<symbol_kind>(const symbol_kind& t)
        {
            return static_cast<int>(t);
        }

        enum class trace_mode
        {
            off,
            message,
            verbose
        };
        template<>
        void from_json<trace_mode>(const nlohmann::json& node, const char* key, trace_mode& t)
        {

            t = trace_mode::off;
            std::string actual = node[key];
            if (actual == "message") { t = trace_mode::message; return; }
            if (actual == "verbose") { t = trace_mode::verbose; return; }
        }
        template<>
        nlohmann::json to_json<trace_mode>(const trace_mode& t)
        {
            switch (t)
            {
            case trace_mode::off: return "off";
            case trace_mode::message: return "message";
            case trace_mode::verbose: return "verbose";
            default: return {};
            }
        }

        enum class completion_item_tag
        {
            Deprecated = 1
        };
        template<>
        void from_json<completion_item_tag>(const nlohmann::json& node, const char* key, completion_item_tag& t)
        {
            t = static_cast<completion_item_tag>(node[key].get<int>());
        }
        template<>
        nlohmann::json to_json<completion_item_tag>(const completion_item_tag& t)
        {
            return static_cast<int>(t);
        }

        enum class markup_kind
        {
            /**
             * Plain text is supported as a content format
             * Value String: "plaintext"
             */
            PlainText,
            /**
             * Markdown is supported as a content format
             * Value String: "markdown
             */
            Markdown
        };
        template<>
        void from_json<markup_kind>(const nlohmann::json& node, const char* key, markup_kind& t)
        {

            t = markup_kind::PlainText;
            std::string actual = node[key];
            if (actual == "plaintext") { t = markup_kind::PlainText; return; }
            if (actual == "markdown") { t = markup_kind::Markdown; return; }
        }
        template<>
        nlohmann::json to_json<markup_kind>(const markup_kind& t)
        {
            switch (t)
            {
            case markup_kind::PlainText: return "plaintext";
            case markup_kind::Markdown: return "markdown";
            default: return {};
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
        void from_json<completion_item_kind>(const nlohmann::json& node, const char* key, completion_item_kind& t)
        {
            t = static_cast<completion_item_kind>(node[key].get<int>());
        }
        template<>
        nlohmann::json to_json<completion_item_kind>(const completion_item_kind& t)
        {
            return static_cast<int>(t);
        }

        enum class diagnostic_tag
        {
            /**
             * Unused or unnecessary code.
             *
             * Clients are allowed to render diagnostics with this tag faded out instead of having
             * an error squiggle.
             */
            Unnecessary = 1,
            /**
             * Deprecated or obsolete code.
             *
             * Clients are allowed to rendered diagnostics with this tag strike through.
             */
            Deprecated = 2
        };
        template<>
        void from_json<diagnostic_tag>(const nlohmann::json& node, const char* key, diagnostic_tag& t)
        {
            t = static_cast<diagnostic_tag>(node[key].get<int>());
        }
        template<>
        nlohmann::json to_json<diagnostic_tag>(const diagnostic_tag& t)
        {
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
        void from_json<code_action_kind>(const nlohmann::json& node, const char* key, code_action_kind& t)
        {

            t = code_action_kind::Empty;
            std::string actual = node[key];
            if (actual == "quickfix") { t = code_action_kind::QuickFix; return; }
            if (actual == "refactor") { t = code_action_kind::Refactor; return; }
            if (actual == "refactor.extract") { t = code_action_kind::RefactorExtract; return; }
            if (actual == "refactor.inline") { t = code_action_kind::RefactorInline; return; }
            if (actual == "refactor.rewrite") { t = code_action_kind::RefactorRewrite; return; }
            if (actual == "source") { t = code_action_kind::Source; return; }
            if (actual == "source.organizeImports") { t = code_action_kind::SourceOrganizeImports; return; }
        }
        template<>
        nlohmann::json to_json<code_action_kind>(const code_action_kind& t)
        {
            switch (t)
            {
            case code_action_kind::QuickFix: return "quickfix";
            case code_action_kind::Refactor: return "refactor";
            case code_action_kind::RefactorExtract: return "refactor.extract";
            case code_action_kind::RefactorInline: return "refactor.inline";
            case code_action_kind::RefactorRewrite: return "refactor.rewrite";
            case code_action_kind::Source: return "source";
            case code_action_kind::SourceOrganizeImports: return "source.organizeImports";
            default: return {};
            }
        }

        enum class text_document_sync_kind
        {
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
        void from_json<text_document_sync_kind>(const nlohmann::json& node, const char* key, text_document_sync_kind& t)
        {
            t = static_cast<text_document_sync_kind>(node[key].get<int>());
        }
        template<>
        nlohmann::json to_json<text_document_sync_kind>(const text_document_sync_kind& t)
        {
            return static_cast<int>(t);
        }

        enum class initialize_error
        {
            unknownProtocolVersion = 1
        };
        template<>
        void from_json<initialize_error>(const nlohmann::json& node, const char* key, initialize_error& t)
        {
            t = static_cast<initialize_error>(node[key].get<int>());
        }
        template<>
        nlohmann::json to_json<initialize_error>(const initialize_error& t)
        {
            return static_cast<int>(t);
        }
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
             * - `{}` to group conditions (e.g. `**​/*.{ts,js}` matches all TypeScript and JavaScript files)
             * - `[]` to declare a range of characters to match in a path segment (e.g., `example.[0-9]` to match on `example.0`, `example.1`, …)
             * - `[!...]` to negate a range of characters to match in a path segment (e.g., `example.[!0-9]` to match on `example.a`, `example.b`, but not `example.0`)
             */
            std::string pattern;
        };
        namespace responses
        {
            struct initialize_error
            {
                /**
                 * Indicates whether the client execute the following retry logic:
                 * (1) show the message provided by the ResponseError to the user
                 * (2) user selects retry or cancel
                 * (3) if user selected retry the initialize method is sent again.
                 */
                bool retry;
            };
            struct initialize_result
            {
                struct server_capabilities
                {
                    struct text_document_sync_options
                    {
                        /**
                         * Open and close notifications are sent to the server. If omitted open close notification should not
                         * be sent.
                         */
                        std::optional<bool> openClose;

                        /**
                         * Change notifications are sent to the server. See TextDocumentSyncKind.None, TextDocumentSyncKind.Full
                         * and TextDocumentSyncKind.Incremental. If omitted it defaults to TextDocumentSyncKind.None.
                         */
                        std::optional<text_document_sync_kind> change;
                    };
                    struct completion_options
                    {
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
                    };
                    struct hover_options
                    {
                        std::optional<bool> workDoneProgress;
                    };
                    struct signature_help_options
                    {
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
                    };
                    struct declaration_registration_options
                    {
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
                    };
                    struct definition_options
                    {
                        std::optional<bool> workDoneProgress;
                    };
                    struct type_definition_registration_options
                    {
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
                    };
                    struct implementation_registration_options
                    {
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
                    };
                    struct reference_options
                    {
                        std::optional<bool> workDoneProgress;
                    };
                    struct document_highlight_options
                    {
                        std::optional<bool> workDoneProgress;
                    };
                    struct document_symbol_options
                    {
                        std::optional<bool> workDoneProgress;
                    };
                    struct code_action_options
                    {
                        std::optional<bool> workDoneProgress;
                        /**
                         * CodeActionKinds that this server may return.
                         *
                         * The list of kinds may be generic, such as `CodeActionKind.Refactor`, or the server
                         * may list out every specific kind they provide.
                         */
                        std::optional<std::vector<code_action_kind>> codeActionKinds;
                    };
                    struct code_lens_options
                    {
                        std::optional<bool> workDoneProgress;
                        /**
                         * Code lens has a resolve provider as well.
                         */
                        std::optional<bool> resolveProvider;
                    };
                    struct document_link_options
                    {
                        std::optional<bool> workDoneProgress;
                        /**
                         * Code lens has a resolve provider as well.
                         */
                        std::optional<bool> resolveProvider;
                    };
                    struct document_color_registration_options
                    {
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
                    };
                    struct document_formatting_options
                    {
                        std::optional<bool> workDoneProgress;
                    };
                    struct document_range_formatting_options
                    {
                        std::optional<bool> workDoneProgress;
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
                    };
                    struct rename_options
                    {
                        std::optional<bool> workDoneProgress;
                        /**
                         * Renames should be checked and tested before being executed.
                         */
                        std::optional<bool> prepareProvider;
                    };
                    struct folding_range_registration_options
                    {
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
                    };
                    struct execute_command_options
                    {
                        std::optional<bool> workDoneProgress;
                        /**
                         * The commands to be executed on the server.
                         */
                        std::optional<std::vector<std::string>> commands;
                    };
                    struct selection_range_registration_options
                    {
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
                    };
                    struct workspace_folders_server_capabilities
                    {
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
                         */
                        std::optional<std::variant<std::string, bool>> changeNotifications;
                    };
                    struct Workspace
                    {
                        /**
                         * The server supports workspace folder.
                         *
                         * @since 3.6.0
                         */
                        std::optional<workspace_folders_server_capabilities> workspaceFolders;
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
                };
                struct server_info
                {
                    std::string name;
                    std::optional<std::string> version;
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
            };
        }
        namespace requests
        { // See https://microsoft.github.io/language-server-protocol/specifications/specification-current/#initialize
            struct initialize_params
            {
                struct client_info
                {
                    std::string name;
                    std::string version;
                    
                    static client_info from_json(const nlohmann::json& node)
                    {
                        return
                        {
                            node["name"],
                            node["version"]
                        };
                    }
                    nlohmann::json to_json() const
                    {
                        return
                        {
                            { "name", name },
                            { "version", version }
                        };
                    }
                };
                struct workspace_edit_client_capabilities
                {
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

                    static workspace_edit_client_capabilities from_json(const nlohmann::json& node)
                    {
                        workspace_edit_client_capabilities cap;
                        data::from_json(node, "documentChanges", cap.documentChanges);
                        data::from_json(node, "resourceOperations", cap.resourceOperations);
                        data::from_json(node, "failureHandling", cap.failureHandling);
                        return cap;
                    }
                    nlohmann::json to_json() const
                    {
                        nlohmann::json json;
                        json["documentChanges"] = data::to_json(documentChanges);
                        json["resourceOperations"] = data::to_json(resourceOperations);
                        json["failureHandling"] = data::to_json(failureHandling);
                        return json;
                    }
                };
                struct did_change_configuration_client_capabilities
                {
                    /*
                       Did change configuration notification supports dynamic registration.
                    */
                    std::optional<bool> dynamicRegistration;

                    static did_change_configuration_client_capabilities from_json(const nlohmann::json& node)
                    {
                        did_change_configuration_client_capabilities res;
                        data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                        return res;
                    }
                    nlohmann::json to_json() const
                    {
                        nlohmann::json json;
                        json["dynamicRegistration"] = data::to_json(dynamicRegistration);
                        return json;
                    }
                };
                struct did_change_watched_files_client_capabilities
                {
                    /*
                       Did change watched files notification supports dynamic registration. Please note
                       that the current protocol doesn't support static configuration for file changes
                       from the server side.
                    */
                    std::optional<bool> dynamicRegistration;

                    static did_change_watched_files_client_capabilities from_json(const nlohmann::json& node)
                    {
                        did_change_watched_files_client_capabilities res;
                        data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                        return res;
                    }
                    nlohmann::json to_json() const
                    {
                        nlohmann::json json;
                        json["dynamicRegistration"] = data::to_json(dynamicRegistration);
                        return json;
                    }
                };
                struct workspace_symbol_client_capabilities
                {
                    struct SymbolKind
                    {
                        std::optional<std::vector<symbol_kind>> valueSet;

                        static SymbolKind from_json(const nlohmann::json& node)
                        {
                            SymbolKind res;
                            data::from_json(node, "valueSet", res.valueSet);
                            return res;
                        }
                        nlohmann::json to_json() const
                        {
                            nlohmann::json json;
                            json["valueSet"] = data::to_json(valueSet);
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

                    static workspace_symbol_client_capabilities from_json(const nlohmann::json& node)
                    {
                        workspace_symbol_client_capabilities res;
                        data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                        data::from_json(node, "symbolKind", res.symbolKind);
                        return res;
                    }
                    nlohmann::json to_json() const
                    {
                        nlohmann::json json;
                        json["dynamicRegistration"] = data::to_json(dynamicRegistration);
                        json["symbolKind"] = data::to_json(symbolKind);
                        return json;
                    }
                };
                struct execute_command_client_capabilities
                {
                    /*
                       Execute command supports dynamic registration.
                    */
                    std::optional<bool> dynamicRegistration;

                    static execute_command_client_capabilities from_json(const nlohmann::json& node)
                    {
                        execute_command_client_capabilities res;
                        data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                        return res;
                    }
                    nlohmann::json to_json() const
                    {
                        nlohmann::json json;
                        json["dynamicRegistration"] = data::to_json(dynamicRegistration);
                        return json;
                    }
                };
                struct text_document_sync_client_capabilities
                {
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

                    static text_document_sync_client_capabilities from_json(const nlohmann::json& node)
                    {
                        text_document_sync_client_capabilities res;
                        data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                        data::from_json(node, "willSave", res.willSave);
                        data::from_json(node, "willSaveWaitUntil", res.willSaveWaitUntil);
                        data::from_json(node, "didSave", res.didSave);
                        return res;
                    }
                    nlohmann::json to_json() const
                    {
                        nlohmann::json json;
                        json["dynamicRegistration"] = data::to_json(dynamicRegistration);
                        json["willSave"] = data::to_json(willSave);
                        json["willSaveWaitUntil"] = data::to_json(willSaveWaitUntil);
                        json["didSave"] = data::to_json(didSave);
                        return json;
                    }
                };
                struct completion_client_capabilities
                {
                    struct CompletionItem
                    {
                        struct TagSupport
                        {
                            /**
                             * The tags supported by the client.
                             */
                            std::optional<std::vector<completion_item_tag>> valueSet;

                            static TagSupport from_json(const nlohmann::json& node)
                            {
                                TagSupport res;
                                data::from_json(node, "valueSet", res.valueSet);
                                return res;
                            }
                            nlohmann::json to_json() const
                            {
                                nlohmann::json json;
                                json["valueSet"] = data::to_json(valueSet);
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

                        static CompletionItem from_json(const nlohmann::json& node)
                        {
                            CompletionItem res;
                            data::from_json(node, "snippetSupport", res.snippetSupport);
                            data::from_json(node, "commitCharactersSupport", res.commitCharactersSupport);
                            data::from_json(node, "documentationFormat", res.documentationFormat);
                            data::from_json(node, "deprecatedSupport", res.deprecatedSupport);
                            data::from_json(node, "preselectSupport", res.preselectSupport);
                            data::from_json(node, "tagSupport", res.tagSupport);
                            return res;
                        }
                        nlohmann::json to_json() const
                        {
                            nlohmann::json json;
                            json["snippetSupport"] = data::to_json(snippetSupport);
                            json["commitCharactersSupport"] = data::to_json(commitCharactersSupport);
                            json["documentationFormat"] = data::to_json(documentationFormat);
                            json["deprecatedSupport"] = data::to_json(deprecatedSupport);
                            json["preselectSupport"] = data::to_json(preselectSupport);
                            json["tagSupport"] = data::to_json(tagSupport);
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

                };
                struct hover_client_capabilities
                {
                    /**
                     * Whether hover supports dynamic registration.
                     */
                    std::optional<bool> dynamicRegistration;

                    /**
                     * Client supports the follow content formats for the content
                     * property. The order describes the preferred format of the client.
                     */
                    std::optional<std::vector<markup_kind>> contentFormat;

                    static hover_client_capabilities from_json(const nlohmann::json& node)
                    {
                        hover_client_capabilities res;
                        data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                        data::from_json(node, "contentFormat", res.contentFormat);
                        return res;
                    }
                    nlohmann::json to_json() const
                    {
                        nlohmann::json json;
                        json["dynamicRegistration"] = data::to_json(dynamicRegistration);
                        json["contentFormat"] = data::to_json(contentFormat);
                        return json;
                    }
                };
                struct signature_help_client_capabilities
                {
                    struct SignatureInformation
                    {
                        struct ParameterInformation
                        {
                            /**
                             * The client supports processing label offsets instead of a
                             * simple label string.
                             *
                             * @since 3.14.0
                             */
                            std::optional<bool> labelOffsetSupport;
                            static ParameterInformation from_json(const nlohmann::json& node)
                            {
                                ParameterInformation res;
                                data::from_json(node, "labelOffsetSupport", res.labelOffsetSupport);
                                return res;
                            }
                            nlohmann::json to_json() const
                            {
                                nlohmann::json json;
                                json["labelOffsetSupport"] = data::to_json(labelOffsetSupport);
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

                        static SignatureInformation from_json(const nlohmann::json& node)
                        {
                            SignatureInformation res;
                            data::from_json(node, "documentationFormat", res.documentationFormat);
                            data::from_json(node, "parameterInformation", res.parameterInformation);
                            return res;
                        }
                        nlohmann::json to_json() const
                        {
                            nlohmann::json json;
                            json["documentationFormat"] = data::to_json(documentationFormat);
                            json["parameterInformation"] = data::to_json(parameterInformation);
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

                    static signature_help_client_capabilities from_json(const nlohmann::json& node)
                    {
                        signature_help_client_capabilities res;
                        data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                        data::from_json(node, "signatureInformation", res.signatureInformation);
                        data::from_json(node, "contextSupport", res.contextSupport);
                        return res;
                    }
                    nlohmann::json to_json() const
                    {
                        nlohmann::json json;
                        json["dynamicRegistration"] = data::to_json(dynamicRegistration);
                        json["signatureInformation"] = data::to_json(signatureInformation);
                        json["contextSupport"] = data::to_json(contextSupport);
                        return json;
                    }
                };
                struct declaration_client_capabilities
                {
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

                    static declaration_client_capabilities from_json(const nlohmann::json& node)
                    {
                        declaration_client_capabilities res;
                        data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                        data::from_json(node, "linkSupport", res.linkSupport);
                        return res;
                    }
                    nlohmann::json to_json() const
                    {
                        nlohmann::json json;
                        json["dynamicRegistration"] = data::to_json(dynamicRegistration);
                        json["linkSupport"] = data::to_json(linkSupport);
                        return json;
                    }
                };
                struct definition_client_capabilities
                {
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

                    static definition_client_capabilities from_json(const nlohmann::json& node)
                    {
                        definition_client_capabilities res;
                        data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                        data::from_json(node, "linkSupport", res.linkSupport);
                        return res;
                    }
                    nlohmann::json to_json() const
                    {
                        nlohmann::json json;
                        json["dynamicRegistration"] = data::to_json(dynamicRegistration);
                        json["linkSupport"] = data::to_json(linkSupport);
                        return json;
                    }
                };
                struct type_definition_client_capabilities
                {
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

                    static type_definition_client_capabilities from_json(const nlohmann::json& node)
                    {
                        type_definition_client_capabilities res;
                        data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                        data::from_json(node, "linkSupport", res.linkSupport);
                        return res;
                    }
                    nlohmann::json to_json() const
                    {
                        nlohmann::json json;
                        json["dynamicRegistration"] = data::to_json(dynamicRegistration);
                        json["linkSupport"] = data::to_json(linkSupport);
                        return json;
                    }
                };
                struct implementation_client_capabilities
                {
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

                    static implementation_client_capabilities from_json(const nlohmann::json& node)
                    {
                        implementation_client_capabilities res;
                        data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                        data::from_json(node, "linkSupport", res.linkSupport);
                        return res;
                    }
                    nlohmann::json to_json() const
                    {
                        nlohmann::json json;
                        json["dynamicRegistration"] = data::to_json(dynamicRegistration);
                        json["linkSupport"] = data::to_json(linkSupport);
                        return json;
                    }
                };
                struct reference_client_capabilities
                {
                    /**
                     * Whether references supports dynamic registration.
                     */
                    std::optional<bool> dynamicRegistration;

                    static reference_client_capabilities from_json(const nlohmann::json& node)
                    {
                        reference_client_capabilities res;
                        data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                        return res;
                    }
                    nlohmann::json to_json() const
                    {
                        nlohmann::json json;
                        json["dynamicRegistration"] = data::to_json(dynamicRegistration);
                        return json;
                    }
                };
                struct document_highlight_client_capabilities
                {
                    /**
                     * Whether document highlight supports dynamic registration.
                     */
                    std::optional<bool> dynamicRegistration;

                    static document_highlight_client_capabilities from_json(const nlohmann::json& node)
                    {
                        document_highlight_client_capabilities res;
                        data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                        return res;
                    }
                    nlohmann::json to_json() const
                    {
                        nlohmann::json json;
                        json["dynamicRegistration"] = data::to_json(dynamicRegistration);
                        return json;
                    }
                };
                struct document_symbol_client_capabilities
                {
                    struct SymbolKind
                    {
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

                        static SymbolKind from_json(const nlohmann::json& node)
                        {
                            SymbolKind res;
                            data::from_json(node, "valueSet", res.valueSet);
                            return res;
                        }
                        nlohmann::json to_json() const
                        {
                            nlohmann::json json;
                            json["valueSet"] = data::to_json(valueSet);
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

                    static document_symbol_client_capabilities from_json(const nlohmann::json& node)
                    {
                        document_symbol_client_capabilities res;
                        data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                        data::from_json(node, "symbolKind", res.symbolKind);
                        data::from_json(node, "hierarchicalDocumentSymbolSupport", res.hierarchicalDocumentSymbolSupport);
                        return res;
                    }
                    nlohmann::json to_json() const
                    {
                        nlohmann::json json;
                        json["dynamicRegistration"] = data::to_json(dynamicRegistration);
                        json["symbolKind"] = data::to_json(symbolKind);
                        json["hierarchicalDocumentSymbolSupport"] = data::to_json(hierarchicalDocumentSymbolSupport);
                        return json;
                    }
                };
                struct code_action_client_capabilities
                {
                    struct CodeActionLiteralSupport
                    {
                        /**
                         * The code action kind is supported with the following value
                         * set.
                         */
                        struct
                        {
                            /**
                             * The code action kind values the client supports. When this
                             * property exists the client also guarantees that it will
                             * handle values outside its set gracefully and falls back
                             * to a default value when unknown.
                             */
                            std::vector<code_action_kind> valueSet;
                        } codeActionKind;

                        static CodeActionLiteralSupport from_json(const nlohmann::json& node)
                        {
                            CodeActionLiteralSupport res;
                            data::from_json(node["codeActionKind"], "valueSet", res.codeActionKind.valueSet);
                            return res;
                        }
                        nlohmann::json to_json() const
                        {
                            nlohmann::json json;
                            json["codeActionKind"] = { "valueSet", data::to_json(codeActionKind.valueSet) };
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
                };
                struct code_lens_client_capabilities
                {
                    /**
                     * Whether code lens supports dynamic registration.
                     */
                    std::optional<bool> dynamicRegistration;

                    static code_lens_client_capabilities from_json(const nlohmann::json& node)
                    {
                        code_lens_client_capabilities res;
                        data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                        return res;
                    }
                    nlohmann::json to_json() const
                    {
                        nlohmann::json json;
                        json["dynamicRegistration"] = data::to_json(dynamicRegistration);
                        return json;
                    }
                };
                struct document_link_client_capabilities
                {
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

                    static document_link_client_capabilities from_json(const nlohmann::json& node)
                    {
                        document_link_client_capabilities res;
                        data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                        data::from_json(node, "tooltipSupport", res.tooltipSupport);
                        return res;
                    }
                    nlohmann::json to_json() const
                    {
                        nlohmann::json json;
                        json["dynamicRegistration"] = data::to_json(dynamicRegistration);
                        json["tooltipSupport"] = data::to_json(tooltipSupport);
                        return json;
                    }
                };
                struct document_color_client_capabilities
                {
                    /**
                     * Whether document color supports dynamic registration.
                     */
                    std::optional<bool> dynamicRegistration;

                    static document_color_client_capabilities from_json(const nlohmann::json& node)
                    {
                        document_color_client_capabilities res;
                        data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                        return res;
                    }
                    nlohmann::json to_json() const
                    {
                        nlohmann::json json;
                        json["dynamicRegistration"] = data::to_json(dynamicRegistration);
                        return json;
                    }
                };
                struct document_formatting_client_capabilities
                {
                    /**
                     * Whether formatting supports dynamic registration.
                     */
                    std::optional<bool> dynamicRegistration;

                    static document_formatting_client_capabilities from_json(const nlohmann::json& node)
                    {
                        document_formatting_client_capabilities res;
                        data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                        return res;
                    }
                    nlohmann::json to_json() const
                    {
                        nlohmann::json json;
                        json["dynamicRegistration"] = data::to_json(dynamicRegistration);
                        return json;
                    }
                };
                struct document_range_formatting_client_capabilities
                {
                    /**
                     * Whether formatting supports dynamic registration.
                     */
                    std::optional<bool> dynamicRegistration;

                    static document_range_formatting_client_capabilities from_json(const nlohmann::json& node)
                    {
                        document_range_formatting_client_capabilities res;
                        data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                        return res;
                    }
                    nlohmann::json to_json() const
                    {
                        nlohmann::json json;
                        json["dynamicRegistration"] = data::to_json(dynamicRegistration);
                        return json;
                    }
                };
                struct document_on_type_formatting_client_capabilities
                {
                    /**
                     * Whether on type formatting supports dynamic registration.
                     */
                    std::optional<bool> dynamicRegistration;

                    static document_on_type_formatting_client_capabilities from_json(const nlohmann::json& node)
                    {
                        document_on_type_formatting_client_capabilities res;
                        data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                        return res;
                    }
                    nlohmann::json to_json() const
                    {
                        nlohmann::json json;
                        json["dynamicRegistration"] = data::to_json(dynamicRegistration);
                        return json;
                    }
                };
                struct rename_client_capabilities
                {
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

                    static rename_client_capabilities from_json(const nlohmann::json& node)
                    {
                        rename_client_capabilities res;
                        data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                        data::from_json(node, "prepareSupport", res.prepareSupport);
                        return res;
                    }
                    nlohmann::json to_json() const
                    {
                        nlohmann::json json;
                        json["dynamicRegistration"] = data::to_json(dynamicRegistration);
                        json["prepareSupport"] = data::to_json(prepareSupport);
                        return json;
                    }
                };
                struct publish_diagnostics_client_capabilities
                {
                    struct TagSupport
                    {
                        /**
                         * The tags supported by the client.
                         */
                        std::optional<std::vector<diagnostic_tag>> valueSet;

                        static TagSupport from_json(const nlohmann::json& node)
                        {
                            TagSupport res;
                            data::from_json(node, "valueSet", res.valueSet);
                            return res;
                        }
                        nlohmann::json to_json() const
                        {
                            nlohmann::json json;
                            json["valueSet"] = data::to_json(valueSet);
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

                    static publish_diagnostics_client_capabilities from_json(const nlohmann::json& node)
                    {
                        publish_diagnostics_client_capabilities res;
                        data::from_json(node, "relatedInformation", res.relatedInformation);
                        data::from_json(node, "tagSupport", res.tagSupport);
                        data::from_json(node, "versionSupport", res.versionSupport);
                        return res;
                    }
                    nlohmann::json to_json() const
                    {
                        nlohmann::json json;
                        json["relatedInformation"] = data::to_json(relatedInformation);
                        json["tagSupport"] = data::to_json(tagSupport);
                        json["versionSupport"] = data::to_json(versionSupport);
                        return json;
                    }
                };
                struct folding_range_client_capabilities
                {
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

                    static folding_range_client_capabilities from_json(const nlohmann::json& node)
                    {
                        folding_range_client_capabilities res;
                        data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                        data::from_json(node, "rangeLimit", res.rangeLimit);
                        data::from_json(node, "lineFoldingOnly", res.lineFoldingOnly);
                        return res;
                    }
                    nlohmann::json to_json() const
                    {
                        nlohmann::json json;
                        json["dynamicRegistration"] = data::to_json(dynamicRegistration);
                        json["rangeLimit"] = data::to_json(rangeLimit);
                        json["lineFoldingOnly"] = data::to_json(lineFoldingOnly);
                        return json;
                    }
                };
                struct selection_range_client_capabilities
                {
                    /**
                     * Whether implementation supports dynamic registration for selection range providers. If this is set to `true`
                     * the client supports the new `SelectionRangeRegistrationOptions` return value for the corresponding server
                     * capability as well.
                     */
                    std::optional<bool> dynamicRegistration;

                    static selection_range_client_capabilities from_json(const nlohmann::json& node)
                    {
                        selection_range_client_capabilities res;
                        data::from_json(node, "dynamicRegistration", res.dynamicRegistration);
                        return res;
                    }
                    nlohmann::json to_json() const
                    {
                        nlohmann::json json;
                        json["dynamicRegistration"] = data::to_json(dynamicRegistration);
                        return json;
                    }
                };
                /*
                   Text document specific client capabilities.
                */
                struct text_document_client_capabilities
                {
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

                    static text_document_client_capabilities from_json(const nlohmann::json& node)
                    {
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
                    nlohmann::json to_json() const
                    {
                        nlohmann::json json;
                        json["synchronization"] = data::to_json(synchronization);
                        json["completion"] = data::to_json(completion);
                        json["hover"] = data::to_json(hover);
                        json["signatureHelp"] = data::to_json(signatureHelp);
                        json["declaration"] = data::to_json(declaration);
                        json["definition"] = data::to_json(definition);
                        json["typeDefinition"] = data::to_json(typeDefinition);
                        json["implementation"] = data::to_json(implementation);
                        json["references"] = data::to_json(references);
                        json["documentHighlight"] = data::to_json(documentHighlight);
                        json["documentSymbol"] = data::to_json(documentSymbol);
                        json["codeAction"] = data::to_json(codeAction);
                        json["codeLens"] = data::to_json(codeLens);
                        json["documentLink"] = data::to_json(documentLink);
                        json["colorProvider"] = data::to_json(colorProvider);
                        json["formatting"] = data::to_json(formatting);
                        json["rangeFormatting"] = data::to_json(rangeFormatting);
                        json["onTypeFormatting"] = data::to_json(onTypeFormatting);
                        json["rename"] = data::to_json(rename);
                        json["publishDiagnostics"] = data::to_json(publishDiagnostics);
                        json["foldingRange"] = data::to_json(foldingRange);
                        json["selectionRange"] = data::to_json(selectionRange);
                        return json;
                    }
                };
                struct client_capabilities
                {
                    struct Workspace
                    {
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

                        static Workspace from_json(const nlohmann::json& node)
                        {
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
                        nlohmann::json to_json() const
                        {
                            nlohmann::json json;
                            json["applyEdit"] = data::to_json(applyEdit);
                            json["workspaceEdit"] = data::to_json(workspaceEdit);
                            json["didChangeConfiguration"] = data::to_json(didChangeConfiguration);
                            json["didChangeWatchedFiles"] = data::to_json(didChangeWatchedFiles);
                            json["symbol"] = data::to_json(symbol);
                            json["executeCommand"] = data::to_json(executeCommand);
                            json["workspaceFolders"] = data::to_json(workspaceFolders);
                            json["configuration"] = data::to_json(configuration);
                            return json;
                        }
                    };
                    struct Window
                    {
                        /*
                           Whether client supports handling progress notifications. If set servers are allowed to
                           report in `workDoneProgress` property in the request specific server capabilities.

                           Since 3.15.0
                        */
                        std::optional<bool> workDoneProgress;

                        static Window from_json(const nlohmann::json& node)
                        {
                            Window res;
                            data::from_json(node, "workDoneProgress", res.workDoneProgress);
                            return res;
                        }
                        nlohmann::json to_json() const
                        {
                            nlohmann::json json;
                            json["workDoneProgress"] = data::to_json(workDoneProgress);
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

                    static client_capabilities from_json(const nlohmann::json& node)
                    {
                        client_capabilities res;
                        data::from_json(node, "workspace", res.workspace);
                        data::from_json(node, "textDocument", res.textDocument);
                        data::from_json(node, "experimental", res.experimental);
                        return res;
                    }
                    nlohmann::json to_json() const
                    {
                        nlohmann::json json;
                        json["workspace"] = data::to_json(workspace);
                        json["textDocument"] = data::to_json(textDocument);
                        json["experimental"] = data::to_json(experimental);
                        return json;
                    }
                };
                struct workspace_folder
                {
                    /*
                       The associated URI for this workspace folder.
                    */
                    std::string uri;
                    /*
                       The name of the workspace folder. Used to refer to this
                       workspace folder in the user interface.
                    */
                    std::string name;

                    static workspace_folder from_json(const nlohmann::json& node)
                    {
                        workspace_folder res;
                        data::from_json(node, "uri", res.uri);
                        data::from_json(node, "name", res.name);
                        return res;
                    }
                    nlohmann::json to_json() const
                    {
                        nlohmann::json json;
                        json["uri"] = data::to_json(uri);
                        json["name"] = data::to_json(name);
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
                std::optional<std::string> rootUri;
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
                std::optional<std::vector<workspace_folder>> workspaceFolders;

                static initialize_params from_json(const nlohmann::json& node)
                {
                    initialize_params res;
                    data::from_json(node, "processId", res.processId);
                    data::from_json(node, "clientInfo", res.clientInfo);
                    data::from_json(node, "rootPath", res.rootPath);
                    data::from_json(node, "rootUri", res.rootUri);
                    data::from_json(node, "initializationOptions", res.initializationOptions);
                    data::from_json(node, "capabilities", res.capabilities);
                    data::from_json(node, "trace", res.trace);
                    data::from_json(node, "workspaceFolders", res.workspaceFolders);
                    return res;
                }
                nlohmann::json to_json() const
                {
                    nlohmann::json json;
                    json["processId"] = data::to_json(processId);
                    json["clientInfo"] = data::to_json(clientInfo);
                    json["rootPath"] = data::to_json(rootPath);
                    json["rootUri"] = data::to_json(rootUri);
                    json["initializationOptions"] = data::to_json(initializationOptions);
                    json["capabilities"] = data::to_json(capabilities);
                    json["trace"] = data::to_json(trace);
                    json["workspaceFolders"] = data::to_json(workspaceFolders);
                    return json;
                }
            };
        }
    }
    class server
    {

    private:
        void initialize(jsonrpc& rpc, const jsonrpc::rpcmessage& msg)
        {
            rpc.send({ msg.id, {
                {
                    "capabilities", {
                        "textDocumentSync"
                    }
                }
                } });
        }
        bool m_die;
    public:
        jsonrpc rpc;
        server() : rpc(std::cin, std::cout, jsonrpc::detach, jsonrpc::skip), m_die(false)
        {
            rpc.register_method("initialize", [&](jsonrpc& rpc, const jsonrpc::rpcmessage& msg) { initialize(rpc, msg); });
        }

        void listen()
        {
            while (!m_die)
            {
                if (!rpc.handle_single_message())
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    continue;
                }
            }
        }

        void kill() { m_die = true; }

        virtual data::responses::initialize_result on_initialize(const data::requests::initialize_params& params) = 0;
    };
}