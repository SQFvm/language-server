#pragma once
#include "lspserver.h"

#include <parser/sqf/default.h>

#include <string>
#include <vector>

struct variable_declaration
{
    size_t level;
    lsp::data::position position;
    std::string variable;
    std::vector<lsp::data::position> usages;

    variable_declaration(size_t layer, sqf::parser::sqf::impl_default::astnode node, std::string variable) :
        level(layer),
        position({ node.line, node.column }),
        variable(variable),
        usages() { }
};