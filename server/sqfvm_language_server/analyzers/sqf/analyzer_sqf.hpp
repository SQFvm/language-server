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

        void analyze(sqf::runtime::runtime &runtime, std::string &document, std::filesystem::path file_path) override;
    };
}