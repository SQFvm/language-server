#pragma once
#include "analyzer.hpp"
#include <string>
#include <algorithm>
#include <filesystem>
#include "parser/sqf/tokenizer.hpp"
#include "parser/sqf/astnode.hpp"

namespace sqfvm::lsp
{
    class sqfanalyzer;
    struct sqfvisitor
    {
        virtual ~sqfvisitor() { }
        virtual void start(sqfanalyzer& a) = 0;
        virtual void enter(sqfanalyzer& a, const sqf::parser::sqf::bison::astnode& node, const std::vector<const ::sqf::parser::sqf::bison::astnode&>& astkinds) = 0;
        virtual void exit(sqfanalyzer& a, const sqf::parser::sqf::bison::astnode& node, const std::vector<const ::sqf::parser::sqf::bison::astnode&>& astkinds) = 0;
        virtual void end(sqfanalyzer& a) = 0;
    };
    class sqfanalyzer : public analyzer
    {
        std::vector<sqfvisitor*> m_visitors;
        std::vector<const ::sqf::parser::sqf::bison::astnode&> m_astkinds;

        void recurse(const sqf::parser::sqf::bison::astnode& parent);
    public:
        sqfanalyzer();
        virtual ~sqfanalyzer()
        {
            for (auto v : m_visitors)
            {
                delete v;
            }
        }
        virtual void analyze(sqf::runtime::runtime& runtime, std::string& document, std::filesystem::path fpath) override;
    };
}