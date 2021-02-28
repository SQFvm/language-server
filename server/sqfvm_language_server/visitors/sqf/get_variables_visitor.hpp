#pragma once
#include "../../sqfanalyzer.hpp"

namespace sqfvm::lsp::visitors::sqf
{
    struct get_variable_visitor : public sqfvisitor
    {
        virtual ~get_variable_visitor() { }
        virtual void enter(sqfanalyzer& a, const ::sqf::parser::sqf::bison::astnode& node) override { }
        virtual void exit(sqfanalyzer& a, const ::sqf::parser::sqf::bison::astnode& node) override { }
    };
}