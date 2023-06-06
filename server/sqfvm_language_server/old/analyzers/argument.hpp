#pragma once
#include "variable_type.hpp"

#include <string>
namespace sqfvm::lsp
{
    struct argument
    {
        variable_type type;
        std::string name;
        std::string description;
        size_t index;
    };
}