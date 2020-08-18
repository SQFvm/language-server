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
        namespace client
        { // See https://microsoft.github.io/language-server-protocol/specifications/specification-current/#initialize
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

            };
            struct completion_client_capabilities
            {

            };
            struct hover_client_capabilities
            {

            };
            struct signature_help_client_capabilities
            {

            };
            struct declaration_client_capabilities
            {

            };
            struct definition_client_capabilities
            {

            };
            struct type_definition_client_capabilities
            {

            };
            struct implementation_client_capabilities
            {

            };
            struct reference_client_capabilities
            {

            };
            struct document_highlight_client_capabilities
            {

            };
            struct document_symbol_client_capabilities
            {

            };
            struct code_action_client_capabilities
            {

            };
            struct code_lens_client_capabilities
            {

            };
            struct document_link_client_capabilities
            {

            };
            struct document_color_client_capabilities
            {

            };
            struct document_formatting_client_capabilities
            {

            };
            struct document_range_formatting_client_capabilities
            {

            };
            struct document_on_type_formatting_client_capabilities
            {

            };
            struct rename_client_capabilities
            {

            };
            struct publish_diagnostics_client_capabilities
            {

            };
            struct folding_range_client_capabilities
            {

            };
            struct selection_range_client_capabilities
            {

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
            struct initialize_params
            {
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
    public:
        jsonrpc rpc;
        server() : rpc(std::cin, std::cout, jsonrpc::detach, jsonrpc::skip)
        {
            rpc.register_method("initialize", [&](jsonrpc& rpc, const jsonrpc::rpcmessage& msg) { initialize(rpc, msg); });
        }

        void listen()
        {
            while (true)
            {
                if (!rpc.handle_single_message())
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    continue;
                }
            }
        }
    };
}