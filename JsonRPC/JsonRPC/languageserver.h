#pragma once
#include "jsonrpc.h"

#include <optional>
#include <string>
#include <nlohmann/json.hpp>
#include <vector>

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
        namespace responses
        {
            struct server_capabilities
            {
                /**
                 * Defines how text documents are synced. Is either a detailed structure defining each notification or
                 * for backwards compatibility the TextDocumentSyncKind number. If omitted it defaults to `TextDocumentSyncKind.None`.
                 */
                textDocumentSync ? : TextDocumentSyncOptions | number;

                /**
                 * The server provides completion support.
                 */
                completionProvider ? : CompletionOptions;

                /**
                 * The server provides hover support.
                 */
                hoverProvider ? : boolean | HoverOptions;

                /**
                 * The server provides signature help support.
                 */
                signatureHelpProvider ? : SignatureHelpOptions;

                /**
                 * The server provides go to declaration support.
                 *
                 * @since 3.14.0
                 */
                declarationProvider ? : boolean | DeclarationOptions | DeclarationRegistrationOptions;

                /**
                 * The server provides goto definition support.
                 */
                definitionProvider ? : boolean | DefinitionOptions;

                /**
                 * The server provides goto type definition support.
                 *
                 * @since 3.6.0
                 */
                typeDefinitionProvider ? : boolean | TypeDefinitionOptions | TypeDefinitionRegistrationOptions;

                /**
                 * The server provides goto implementation support.
                 *
                 * @since 3.6.0
                 */
                implementationProvider ? : boolean | ImplementationOptions | ImplementationRegistrationOptions;

                /**
                 * The server provides find references support.
                 */
                referencesProvider ? : boolean | ReferenceOptions;

                /**
                 * The server provides document highlight support.
                 */
                documentHighlightProvider ? : boolean | DocumentHighlightOptions;

                /**
                 * The server provides document symbol support.
                 */
                documentSymbolProvider ? : boolean | DocumentSymbolOptions;

                /**
                 * The server provides code actions. The `CodeActionOptions` return type is only
                 * valid if the client signals code action literal support via the property
                 * `textDocument.codeAction.codeActionLiteralSupport`.
                 */
                codeActionProvider ? : boolean | CodeActionOptions;

                /**
                 * The server provides code lens.
                 */
                codeLensProvider ? : CodeLensOptions;

                /**
                 * The server provides document link support.
                 */
                documentLinkProvider ? : DocumentLinkOptions;

                /**
                 * The server provides color provider support.
                 *
                 * @since 3.6.0
                 */
                colorProvider ? : boolean | DocumentColorOptions | DocumentColorRegistrationOptions;

                /**
                 * The server provides document formatting.
                 */
                documentFormattingProvider ? : boolean | DocumentFormattingOptions;

                /**
                 * The server provides document range formatting.
                 */
                documentRangeFormattingProvider ? : boolean | DocumentRangeFormattingOptions;

                /**
                 * The server provides document formatting on typing.
                 */
                documentOnTypeFormattingProvider ? : DocumentOnTypeFormattingOptions;

                /**
                 * The server provides rename support. RenameOptions may only be
                 * specified if the client states that it supports
                 * `prepareSupport` in its initial `initialize` request.
                 */
                renameProvider ? : boolean | RenameOptions;

                /**
                 * The server provides folding provider support.
                 *
                 * @since 3.10.0
                 */
                foldingRangeProvider ? : boolean | FoldingRangeOptions | FoldingRangeRegistrationOptions;

                /**
                 * The server provides execute command support.
                 */
                executeCommandProvider ? : ExecuteCommandOptions;

                /**
                 * The server provides selection range support.
                 *
                 * @since 3.15.0
                 */
                selectionRangeProvider ? : boolean | SelectionRangeOptions | SelectionRangeRegistrationOptions;

                /**
                 * The server provides workspace symbol support.
                 */
                workspaceSymbolProvider ? : boolean;

                /**
                 * Workspace specific server capabilities
                 */
                workspace ? : {
                    /**
                     * The server supports workspace folder.
                     *
                     * @since 3.6.0
                     */
                    workspaceFolders ? : WorkspaceFoldersServerCapabilities;
                }

                /**
                 * Experimental server capabilities.
                 */
                experimental ? : any;
            };
            struct server_info
            {
                std::string name;
                std::optional<std::string> version;
            };
            struct initialize_result
            {
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