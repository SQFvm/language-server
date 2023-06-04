#include "language_server_logger.h"
#include "sqf_language_server.h"
#include <sstream>

void language_server_logger::log(const LogMessageBase &base) {
    {
        std::stringstream sstream;
        sstream << Logger::loglevelstring(base.getLevel()) << ' ' << base.formatMessage();
        language_server.window_logMessage(lsp::data::message_type::Log, sstream.str());
    }
    auto &message = dynamic_cast<const logmessage::RuntimeLogMessageBase &>(base);
    if (message == nullptr) {
        return;
    }

    auto location = message.location();
    auto uri = sanitize_to_uri(location.path);
    auto fpath = sanitize_to_string(uri);
    lsp::data::diagnostics msg;
    msg.range.start.line = location.line - 1;
    msg.range.start.character = location.col;
    msg.range.end.line = location.line - 1;
    msg.range.end.character = location.col;


    msg.code = std::to_string(base.getErrorCode());
    msg.message = message.formatMessage();
    msg.source = "SQF-VM";

    switch (message.getLevel()) {
        case loglevel::fatal:
        case loglevel::error:
            msg.severity = lsp::data::diagnostic_severity::Error;
            break;
        case loglevel::warning:
            msg.severity = lsp::data::diagnostic_severity::Warning;
            break;
        case loglevel::info:
            msg.severity = lsp::data::diagnostic_severity::Information;
            break;
        case loglevel::verbose:
        case loglevel::trace:
        default:
            msg.severity = lsp::data::diagnostic_severity::Hint;
            break;
    }


    language_server.get_or_create(uri)->locked([&](auto& doc) {
        lsp::data::publish_diagnostics_params &params = doc.diagnostics;
        params.diagnostics.push_back(msg);
    });
}