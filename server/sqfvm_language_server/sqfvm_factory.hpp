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
        struct mapping_tuple {
            sqf::runtime::fileio::pathinfo mapping;
            bool is_workspace_mapping;
        };
        std::vector<mapping_tuple> m_mappings;
    public:
        void add_mapping(std::string physical_path, std::string virtual_path, bool is_workspace_mapping = false) {
            m_mappings.emplace_back(
                sqf::runtime::fileio::pathinfo{
                    std::move(physical_path),
                    std::move(virtual_path)
                },
                false
            );
        }
        void update_mapping(std::string physical_path, std::string virtual_path, bool is_workspace_mapping = false) {
            for (auto& tuple : m_mappings) {
                auto mapping = tuple.mapping;
                if (mapping.physical == physical_path) {
                    mapping.virtual_ = std::move(virtual_path);
                    return;
                }
            }
            add_mapping(std::move(physical_path), std::move(virtual_path), is_workspace_mapping);
        }

        void remove_mapping(std::string physical_path) {
            for (auto it = m_mappings.begin(); it != m_mappings.end(); ++it) {
                if (it->mapping.physical == physical_path) {
                    m_mappings.erase(it);
                    return;
                }
            }
        }

        void clear_workspace_mappings() {
            for (auto i = 0; i < m_mappings.size(); ++i) {
                if (m_mappings[i].is_workspace_mapping) {
                    m_mappings.erase(m_mappings.begin() + i);
                    --i;
                }
            }
        }

        [[nodiscard]] std::shared_ptr<sqf::runtime::runtime> create(
                const std::function<void(const sqfvm::language_server::database::tables::t_diagnostic &)>& log,
                database::context &context,
                const std::shared_ptr<analysis::slspp_context>& slspp
        ) const;
    };
}

#endif //SQFVM_LANGUAGE_SERVER_SQFVM_FACTORY_HPP
