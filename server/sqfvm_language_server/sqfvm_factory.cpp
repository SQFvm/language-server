#include "sqfvm_factory.hpp"
#include <operators/ops.h>
#include <fileio/default.h>
#include <parser/sqf/sqf_parser.hpp>
#include <parser/preprocessor/default.h>
#include <parser/config/config_parser.hpp>
#include <utility>

std::shared_ptr<sqf::runtime::runtime> sqfvm::language_server::sqfvm_factory::create(
        std::function<void(const sqfvm::language_server::database::tables::t_diagnostic &)> func,
        sqfvm::language_server::database::context &context) const {
    auto logger = std::make_shared<runtime_logger>(context, func);
    auto runtime = std::make_shared<::sqf::runtime::runtime>(*logger, ::sqf::runtime::runtime::runtime_conf{});
    runtime->add_finalizer([logger]() { /* holds reference to logger */ });
    runtime->fileio(std::make_unique<::sqf::fileio::impl_default>(*logger));
    runtime->parser_config(std::make_unique<::sqf::parser::config::parser>(*logger));
    runtime->parser_sqf(std::make_unique<::sqf::parser::sqf::parser>(*logger));
    runtime->parser_preprocessor(std::make_unique<::sqf::parser::preprocessor::impl_default>(*logger));
    for (const auto &mapping: m_mappings) {
        runtime->fileio().add_mapping(mapping.physical, mapping.virtual_);
    }
    return runtime;
}
