#pragma once

#include "variable_type.hpp"
#include "position.hpp"
#include "argument.hpp"
#include "variable_scope.hpp"
#include <string>
#include <vector>

namespace sqfvm::lsp
{
    struct method
    {
        std::string name;
        std::string description;
        variable_type type;
        position position;
        variable_scope varscope;
        std::string scope;
        std::vector<argument> args;
    };
}