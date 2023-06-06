#pragma once

#include "../analyzer.hpp"
#include <string>
#include <algorithm>
#include <filesystem>
#include "parser/sqf/tokenizer.hpp"
#include "parser/sqf/astnode.hpp"

namespace sqfvm::lsp
{
    class analyzer_sqf;

    struct sqfvisitor
    {
        virtual ~sqfvisitor()
        {
        }

        virtual void start(analyzer_sqf &a) = 0;

        virtual void enter(analyzer_sqf &a, const sqf::parser::sqf::bison::astnode &node,
                           const std::vector<const ::sqf::parser::sqf::bison::astnode *> &parent_nodes) = 0;

        virtual void exit(analyzer_sqf &a, const sqf::parser::sqf::bison::astnode &node,
                          const std::vector<const ::sqf::parser::sqf::bison::astnode *> &parent_nodes) = 0;

        virtual void end(analyzer_sqf &a) = 0;
    };
}