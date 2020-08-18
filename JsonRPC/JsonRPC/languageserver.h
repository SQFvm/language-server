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
        enum class resource_operations
        {
            ro_empty = 0b000,
            /*
               Supports creating new files and folders.
            */
            ro_create = 0b001,
            /*
               Supports renaming existing files and folders.
            */
            ro_rename = 0b010,
            /*
               Supports deleting existing files and folders.
            */
            ro_delete = 0b100
        };
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
        enum class trace_mode
        {
            off,
            message,
            verbose
        };
        enum class completion_item_tag
        {
            Deprecated = 1
        };
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
        enum class initialize_error
        {
            unknownProtocolVersion = 1
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
                };
                struct did_change_configuration_client_capabilities
                {
                    /*
                       Did change configuration notification supports dynamic registration.
                    */
                    std::optional<bool> dynamicRegistration;
                };
                struct did_change_watched_files_client_capabilities
                {
                    /*
                       Did change watched files notification supports dynamic registration. Please note
                       that the current protocol doesn't support static configuration for file changes
                       from the server side.
                    */
                    std::optional<bool> dynamicRegistration;
                };
                struct workspace_symbol_client_capabilities
                {
                    struct SymbolKind
                    {
                        std::optional<std::vector<symbol_kind>> valueSet;
                    };
                    /*
                       Symbol request supports dynamic registration.
                    */
                    std::optional<bool> dynamicRegistration;
                    /*
                       Specific capabilities for the `SymbolKind` in the `workspace/symbol` request.
                    */
                    std::optional<SymbolKind> symbolKind;
                };
                struct execute_command_client_capabilities
                {
                    /*
                       Execute command supports dynamic registration.
                    */
                    std::optional<bool> dynamicRegistration;
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
                        std::optional<bool>deprecatedSupport;

                        /**
                         * Client supports the preselect property on a completion item.
                         */
                        std::optional<bool>preselectSupport;

                        /**
                         * Client supports the tag property on a completion item. Clients supporting
                         * tags have to handle unknown tags gracefully. Clients especially need to
                         * preserve unknown tags when sending a completion item back to the server in
                         * a resolve call.
                         *
                         * @since 3.15.0
                         */
                        std::optional<TagSupport> tagSupport;
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
                };
                struct reference_client_capabilities
                {
                    /**
                     * Whether references supports dynamic registration.
                     */
                    std::optional<bool> dynamicRegistration;
                };
                struct document_highlight_client_capabilities
                {
                    /**
                     * Whether document highlight supports dynamic registration.
                     */
                    std::optional<bool> dynamicRegistration;
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
                };
                struct document_color_client_capabilities
                {
                    /**
                     * Whether document color supports dynamic registration.
                     */
                    std::optional<bool> dynamicRegistration;
                };
                struct document_formatting_client_capabilities
                {
                    /**
                     * Whether formatting supports dynamic registration.
                     */
                    std::optional<bool> dynamicRegistration;
                };
                struct document_range_formatting_client_capabilities
                {
                    /**
                     * Whether formatting supports dynamic registration.
                     */
                    std::optional<bool> dynamicRegistration;
                };
                struct document_on_type_formatting_client_capabilities
                {
                    /**
                     * Whether on type formatting supports dynamic registration.
                     */
                    std::optional<bool> dynamicRegistration;
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
                };
                struct publish_diagnostics_client_capabilities
                {
                    struct TagSupport
                    {
                        /**
                         * The tags supported by the client.
                         */
                        std::optional<std::vector<diagnostic_tag>> valueSet;
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
                };
                struct selection_range_client_capabilities
                {
                    /**
                     * Whether implementation supports dynamic registration for selection range providers. If this is set to `true`
                     * the client supports the new `SelectionRangeRegistrationOptions` return value for the corresponding server
                     * capability as well.
                     */
                    std::optional<bool> dynamicRegistration;
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
                    };
                    struct Window
                    {
                        /*
                           Whether client supports handling progress notifications. If set servers are allowed to
                           report in `workDoneProgress` property in the request specific server capabilities.

                           Since 3.15.0
                        */
                        std::optional<bool> workDoneProgress;
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
                std::optional<std::string> rootUri;
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