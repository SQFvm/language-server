#pragma once
#include "runtime/runtime.h"

#include <vector>
#include <string>
#include <string_view>
#include <filesystem>
#include <unordered_map>

namespace sqfvm::lsp
{
    enum class severity
    {
        info,
        warning,
        error
    };
    enum class variable_scope
    {
        global,
        local
    };
    enum class variable_type
    {
        any,
        method
    };
    struct position
    {
        size_t line;
        size_t column;
        size_t length;
        size_t offset;
    };
    struct argument
    {
        variable_type type;
        std::string name;
        std::string description;
        size_t index;
    };
    struct method
    {
        std::string name;
        std::string description;
        variable_type type;
        position position;
        variable_scope scope;
        std::vector<argument> args;
    };
    struct strmtch
    {
        std::string name;
        position pos;
    };
    struct lintinfo
    {
        severity sev;
        position pos;
        std::string msg;
    };
    struct analyze_result
    {
        std::vector<lintinfo> linting;
        std::vector<argument> args;
        std::vector<strmtch> methods_used;
        std::vector<method> methods_set;
        std::vector<strmtch> variables_used;
        std::vector<strmtch> variables_set;
    };
    class analyzer
    {
    public:
        analyze_result res;
        virtual ~analyzer() { }
        virtual void analyze(sqf::runtime::runtime& runtime, std::string& document, std::filesystem::path fpath) = 0;
    };

    class analyzer_factory
    {
    public:
        using generator_func = std::unique_ptr<analyzer>(*)();
    private:
        std::unordered_map<std::string, generator_func> generators;
    public:
        bool has(std::string ext) const
        {
            auto res = generators.find(ext);
            return res != generators.end();
        }
        std::optional<std::unique_ptr<analyzer>> get(std::string ext) const
        {
            auto res = generators.find(ext);
            return res == generators.end() ? std::optional<std::unique_ptr<analyzer>>{} : res->second();
        }
        void set(std::string ext, generator_func generator)
        {
            generators[ext] = generator;
        }
    };
}