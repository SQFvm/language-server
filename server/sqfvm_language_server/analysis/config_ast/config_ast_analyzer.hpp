//
// Created by marco.silipo on 17.08.2023.
//

#ifndef SQFVM_LANGUAGE_SERVER_CONFIG_AST_ANALYZER_HPP
#define SQFVM_LANGUAGE_SERVER_CONFIG_AST_ANALYZER_HPP

#include "../slspp_context.hpp"
#include "../sqfvm_analyzer.hpp"
#include "../../sqfvm_factory.hpp"
#include <string_view>
#include <utility>
#include <parser/sqf/astnode.hpp>

namespace sqfvm::language_server::analysis::config_ast {
    class config_ast_analyzer : public sqfvm_analyzer {

    };
}

#endif //SQFVM_LANGUAGE_SERVER_CONFIG_AST_ANALYZER_HPP
