#include "language_server_logger.h"
#include "sqf_language_server.h"

void language_server_logger::log(const LogMessageBase& base)

{
    auto& message = dynamic_cast<const logmessage::RuntimeLogMessageBase&>(base);
    if (message == nullptr)
    {
        return;
    }

    auto location = message.location();
    auto findRes = language_server.text_documents.find(location.path);
    if (findRes == language_server.text_documents.end())
    {
        return;
    }

    lsp::data::publish_diagnostics_params& params = findRes->second.diagnostics;

    lsp::data::diagnostics msg;
    msg.range.start.line = location.line - 1;
    msg.range.start.character = location.col;
    msg.range.end.line = location.line - 1;
    msg.range.end.character = location.col;


    msg.code = std::to_string(base.getErrorCode());
    msg.message = message.formatMessage();
    msg.source = "SQF-VM";

    switch (message.getLevel())
    {
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
    params.diagnostics.push_back(msg);
}