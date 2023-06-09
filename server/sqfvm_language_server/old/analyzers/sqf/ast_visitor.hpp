#pragma once

#include "../../../analysis/analyzer.hpp"
#include <string>
#include <algorithm>
#include <filesystem>
#include "parser/sqf/tokenizer.hpp"
#include "parser/sqf/astnode.hpp"

namespace sqfvm::language_server::analysis::assembly
{
    class assembly_analyzer;

    struct ast_visitor
    {
        virtual ~ast_visitor()
        {
        }

        virtual void start(assembly_analyzer &a) = 0;

        virtual void enter(assembly_analyzer &a, const sqf::parser::sqf::bison::astnode &node,
                           const std::vector<const ::sqf::parser::sqf::bison::astnode *> &parent_nodes) = 0;

        virtual void exit(assembly_analyzer &a, const sqf::parser::sqf::bison::astnode &node,
                          const std::vector<const ::sqf::parser::sqf::bison::astnode *> &parent_nodes) = 0;

        virtual void end(assembly_analyzer &a) = 0;
    };
}