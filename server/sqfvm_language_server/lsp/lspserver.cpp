#include "lspserver.hpp"

nlohmann::json lsp::data::log_message_params::to_json() const {
    nlohmann::json json;
    data::set_json(json, "type", type);
    data::set_json(json, "message", message);
    return json;
}

lsp::data::log_message_params lsp::data::log_message_params::from_json(const nlohmann::json &node) {
    log_message_params res;
    data::from_json(node, "type", res.type);
    data::from_json(node, "message", res.message);
    return res;
}

nlohmann::json lsp::data::color_presentation_params::to_json() const {
    nlohmann::json json;
    data::set_json(json, "partialResultToken", partialResultToken);
    data::set_json(json, "workDoneToken", workDoneToken);
    data::set_json(json, "textDocument", textDocument);
    data::set_json(json, "color", color);
    data::set_json(json, "range", range);
    return json;
}

lsp::data::color_presentation_params lsp::data::color_presentation_params::from_json(const nlohmann::json &node) {
    color_presentation_params res;
    data::from_json(node, "partialResultToken", res.partialResultToken);
    data::from_json(node, "workDoneToken", res.workDoneToken);
    data::from_json(node, "textDocument", res.textDocument);
    data::from_json(node, "color", res.color);
    data::from_json(node, "range", res.range);
    return res;
}

lsp::data::color_presentation lsp::data::color_presentation::from_json(const nlohmann::json &node) {
    color_presentation res;
    data::from_json(node, "label", res.label);
    data::from_json(node, "textEdit", res.textEdit);
    data::from_json(node, "additionalTextEdits", res.additionalTextEdits);
    return res;
}

nlohmann::json lsp::data::color_presentation::to_json() const {
    nlohmann::json json;
    data::set_json(json, "label", label);
    data::set_json(json, "textEdit", textEdit);
    data::set_json(json, "additionalTextEdits", additionalTextEdits);
    return json;
}

lsp::data::document_color_params lsp::data::document_color_params::from_json(const nlohmann::json &node) {
    document_color_params res;
    data::from_json(node, "partialResultToken", res.partialResultToken);
    data::from_json(node, "workDoneToken", res.workDoneToken);
    data::from_json(node, "textDocument", res.textDocument);
    return res;
}

nlohmann::json lsp::data::document_color_params::to_json() const {
    nlohmann::json json;
    data::set_json(json, "partialResultToken", partialResultToken);
    data::set_json(json, "workDoneToken", workDoneToken);
    data::set_json(json, "textDocument", textDocument);
    return json;
}

lsp::data::did_change_configuration_params
lsp::data::did_change_configuration_params::from_json(const nlohmann::json &node) {
    did_change_configuration_params res;
    res.settings = node.contains("settings") ? node["settings"] : nlohmann::json(nullptr);
    return res;
}

nlohmann::json lsp::data::did_change_configuration_params::to_json() const {
    nlohmann::json json;
    if (settings.has_value()) {
        json["settings"] = *settings;
    }
    return json;
}

lsp::data::color_information lsp::data::color_information::from_json(const nlohmann::json &node) {
    color_information res{};
    data::from_json(node, "range", res.range);
    data::from_json(node, "color", res.color);
    return res;
}

nlohmann::json lsp::data::color_information::to_json() const {
    nlohmann::json json;
    data::set_json(json, "range", range);
    data::set_json(json, "color", color);
    return json;
}

lsp::data::color lsp::data::color::from_json(const nlohmann::json &node) {
    color res{};
    data::from_json(node, "red", res.red);
    data::from_json(node, "green", res.green);
    data::from_json(node, "blue", res.blue);
    data::from_json(node, "alpha", res.alpha);
    return res;
}

nlohmann::json lsp::data::color::to_json() const {
    nlohmann::json json;
    data::set_json(json, "red", red);
    data::set_json(json, "green", green);
    data::set_json(json, "blue", blue);
    data::set_json(json, "alpha", alpha);
    return json;
}

lsp::data::folding_range_params lsp::data::folding_range_params::from_json(const nlohmann::json &node) {
    folding_range_params res;
    data::from_json(node, "partialResultToken", res.partialResultToken);
    data::from_json(node, "workDoneToken", res.workDoneToken);
    data::from_json(node, "textDocument", res.textDocument);
    return res;
}

nlohmann::json lsp::data::folding_range_params::to_json() const {
    nlohmann::json json;
    data::set_json(json, "partialResultToken", partialResultToken);
    data::set_json(json, "workDoneToken", workDoneToken);
    data::set_json(json, "textDocument", textDocument);
    return json;
}

lsp::data::publish_diagnostics_params lsp::data::publish_diagnostics_params::from_json(const nlohmann::json &node) {
    publish_diagnostics_params res;
    data::from_json(node, "uri", res.uri);
    data::from_json(node, "version", res.version);
    data::from_json(node, "diagnostics", res.diagnostics);
    return res;
}

nlohmann::json lsp::data::publish_diagnostics_params::to_json() const {
    nlohmann::json json;
    data::set_json(json, "uri", uri);
    data::set_json(json, "version", version);
    data::set_json(json, "diagnostics", diagnostics);
    return json;
}

lsp::data::inlay_hint_params lsp::data::inlay_hint_params::from_json(const nlohmann::json &node) {
    inlay_hint_params res;
    data::from_json(node, "workDoneToken", res.work_done_token);
    data::from_json(node, "textDocument", res.text_document);
    data::from_json(node, "range", res.range);
    return res;
}

nlohmann::json lsp::data::inlay_hint_params::to_json() const {
    nlohmann::json json;
    data::set_json(json, "workDoneToken", work_done_token);
    data::set_json(json, "textDocument", text_document);
    data::set_json(json, "range", range);
    return json;
}

