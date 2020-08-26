#pragma once
#include "lspserver.h"

#include <parser/sqf/default.h>

#include <string>
#include <vector>
#include <memory>

struct variable_declaration
{
    using sptr = std::shared_ptr<variable_declaration>;
    struct parameter
    {
        // The types accepted by this parameter
        std::vector<sqf::runtime::type> types;

        // Wether this parameter is optional or not
        bool optional;
    };
    // The ast-level this was created at
    size_t level;

    // Position info for the initial variable declaration
    lsp::data::position position;

    // The actual variable name
    std::string variable;

    // The different positions where this is being used
    std::vector<lsp::data::position> usages;

    // The different types, known for this variable
    std::vector<sqf::runtime::type> types;

    // If one known type is code, this will contain the params expected by this piece of code.
    std::vector<parameter> args;

    // The uri of the owning file. Will be empty for private variables.
    std::string owner;

    variable_declaration(size_t layer, sqf::parser::sqf::impl_default::astnode node, std::string variable) :
        level(layer),
        position({ node.line, node.column }),
        variable(variable),
        usages() { }
};