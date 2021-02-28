#pragma once

#include <vector>
#include <string>
#include <string_view>
#include <filesystem>
#include "uri.hpp"

namespace sqfvm::lsp
{
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
    struct analyze_result
    {
        std::vector<argument> args;
        std::vector<strmtch> methods_used;
        std::vector<method> methods_set;
        std::vector<strmtch> variables_used;
        std::vector<strmtch> variables_set;
    };
    class analyzer
    {
    public:
        virtual ~analyzer() { }
        virtual bool handles(std::filesystem::path uri) const = 0;
        virtual analyze_result analyze(std::string_view document) const = 0;
    };
}