lsp::data::references_params lsp::data::references_params::from_json(const nlohmann::json &node) {
    references_params res;
    data::from_json(node, "partialResultToken", res.partialResultToken);
    data::from_json(node, "workDoneToken", res.workDoneToken);
    data::from_json(node, "textDocument", res.textDocument);
    data::from_json(node, "position", res.position);
    data::from_json(node, "context", res.context);
    return res;
}

nlohmann::json lsp::data::references_params::to_json() const {
    nlohmann::json json;
    data::set_json(json, "partialResultToken", partialResultToken);
    data::set_json(json, "workDoneToken", workDoneToken);
    data::set_json(json, "textDocument", textDocument);
    data::set_json(json, "position", position);
    data::set_json(json, "context", context);
    return json;
}

nlohmann::json lsp::data::references_params::ReferenceContext::to_json() const {
    nlohmann::json json;
    data::set_json(json, "includeDeclaration", includeDeclaration);
    return json;
}

lsp::data::references_params::ReferenceContext
lsp::data::references_params::ReferenceContext::from_json(const nlohmann::json &node) {
    ReferenceContext res{};
    data::from_json(node, "includeDeclaration", res.includeDeclaration);
    return res;
}

nlohmann::json lsp::data::code_action_params::to_json() const {
    nlohmann::json json;
    data::set_json(json, "partialResultToken", partialResultToken);
    data::set_json(json, "workDoneToken", workDoneToken);
    data::set_json(json, "textDocument", textDocument);
    data::set_json(json, "range", range);
    data::set_json(json, "context", context);
    return json;
}

lsp::data::code_action_params lsp::data::code_action_params::from_json(const nlohmann::json &node) {
    code_action_params res;
    data::from_json(node, "partialResultToken", res.partialResultToken);
    data::from_json(node, "workDoneToken", res.workDoneToken);
    data::from_json(node, "textDocument", res.textDocument);
    data::from_json(node, "range", res.range);
    data::from_json(node, "context", res.context);
    return res;
}

lsp::data::code_action_params::code_action_context
lsp::data::code_action_params::code_action_context::from_json(const nlohmann::json &node) {
    code_action_context res;
    data::from_json(node, "diagnostics", res.diagnostics);
    data::from_json(node, "only", res.only);
    data::from_json(node, "triggerKind", res.triggerKind);
    return res;
}

nlohmann::json lsp::data::code_action_params::code_action_context::to_json() const {
    nlohmann::json json;
    data::set_json(json, "diagnostics", diagnostics);
    data::set_json(json, "only", only);
    data::set_json(json, "triggerKind", triggerKind);
    return json;
}

lsp::data::completion_params lsp::data::completion_params::from_json(const nlohmann::json &node) {
    completion_params res;
    data::from_json(node, "partialResultToken", res.partialResultToken);
    data::from_json(node, "workDoneToken", res.workDoneToken);
    data::from_json(node, "textDocument", res.textDocument);
    data::from_json(node, "position", res.position);
    data::from_json(node, "context", res.context);
    return res;
}

nlohmann::json lsp::data::completion_params::to_json() const {
    nlohmann::json json;
    data::set_json(json, "partialResultToken", partialResultToken);
    data::set_json(json, "workDoneToken", workDoneToken);
    data::set_json(json, "textDocument", textDocument);
    data::set_json(json, "position", position);
    data::set_json(json, "context", context);
    return json;
}

lsp::data::completion_params::completion_context
lsp::data::completion_params::completion_context::from_json(const nlohmann::json &node) {
    completion_context res;
    data::from_json(node, "triggerKind", res.trigger_kind);
    data::from_json(node, "triggerCharacter", res.trigger_character);
    return res;
}

nlohmann::json lsp::data::completion_params::completion_context::to_json() const {
    nlohmann::json json;
    data::set_json(json, "triggerKind", trigger_kind);
    data::set_json(json, "triggerCharacter", trigger_character);
    return json;
}

lsp::data::hover_params lsp::data::hover_params::from_json(const nlohmann::json &node) {
    hover_params res;
    data::from_json(node, "workDoneToken", res.work_done_token);
    data::from_json(node, "textDocument", res.text_document);
    data::from_json(node, "position", res.position);
    return res;
}

nlohmann::json lsp::data::hover_params::to_json() const {
    nlohmann::json json;
    data::set_json(json, "workDoneToken", work_done_token);
    data::set_json(json, "textDocument", text_document);
    data::set_json(json, "position", position);
    return json;
}

nlohmann::json lsp::data::hover::to_json() const {
    nlohmann::json json;
    data::set_json(json, "contents", contents);
    data::set_json(json, "range", range);
    return json;
}

lsp::data::inlay_hint lsp::data::inlay_hint::from_json(const nlohmann::json &node) {
    inlay_hint res;
    data::from_json(node, "position", res.position);
    data::from_json(node, "label", res.label);
    data::from_json(node, "kind", res.kind);
    data::from_json(node, "textEdits", res.text_edits);
    data::from_json(node, "tooltip", res.tooltip);
    data::from_json(node, "paddingLeft", res.padding_left);
    data::from_json(node, "paddingRight", res.padding_right);
    data::from_json(node, "data", res.data);
    return res;
}

nlohmann::json lsp::data::inlay_hint::to_json() const {
    nlohmann::json json;
    data::set_json(json, "position", position);
    data::set_json(json, "label", label);
    data::set_json(json, "kind", kind);
    data::set_json(json, "textEdits", text_edits);
    data::set_json(json, "tooltip", tooltip);
    data::set_json(json, "paddingLeft", padding_left);
    data::set_json(json, "paddingRight", padding_right);
    data::set_json(json, "data", data);
    return json;
}
