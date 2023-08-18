#pragma once

#include "../database/context.hpp"
#include "../sqfvm_factory.hpp"

#include <runtime/runtime.h>
#include <utility>
#include <vector>
#include <string>
#include <string_view>
#include <filesystem>
#include <unordered_map>

namespace sqfvm::language_server::analysis {
    // An analyzer is responsible for analyzing a document and committing the analysis to the database.
    // The analysis is split into two steps:
    // 1. Analyze the document and gather references of variables, functions, etc.
    // 2. Commit the analysis to the database.
    class analyzer {
    public:
        virtual ~analyzer() = default;

        // Perform an abstract analysis of the document, gathering references of variables, functions, etc.
        // to be committed to the database in the next step.
        virtual void analyze() = 0;

        // Commit the analysis to the database.
        virtual void commit() = 0;
    };

    class analyzer_factory {
    public:
        using generator_func = std::unique_ptr<analyzer>(*)(
                std::filesystem::path ls_path,
                std::filesystem::path db_path,
                sqfvm_factory &,
                database::tables::t_file,
                std::string &);
    private:
        std::unordered_map<std::string, generator_func> m_generators;
    public:
        [[nodiscard]] bool has(const std::string &ext) const {
            auto res = m_generators.find(ext);
            return res != m_generators.end();
        }

        [[nodiscard]] std::optional<std::unique_ptr<analyzer>> get(
                const std::string &ext,
                std::filesystem::path ls_path,
                std::filesystem::path db_path,
                sqfvm_factory &factory,
                const database::tables::t_file &file,
                std::string &text) const {
            auto res = m_generators.find(ext);
            return res == m_generators.end()
                   ? std::optional<std::unique_ptr<analyzer>>{}
                   : res->second(
                            std::move(ls_path),
                            std::move(db_path),
                            factory,
                            file,
                            text);
        }

        void set(const std::string &ext, generator_func generator) {
            m_generators[ext] = generator;
        }
    };
}