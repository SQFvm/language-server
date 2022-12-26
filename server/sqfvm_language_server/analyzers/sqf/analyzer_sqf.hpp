#pragma once

#include "../analyzer.hpp"
#include <string>
#include <algorithm>
#include <filesystem>
#include "parser/sqf/tokenizer.hpp"
#include "parser/sqf/astnode.hpp"
#include "sqfvisitor.hpp"

namespace sqfvm::lsp
{
    class analyzer_sqf :
            public analyzer
    {
        std::vector<sqfvisitor *> m_visitors;
        std::vector<const ::sqf::parser::sqf::bison::astnode *> m_descend_ast_nodes;

        void recurse(const sqf::parser::sqf::bison::astnode &parent);

    public:
        analyzer_sqf();

        ~analyzer_sqf() override
        {
            for (auto v: m_visitors)
            {
                delete v;
            }
        }

        void analyze(sqf::runtime::runtime &runtime, std::string &document, ::sqfvm::lsp::repositories::file& f) override;
        void commit(sqlite::database& db, sqf::runtime::runtime &runtime, std::string &document, ::sqfvm::lsp::repositories::file& f) override;
    };
}