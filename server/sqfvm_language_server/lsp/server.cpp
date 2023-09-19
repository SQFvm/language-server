//
// Created by marco.silipo on 20.08.2023.
//

#include "server.hpp"


void lsp::server::register_methods() {
    m_rpc.register_method(
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
                    window_logMessage(data::message_type::Log, sstream.str());
                }
            });
    m_rpc.register_method(
            "shutdown", [&](jsonrpc &rpc, const jsonrpc::rpcmessage &msg) {
                kill();
                on_shutdown();
            });
    m_rpc.register_method(
            "textDocument/didOpen", [&](jsonrpc &rpc, const jsonrpc::rpcmessage &msg) {
                try {
                    auto params = data::did_open_text_document_params::from_json(msg.params.value());
                    on_textDocument_didOpen(params);
                }
                catch (const std::exception &e) {
                    std::stringstream sstream;
                    sstream << "rpc call 'textDocument/didOpen' failed with: '" << e.what() << "'.";
                    window_logMessage(data::message_type::Log, sstream.str());
                }
            });
    m_rpc.register_method(
            "textDocument/didChange", [&](jsonrpc &rpc, const jsonrpc::rpcmessage &msg) {
                try {
                    auto params = data::did_change_text_document_params::from_json(msg.params.value());
                    on_textDocument_didChange(params);
                }
                catch (const std::exception &e) {
                    std::stringstream sstream;
                    sstream << "rpc call 'textDocument/didChange' failed with: '" << e.what() << "'.";
                    window_logMessage(data::message_type::Log, sstream.str());
                }
            });
    m_rpc.register_method(
            "textDocument/willSave", [&](jsonrpc &rpc, const jsonrpc::rpcmessage &msg) {
                try {
                    auto params = data::will_save_text_document_params::from_json(msg.params.value());
                    on_textDocument_willSave(params);
                }
                catch (const std::exception &e) {
                    std::stringstream sstream;
                    sstream << "rpc call 'textDocument/willSave' failed with: '" << e.what() << "'.";
                    window_logMessage(data::message_type::Log, sstream.str());
                }
            });
    m_rpc.register_method(
            "textDocument/willSaveWaitUntil", [&](jsonrpc &rpc, const jsonrpc::rpcmessage &msg) {
                try {
                    auto params = data::will_save_text_document_params::from_json(msg.params.value());
                    auto res = on_textDocument_willSaveWaitUntil(params);
                    rpc.send({msg.id, res.has_value() ? res->to_json() : nlohmann::json(nullptr)});
                }
                catch (const std::exception &e) {
                    std::stringstream sstream;
                    sstream << "rpc call 'textDocument/willSaveWaitUntil' failed with: '" << e.what() << "'.";
                    window_logMessage(data::message_type::Log, sstream.str());
                }
            });
    m_rpc.register_method(
            "textDocument/didSave", [&](jsonrpc &rpc, const jsonrpc::rpcmessage &msg) {
                try {
                    auto params = data::did_save_text_document_params::from_json(msg.params.value());
                    on_textDocument_didSave(params);
                }
                catch (const std::exception &e) {
                    std::stringstream sstream;
                    sstream << "rpc call 'textDocument/didSave' failed with: '" << e.what() << "'.";
                    window_logMessage(data::message_type::Log, sstream.str());
                }
            });
    m_rpc.register_method(
            "textDocument/didClose", [&](jsonrpc &rpc, const jsonrpc::rpcmessage &msg) {
                try {
                    auto params = data::did_close_text_document_params::from_json(msg.params.value());
                    on_textDocument_didClose(params);
                }
                catch (const std::exception &e) {
                    std::stringstream sstream;
                    sstream << "rpc call 'textDocument/didClose' failed with: '" << e.what() << "'.";
                    window_logMessage(data::message_type::Log, sstream.str());
                }
            });
    m_rpc.register_method(
            "textDocument/completion", [&](jsonrpc &rpc, const jsonrpc::rpcmessage &msg) {
                try {
                    auto params = data::completion_params::from_json(msg.params.value());
                    auto res = on_textDocument_completion(params);
                    rpc.send({msg.id, res.has_value() ? res->to_json() : nlohmann::json(nullptr)});
                }
                catch (const std::exception &e) {
                    std::stringstream sstream;
                    sstream << "rpc call 'textDocument/completion' failed with: '" << e.what() << "'.";
                    window_logMessage(data::message_type::Log, sstream.str());
                }
            });
    m_rpc.register_method(
            "textDocument/foldingRange", [&](jsonrpc &rpc, const jsonrpc::rpcmessage &msg) {
                try {
                    auto params = data::folding_range_params::from_json(msg.params.value());
                    auto res = on_textDocument_foldingRange(params);
                    rpc.send({msg.id, res.has_value() ? to_json(*res) : nlohmann::json(nullptr)});
                }
                catch (const std::exception &e) {
                    std::stringstream sstream;
                    sstream << "rpc call 'textDocument/foldingRange' failed with: '" << e.what() << "'.";
                    window_logMessage(data::message_type::Log, sstream.str());
                }
            });
    m_rpc.register_method(
            "textDocument/documentColor", [&](jsonrpc &rpc, const jsonrpc::rpcmessage &msg) {
                try {
                    auto params = data::document_color_params::from_json(msg.params.value());
                    auto res = on_textDocument_documentColor(params);
                    rpc.send({msg.id, to_json(res)});
                }
                catch (const std::exception &e) {
                    std::stringstream sstream;
                    sstream << "rpc call 'textDocument/documentColor' failed with: '" << e.what() << "'.";
                    window_logMessage(data::message_type::Log, sstream.str());
                }
            });
    m_rpc.register_method(
            "textDocument/references", [&](jsonrpc &rpc, const jsonrpc::rpcmessage &msg) {
                try {
                    auto params = data::references_params::from_json(msg.params.value());
                    auto res = on_textDocument_references(params);
                    rpc.send({msg.id, to_json(res)});
                }
                catch (const std::exception &e) {
                    std::stringstream sstream;
                    sstream << "rpc call 'textDocument/references' failed with: '" << e.what() << "'.";
                    window_logMessage(data::message_type::Log, sstream.str());
                }
            });
    m_rpc.register_method(
            "textDocument/colorPresentation", [&](jsonrpc &rpc, const jsonrpc::rpcmessage &msg) {
                try {
                    auto params = data::color_presentation_params::from_json(msg.params.value());
                    auto res = on_textDocument_colorPresentation(params);
                    rpc.send({msg.id, to_json(res)});
                }
                catch (const std::exception &e) {
                    std::stringstream sstream;
                    sstream << "rpc call 'textDocument/colorPresentation' failed with: '" << e.what() << "'.";
                    window_logMessage(data::message_type::Log, sstream.str());
                }
            });
    m_rpc.register_method(
            "textDocument/codeAction", [&](jsonrpc &rpc, const jsonrpc::rpcmessage &msg) {
                try {
                    auto params = data::code_action_params::from_json(msg.params.value());
                    auto res = on_textDocument_codeAction(params);
                    rpc.send({msg.id, to_json(res)});
                }
                catch (const std::exception &e) {
                    std::stringstream sstream;
                    sstream << "rpc call 'textDocument/codeAction' failed with: '" << e.what() << "'.";
                    window_logMessage(data::message_type::Log, sstream.str());
                }
            });
    m_rpc.register_method(
            "textDocument/hover", [&](jsonrpc &rpc, const jsonrpc::rpcmessage &msg) {
                try {
                    auto params = data::hover_params::from_json(msg.params.value());
                    auto res = on_textDocument_hover(params);
                    rpc.send({msg.id, to_json(res)});
                }
                catch (const std::exception &e) {
                    std::stringstream sstream;
                    sstream << "rpc call 'textDocument/hover' failed with: '" << e.what() << "'.";
                    window_logMessage(data::message_type::Log, sstream.str());
                }
            });
    m_rpc.register_method(
            "textDocument/inlayHint", [&](jsonrpc &rpc, const jsonrpc::rpcmessage &msg) {
                try {
                    auto params = data::inlay_hint_params::from_json(msg.params.value());
                    auto res = on_textDocument_inlayHint(params);
                    rpc.send({msg.id, to_json(res)});
                }
                catch (const std::exception &e) {
                    std::stringstream sstream;
                    sstream << "rpc call 'textDocument/inlayHint' failed with: '" << e.what() << "'.";
                    window_logMessage(data::message_type::Log, sstream.str());
                }
            });
    m_rpc.register_method(
            "workspace/didChangeConfiguration", [&](jsonrpc &rpc, const jsonrpc::rpcmessage &msg) {
                try {
                    auto params = data::did_change_configuration_params::from_json(msg.params.value());
                    on_workspace_didChangeConfiguration(params);
                }
                catch (const std::exception &e) {
                    std::stringstream sstream;
                    sstream << "rpc call 'workspace/didChangeConfiguration' failed with: '" << e.what() << "'.";
                    window_logMessage(data::message_type::Log, sstream.str());
                }
            });
}

void lsp::server::listen() {
    while (!m_die) {
        if (!m_rpc.handle_single_message()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
    }
}

void lsp::server::kill() {
    m_die = true;
}