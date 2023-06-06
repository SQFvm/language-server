#include "analyzer_sqf.hpp"
#include "../../database/repositories/t_file.hpp"
#include "visitors/sqf/variables_visitor.hpp"

#include "parser/sqf/sqf_parser.hpp"


#pragma clang diagnostic push
#pragma ide diagnostic ignored "misc-no-recursion"

void sqfvm::language_server::analyzer_sqf::recurse(const sqf::parser::sqf::bison::astnode &parent) {
    m_descend_ast_nodes.push_back(&parent);
    for (auto &visitor: m_visitors) {
        visitor->enter(*this, parent, m_descend_ast_nodes);
    }
    for (auto &child: parent.children) {
        recurse(child);
    }
    for (auto &visitor: m_visitors) {
        visitor->exit(*this, parent, m_descend_ast_nodes);
    }
    m_descend_ast_nodes.pop_back();
}

#pragma clang diagnostic pop

sqfvm::language_server::analyzer_sqf::analyzer_sqf() {
    m_visitors.push_back(new sqfvm::language_server::visitors::sqf::variables_visitor());
}

void sqfvm::language_server::analyzer_sqf::analyze(
        sqf::runtime::runtime &runtime,
        std::string &document,
        ::sqfvm::language_server::repositories::file &f) {
    for (auto &visitor: m_visitors) {
        visitor->start(*this);
    }
    auto logger = StdOutLogger();
    auto parser = sqf::parser::sqf::parser(logger);
    auto tokenizer = sqf::parser::sqf::tokenizer(document.begin(), document.end(), f.path.string());
    sqf::parser::sqf::bison::astnode root;
    parser.get_tree(runtime, tokenizer, &root);
    recurse(root);
    for (auto &visitor: m_visitors) {
        visitor->end(*this);
    }
}

void sqfvm::language_server::analyzer_sqf::commit(
        sqlite::database &db,
        sqf::runtime::runtime &runtime,
        std::string &document,
        ::sqfvm::language_server::repositories::file &f) {
    repositories::file::set(db, f);
    for (auto &it: res.methods_set) {
        repositories::reference ref = {
                .id = 0,
                .file_fk = f.id,
                .type = repositories::reference::etype::method_set,
                .name = it.name,
                .position = it.position,
        };
        repositories::reference::add(db, ref);
    }
    for (auto &it: res.methods_used) {
        repositories::reference ref = {
                .id = 0,
                .file_fk = f.id,
                .type = repositories::reference::etype::method_used,
                .name = it.name,
                .position = it.position,
        };
        repositories::reference::add(db, ref);
    }
    for (auto &it: res.variables_set) {
        repositories::reference ref = {
                .id = 0,
                .file_fk = f.id,
                .type = repositories::reference::etype::variable_set,
                .name = it.name,
                .position = it.position,
        };
        repositories::reference::add(db, ref);
    }
    for (auto &it: res.variables_used) {
        repositories::reference ref = {
                .id = 0,
                .file_fk = f.id,
                .type = repositories::reference::etype::variable_used,
                .name = it.name,
                .position = it.position,
        };
        repositories::reference::add(db, ref);
    }
}
