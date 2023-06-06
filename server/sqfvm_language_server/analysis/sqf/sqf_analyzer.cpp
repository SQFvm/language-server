#include "sqf_analyzer.hpp"
#include "fileio/default.h"
#include "parser/sqf/sqf_parser.hpp"
#include "parser/preprocessor/default.h"
#include "parser/config/config_parser.hpp"
#include "operators/ops.h"
#include "../../util.hpp"

#include <functional>
#include <utility>

namespace {
    class runtime_logger : public Logger {
        sqfvm::language_server::database::context &m_context;
        std::function<void(const sqfvm::language_server::database::tables::t_diagnostic &)> m_func;
    public:
        explicit runtime_logger(
                sqfvm::language_server::database::context &context,
                std::function<void(const sqfvm::language_server::database::tables::t_diagnostic &)> func)
                : Logger(),
                  m_context(context),
                  m_func(std::move(func)) {}

        void log(const LogMessageBase &base) override {
            using namespace sqlite_orm;
            auto &message = dynamic_cast<const logmessage::RuntimeLogMessageBase &>(base);
            if (message == nullptr) {
                return;
            }

            auto location = message.location();
            auto uri = sanitize_to_uri(location.path);
            auto path = sanitize_to_string(uri);

            auto file_id = m_context.storage().get_optional<sqfvm::language_server::database::tables::t_file>(
                    where(c(&sqfvm::language_server::database::tables::t_file::path) == path));
            if (!file_id.has_value()) {
                m_func({
                               .severity = sqfvm::language_server::database::tables::t_diagnostic::severity_level::error,
                               .message = "Failed to get file: " + path,
                       });
                return;
            }

            sqfvm::language_server::database::tables::t_diagnostic msg;
            msg.file_fk = file_id->id_pk;
            msg.line = location.line - 1;
            msg.column = location.col;
            msg.offset = location.offset;
            msg.length = location.length;
            msg.content = std::to_string(base.getErrorCode());
            msg.message = message.formatMessage();
            msg.severity = static_cast<sqfvm::language_server::database::tables::t_diagnostic::severity_level>(base.getLevel());

            m_func(msg);
        }
    };
}

void sqfvm::language_server::analysis::sqf::sqf_analyzer::analyze(database::context &context) {
    auto logger = runtime_logger(context, [&](auto &msg) { m_diagnostics.push_back(msg); });
    auto runtime = ::sqf::runtime::runtime(logger, {});
    runtime.fileio(std::make_unique<::sqf::fileio::impl_default>(logger));
    runtime.parser_config(std::make_unique<::sqf::parser::config::parser>(logger));
    runtime.parser_sqf(std::make_unique<::sqf::parser::sqf::parser>(logger));
    runtime.parser_preprocessor(std::make_unique<::sqf::parser::preprocessor::impl_default>(logger));
    ::sqf::operators::ops(runtime);
    auto path_info = runtime.fileio().get_info(m_file.path, {});
    if (!path_info.has_value()) {
        m_diagnostics.push_back({
                                        .severity = database::tables::t_diagnostic::severity_level::error,
                                        .message = "Failed to get path info for file: " + m_file.path,
                                });
        return;
    }
    auto parser = std::make_unique<::sqf::parser::sqf::parser>(logger);
    auto path = ::sqf::runtime::fileio::pathinfo(m_file.path, "");
    auto instruction_set = parser->parse(runtime, m_text, path);
    if (!instruction_set.has_value())
        return;
    analyze_instruction_set(instruction_set.value());
}

void sqfvm::language_server::analysis::sqf::sqf_analyzer::commit(sqfvm::language_server::database::context &) {
    // ToDo: Synchronize current analysis results with the database.
}
