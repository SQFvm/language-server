#pragma once

#include <vector>
#include <string>
#include <string_view>

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
    struct analyze_result
    {
        std::vector<argument> args;
        std::vector<std::string> variables_used;
        std::vector<std::string> variables_set;
        std::vector<method> methods;
    };
    class analyzer
    {
    public:
        virtual void analyze(std::string_view document) const = 0;
    };
}