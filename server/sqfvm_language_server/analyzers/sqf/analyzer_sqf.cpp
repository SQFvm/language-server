#include "analyzer_sqf.hpp"
#include "../../repositories/t_files.hpp"
#include "visitors/sqf/variables_visitor.hpp"

#include "parser/sqf/sqf_parser.hpp"


#pragma clang diagnostic push
#pragma ide diagnostic ignored "misc-no-recursion"
void sqfvm::lsp::analyzer_sqf::recurse(const sqf::parser::sqf::bison::astnode& parent)
{
    m_descend_ast_nodes.push_back(&parent);
    for (auto& visitor : m_visitors)
    {
        visitor->enter(*this, parent, m_descend_ast_nodes);
    }
    for (auto& child : parent.children)
    {
        recurse(child);
    }
    for (auto& visitor : m_visitors)
    {
        visitor->exit(*this, parent, m_descend_ast_nodes);
    }
    m_descend_ast_nodes.pop_back();
}
#pragma clang diagnostic pop

sqfvm::lsp::analyzer_sqf::analyzer_sqf()
{
    m_visitors.push_back(new sqfvm::lsp::visitors::sqf::variables_visitor());
}

void sqfvm::lsp::analyzer_sqf::analyze(sqf::runtime::runtime& runtime, std::string& document, std::filesystem::path file_path)
{
    for (auto& visitor : m_visitors)
    {
        visitor->start(*this);
    }
    auto logger = StdOutLogger();
    auto parser = sqf::parser::sqf::parser(logger);
    auto tokenizer = sqf::parser::sqf::tokenizer(document.begin(), document.end(), file_path.string());
    sqf::parser::sqf::bison::astnode root;
    parser.get_tree(runtime, tokenizer, &root);
    recurse(root);
    for (auto& visitor : m_visitors)
    {
        visitor->end(*this);
    }
}
