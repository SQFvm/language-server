#pragma once
#include "analyzer.hpp"
#include <string>
#include <algorithm>s

namespace sqfvm::lsp
{
    class sqfanalyzer : public analyzer
    {
    public:
        virtual ~sqfanalyzer() { }
        virtual bool handles(std::filesystem::path path) const override
        {
            auto str = path.extension().string();
            std::transform(str.begin(), str.end(), str.begin(), [](char c) -> char { return (char)std::tolower(c); });
            return str == ".sqf";
        }
        virtual analyze_result analyze(std::string_view document) const override;
    };
}