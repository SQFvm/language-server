#pragma once

#include "position.hpp"
#include <string>

namespace sqfvm::lsp
{
    struct strmtch
    {
        std::string scope;
        std::string name;
        position position;
    };
}