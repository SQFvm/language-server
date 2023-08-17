#ifndef SQFVM_LANGUAGE_SERVER_SQFVM_FACTORY_HPP
#define SQFVM_LANGUAGE_SERVER_SQFVM_FACTORY_HPP

#include "runtime_logger.hpp"
#include "analysis/slspp_context.hpp"
#include "database/context.hpp"
#include <runtime/runtime.h>
#include <runtime/fileio.h>
#include <functional>
#include <memory>
#include <vector>

namespace sqfvm::language_server {
    class sqfvm_factory {
        std::vector<sqf::runtime::fileio::pathinfo> m_mappings;
    public:
        void add_mapping(std::string physical_path, std::string virtual_path) {
            m_mappings.emplace_back(std::move(physical_path), std::move(virtual_path));
        }

        [[nodiscard]] std::shared_ptr<sqf::runtime::runtime> create(
                const std::function<void(const sqfvm::language_server::database::tables::t_diagnostic &)>& log,
                database::context &context,
                const std::shared_ptr<analysis::slspp_context>& slspp
        ) const;
    };
}

#endif //SQFVM_LANGUAGE_SERVER_SQFVM_FACTORY_HPP
