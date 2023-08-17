#ifndef SQFVM_LANGUAGE_SERVER_RUNTIME_LOGGER_HPP
#define SQFVM_LANGUAGE_SERVER_RUNTIME_LOGGER_HPP

#include "database/context.hpp"
#include "util.hpp"
#include <runtime/runtime.h>

namespace sqfvm::language_server {
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
            // Skip virtual file lookup errors
            if (base.getErrorCode() >= 70000 && base.getErrorCode() < 80000 && base.getErrorCode() != 70014) {
                return;
            }
            using namespace sqlite_orm;
            auto &message = dynamic_cast<const logmessage::RuntimeLogMessageBase &>(base);
            if (message == nullptr) {
                return;
            }

            auto location = message.location();
            auto uri = sanitize_to_uri(location.path);
            auto path_str = sanitize_to_string(uri);
            auto path = std::filesystem::path(path_str).lexically_normal();

            auto file = m_context.db_get_file_from_path(path, true);
            if (!file.has_value()) {
                m_func({
                       .severity = sqfvm::language_server::database::tables::t_diagnostic::severity_level::error,
                       .message = "Failed to find file '" + path.string() + "' in database",
               });
                return;
            }

            sqfvm::language_server::database::tables::t_diagnostic msg;
            msg.file_fk = file->id_pk;
            msg.source_file_fk = file->id_pk;
            msg.line = location.line - 1;
            msg.column = location.col;
            msg.offset = location.offset;
            msg.length = location.length;
            msg.content = std::to_string(base.getErrorCode());
            msg.message = message.formatMessage();
            {
                std::stringstream sstream;
                sstream << "SQFVM-" << std::setfill('0') << std::setw(5) << base.getErrorCode();
                msg.code = sstream.str();
            }
            msg.severity = static_cast<sqfvm::language_server::database::tables::t_diagnostic::severity_level>(base.getLevel());

            m_func(msg);
        }
    };
}

#endif //SQFVM_LANGUAGE_SERVER_RUNTIME_LOGGER_HPP
