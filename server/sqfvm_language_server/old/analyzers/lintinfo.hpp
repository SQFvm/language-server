#pragma once
#include "severity.hpp"
#include "position.hpp"

#include <string>
namespace sqfvm::lsp
{
    struct lintinfo
    {
        severity sev;
        position pos;
        std::string msg;
    };
}