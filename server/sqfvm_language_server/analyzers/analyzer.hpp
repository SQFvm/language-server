#pragma once

#include "runtime/runtime.h"

#include <vector>
#include <string>
#include <string_view>
#include <filesystem>
#include <unordered_map>
#include "severity.hpp"
#include "variable_scope.hpp"
#include "variable_type.hpp"
#include "position.hpp"
#include "argument.hpp"
#include "method.hpp"
#include "strmtch.hpp"
#include "lintinfo.hpp"
#include "analyzer_result.hpp"

namespace sqfvm::lsp
{
    class analyzer
    {
    public:
        analyze_result res;

        virtual ~analyzer() = default;

        virtual void analyze(sqf::runtime::runtime &runtime, std::string &document, std::filesystem::path fpath) = 0;
    };

    class analyzer_factory
    {
    public:
        using generator_func = std::unique_ptr<analyzer>(*)();
    private:
        std::unordered_map<std::string, generator_func> generators;
    public:
        [[nodiscard]] bool has(const std::string &ext) const
        {
            auto res = generators.find(ext);
            return res != generators.end();
        }

        [[nodiscard]] std::optional<std::unique_ptr<analyzer>> get(const std::string &ext) const
        {
            auto res = generators.find(ext);
            return res == generators.end() ? std::optional<std::unique_ptr<analyzer>> { } : res->second();
        }

        void set(const std::string &ext, generator_func generator)
        {
            generators[ext] = generator;
        }
    };
}