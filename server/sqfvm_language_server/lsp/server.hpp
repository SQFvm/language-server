//
// Created by marco.silipo on 20.08.2023.
//

#ifndef SQFVM_LANGUAGE_SERVER_SERVER_HPP
#define SQFVM_LANGUAGE_SERVER_SERVER_HPP

//
#include "lspserver.hpp"
//


#include "jsonrpc.hpp"
#include "../uri.hpp"

#include <optional>
#include <string>
#include <nlohmann/json.hpp>
#include <vector>
#include <variant>

namespace lsp {
    class server {

        bool m_die;
        void register_methods();
    public:
        jsonrpc m_rpc;

        server() : m_rpc(std::cin, std::cout, jsonrpc::detach, jsonrpc::skip), m_die(false) {
            register_methods();
        }
        server(jsonrpc&& rpc) : m_rpc(std::move(rpc)), m_die(false) {
            register_methods();
        }

        void listen();

        void kill();

        // Methods that must be overriden by clients
    protected:
        virtual lsp::data::initialize_result on_initialize(const lsp::data::initialize_params &params) = 0;

        virtual void after_initialize(const lsp::data::initialize_params &params) { /* empty */ }

        virtual void on_shutdown() = 0;


        // Methods that can be overriden by implementing clients
    protected:
        virtual void on_textDocument_didOpen(const lsp::data::did_open_text_document_params &params) { /* empty */ }

        virtual void on_textDocument_didChange(const lsp::data::did_change_text_document_params &params) { /* empty */ }

        virtual void on_textDocument_willSave(const lsp::data::will_save_text_document_params &params) { /* empty */ }

        virtual std::optional<lsp::data::text_edit> on_textDocument_willSaveWaitUntil(
                const lsp::data::will_save_text_document_params &params) {
            return {};
        }

        virtual void on_textDocument_didSave(
                const lsp::data::did_save_text_document_params &params)  { /* empty */ }

        virtual void on_textDocument_didClose(
                const lsp::data::did_close_text_document_params &params)  { /* empty */ }

        virtual std::optional<lsp::data::completion_list> on_textDocument_completion(
                const lsp::data::completion_params &params) {
            return {};
        }

        virtual std::optional<lsp::data::hover> on_textDocument_hover(
                const lsp::data::hover_params &params) {
            return {};
        }

        virtual std::optional<std::vector<lsp::data::inlay_hint>>
        on_textDocument_inlayHint(const lsp::data::inlay_hint_params &params) {
            return {};
        }

        virtual std::optional<std::vector<lsp::data::folding_range>> on_textDocument_foldingRange(
                const lsp::data::folding_range_params &params) {
            return {};
        }

        virtual std::vector<lsp::data::color_information> on_textDocument_documentColor(
                const lsp::data::document_color_params &params) {
            return {};
        }

        virtual std::vector<lsp::data::color_presentation> on_textDocument_colorPresentation(
                const lsp::data::color_presentation_params &params) {
            return {};
        }

        virtual std::optional<std::vector<std::variant<lsp::data::command, lsp::data::code_action>>>

        on_textDocument_codeAction(
                const lsp::data::code_action_params &params) {
            return {};
        }

        virtual std::optional<std::vector<lsp::data::location>> on_textDocument_references(
                const lsp::data::references_params &params) {
            return {};
        }

        virtual void on_workspace_didChangeConfiguration(
                const lsp::data::did_change_configuration_params &params) {
        }

    public:
        void textDocument_publishDiagnostics(const lsp::data::publish_diagnostics_params &params) {
            m_rpc.send({{}, "textDocument/publishDiagnostics", params.to_json()});
        }

        void window_logMessage(lsp::data::message_type type, std::string message) {
            m_rpc.send({{}, "window/logMessage", lsp::data::log_message_params{type, message}.to_json()});
        }

        void window_logMessage(const lsp::data::log_message_params &params) {
            m_rpc.send({{}, "window/logMessage", params.to_json()});
        }

    };
}

#endif //SQFVM_LANGUAGE_SERVER_SERVER_HPP
