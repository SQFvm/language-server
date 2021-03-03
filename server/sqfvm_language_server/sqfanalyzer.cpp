#include "sqfanalyzer.hpp"
#include "repositories/t_files.hpp"
#include "visitors/sqf/variables_visitor.hpp"

#include "parser/sqf/sqf_parser.hpp"


void sqfvm::lsp::sqfanalyzer::recurse(const sqf::parser::sqf::bison::astnode& parent)
{
    m_astkinds.push_back(parent);
    for (auto& visitor : m_visitors)
    {
        visitor->enter(*this, parent, m_astkinds);
    }
    for (auto& child : parent.children)
    {
        recurse(child);
    }
    for (auto& visitor : m_visitors)
    {
        visitor->exit(*this, parent, m_astkinds);
    }
    m_astkinds.pop_back();
}

sqfvm::lsp::sqfanalyzer::sqfanalyzer()
{
    m_visitors.push_back(new sqfvm::lsp::visitors::sqf::variables_visitor());
}

void sqfvm::lsp::sqfanalyzer::analyze(sqf::runtime::runtime& runtime, std::string& document, std::filesystem::path fpath)
{
    for (auto& visitor : m_visitors)
    {
        visitor->start(*this);
    }
    auto logger = StdOutLogger();
    auto parser = sqf::parser::sqf::parser(logger);
    auto tokenizer = sqf::parser::sqf::tokenizer(document.begin(), document.end(), fpath.string());
    sqf::parser::sqf::bison::astnode root;
    parser.get_tree(runtime, tokenizer, &root);
    recurse(root);
    for (auto& visitor : m_visitors)
    {
        visitor->end(*this);
    }
}
