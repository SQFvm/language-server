#pragma once

#include "lintinfo.hpp"
#include "argument.hpp"
#include "strmtch.hpp"
#include "method.hpp"
#include <vector>

namespace sqfvm::lsp
{
    struct analyze_result
    {
        std::vector<lintinfo> linting;
        std::vector<argument> args;
        std::vector<strmtch> methods_used;
        std::vector<method> methods_set;
        std::vector<strmtch> variables_used;
        std::vector<strmtch> variables_set;
    };
}