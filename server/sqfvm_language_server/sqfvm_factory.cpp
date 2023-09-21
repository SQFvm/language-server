#include "sqfvm_factory.hpp"
#include <operators/ops.h>
#include <fileio/default.h>
#include <parser/sqf/sqf_parser.hpp>
#include <parser/preprocessor/default.h>
#include <parser/config/config_parser.hpp>
#include <utility>
#include "language_server.hpp"


void sqfvm::language_server::sqfvm_factory::log_to_window(const LogMessageBase &msg) const {
    auto level = msg.getLevel();
    ::lsp::data::message_type message_type;
    switch (level) {
        case loglevel::fatal:
        case loglevel::error:
            message_type = ::lsp::data::message_type::Error;
            break;
        case loglevel::warning:
            message_type = ::lsp::data::message_type::Warning;
            break;
        case loglevel::info:
            message_type = ::lsp::data::message_type::Info;
            break;
        case loglevel::verbose:
        case loglevel::trace:
            message_type = ::lsp::data::message_type::Log;
            break;
    }
    m_language_server->window_log(message_type, [&](auto &sstream) {
        auto error_code = msg.getErrorCode();
        auto message = msg.formatMessage();
        switch (level) {
            case loglevel::fatal:
                sstream << "[" << std::setfill('0') << std::setw(5) << error_code << "] FTL: " << message;
                break;
            case loglevel::error:
                sstream << "[" << std::setfill('0') << std::setw(5) << error_code << "] ERR: " << message;
                break;
            case loglevel::warning:
                sstream << "[" << std::setfill('0') << std::setw(5) << error_code << "] WRN: " << message;
                break;
            case loglevel::info:
                sstream << "[" << std::setfill('0') << std::setw(5) << error_code << "] INF: " << message;
                break;
            case loglevel::verbose:
                sstream << "[" << std::setfill('0') << std::setw(5) << error_code << "] VRB: " << message;
                break;
            case loglevel::trace:
                sstream << "[" << std::setfill('0') << std::setw(5) << error_code << "] TRC: " << message;
                break;
        }


    });
}

std::shared_ptr<sqf::runtime::runtime> sqfvm::language_server::sqfvm_factory::create(
        const std::function<void(const sqfvm::language_server::database::tables::t_diagnostic &)> &log,
        sqfvm::language_server::database::context &context,
        const std::shared_ptr<analysis::slspp_context> &slspp) const {
    using namespace std::string_literals;
    auto logger = std::make_shared<runtime_logger>(
            context,
            [&](const LogMessageBase &msg) { log_to_window(msg); },
            log);
    auto runtime = std::make_shared<::sqf::runtime::runtime>(*logger, ::sqf::runtime::runtime::runtime_conf{});
    runtime->add_finalizer([logger]() { /* holds reference to logger */ });
    runtime->fileio(std::make_unique<::sqf::fileio::impl_default>(*logger));
    runtime->parser_config(std::make_unique<::sqf::parser::config::parser>(*logger));
    runtime->parser_sqf(std::make_unique<::sqf::parser::sqf::parser>(*logger));
    auto preprocessor = std::make_unique<::sqf::parser::preprocessor::impl_default>(*logger);

    preprocessor->push_back(::sqf::runtime::parser::pragma{"sls"s, [slspp](
            const ::sqf::runtime::parser::pragma &self,
            ::sqf::runtime::runtime &runtime,
            ::sqf::runtime::parser::preprocessor::context &file_context,
            const std::string &data) -> std::optional<std::string> {
        // The offset required to convert from the human-readable line index (1-based) to the 0-based line index
        // including the end-of-line offset to correct for pragma read position
        const size_t line_offset = -2;
        // split data by spaces
        std::vector<std::string> args{};
        std::string_view data_view{data};
        while (!data_view.empty()) {
            auto space = data_view.find(' ');
            if (space == std::string_view::npos) {
                args.emplace_back(data_view);
                break;
            }
            args.emplace_back(data_view.substr(0, space));
            data_view.remove_prefix(space + 1);
        }
        if (args.size() < 2) {
            return std::nullopt;
        }
        auto command = args.front();
        auto error_code = args.back();
        if (command == "enable") {
            slspp->push_enable(file_context.path.physical, file_context.line + line_offset, error_code);
        } else if (command == "disable") {
            if (args.size() < 3) {
                slspp->push_disable(file_context.path.physical, file_context.line + line_offset, error_code);
            } else if (args[1] == "line") {
                slspp->push_disable_line(file_context.path.physical, file_context.line + line_offset, error_code);
            } else if (args[1] == "file") {
                slspp->push_disable_file(error_code);
            }
        }
        return std::nullopt;
    }});
    runtime->parser_preprocessor(std::move(preprocessor));
    sqf::operators::ops(*runtime);
    for (const auto &tuple: m_mappings) {
        auto& mapping = tuple.mapping;
        runtime->fileio().add_mapping(mapping.physical, mapping.virtual_);
    }
    return runtime;
}
