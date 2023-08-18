//
// Created by marco.silipo on 17.08.2023.
//

#include "scripted_visitor.hpp"
#include <fstream>
#include "runtime/d_array.h"
#include "runtime/d_string.h"
#include "runtime/d_scalar.h"

using namespace ::sqf::runtime;
using namespace ::sqf::types;
using namespace ::sqfvm::language_server::database::tables;

struct storage : public runtime::datastorage {
    sqfvm::language_server::analysis::sqf_ast::visitors::scripted_visitor *m_visitor;
    sqfvm::language_server::analysis::sqf_ast::sqf_ast_analyzer* m_ast_analyzer;
};

namespace sqf {
    namespace runtime {
        struct t_astnode : public type::extend<t_astnode> {
            t_astnode() : extend() {}

            static const std::string name() { return "ASTNODE"; }
        };
    }
    namespace types {
        class d_astnode : public sqf::runtime::data {
        public:
            using data_type = sqf::runtime::t_scalar;
        private:
            const sqf::parser::sqf::bison::astnode *m_node;
        protected:
            bool do_equals(std::shared_ptr<data> other, bool invariant) const override {
                return m_node == std::static_pointer_cast<d_astnode>(other)->m_node;
            }

        public:
            d_astnode() = default;

            std::string to_string_sqf() const override {
                return "\"ASTNODE\"";
            }

            std::string to_string() const override {
                return "ASTNODE";
            }

            const sqf::parser::sqf::bison::astnode &node() const { return *m_node; }

            void node(const sqf::parser::sqf::bison::astnode &node) { m_node = &node; }

            sqf::runtime::type type() const override { return data_type(); }

            virtual std::size_t hash() const override {
                return std::hash<const sqf::parser::sqf::bison::astnode *>()(m_node);
            }
        };
    }

}

void sqfvm::language_server::analysis::sqf_ast::visitors::scripted_visitor::start(
        sqfvm::language_server::analysis::sqf_ast::sqf_ast_analyzer &a) {
    if (m_is_disabled)
        return;
    auto ls_path = ls_folder_of(a);
    if (!exists(ls_path / "use_scripted_analyzers")) { // magic file to disable scripted analyzers
        m_is_disabled = true;
        return;
    }
    auto base_path = ls_path / "scripted" / "analyzers" / "sqf";
    if (!exists(base_path))
        create_directories(base_path);

    auto runtime = runtime_of(a);
    initialize_functions_and_documentation(a, runtime, base_path / "ReadMe.md");
    initialize_start_script(runtime, base_path / "start.sqf");
    initialize_enter_script(runtime, base_path / "enter.sqf");
    initialize_exit_script(runtime, base_path / "exit.sqf");
    initialize_end_script(runtime, base_path / "end.sqf");
    initialize_analyze_script(runtime, base_path / "analyze.sqf");

    call(runtime, m_start_script, {});
}

void sqfvm::language_server::analysis::sqf_ast::visitors::scripted_visitor::initialize_functions_and_documentation(
        sqfvm::language_server::analysis::sqf_ast::sqf_ast_analyzer &a,
        std::shared_ptr<sqf::runtime::runtime> runtime,
        std::filesystem::path file) {
    if (!exists(file)) {
        auto f = std::ofstream(file);
        f << "# Welcome to scripted analyzers\n"
             "\n"
             "This is a short introduction to scripted analyzers.\n"
             "It will cover the basics of how to write your own analyzer and how to use it.\n"
             "\n"
             "# What are scripted analyzers?\n"
             "\n"
             "Scripted analyzers are a way to extend the functionality of the language server\n"
             "by writing your own analyzers in SQF. This allows you to write e.g. your own\n"
             "diagnostics. The analyzers are run on the server and the results are sent to\n"
             "the client, which will display them in the editor.\n"
             "\n"
             "# Data structures\n"
             "\n"
             "The following are the data structures that are available to the analyzers.\n"
             "They will be referred in the documentation by writing `<type>` where type is\n"
             "the name of the data structure.\n"
             "\n"
             "## severity\n"
             "\n"
             "```sqf\n"
             "// One of the following:\n"
             "\"FATAL\";\n"
             "\"ERROR\";\n"
             "\"WARNING\";\n"
             "\"INFO\";\n"
             "\"VERBOSE\";\n"
             "\"TRACE\";\n"
             "```\n"
             "\n"
             "The severity of a diagnostic. The higher the severity, the more important the\n"
             "diagnostic is.\n"
             "FATAL is the highest severity and TRACE is the lowest.\n"
             "\n"
             "| Severity | Description                                                                                                                       | Problems | Editor |\n"
             "|----------|-----------------------------------------------------------------------------------------------------------------------------------|----------|--------|\n"
             "| FATAL    | The highest severity, should be used sparingly.                                                                                   | ERROR    | RED    |\n"
             "| ERROR    | The second-highest severity, should be used for problems that prevent the code from running.                                      | ERROR    | RED    |\n"
             "| WARNING  | The third-highest severity, should be used for problems that might cause unexpected behavior.                                     | WARNING  | YELLOW |\n"
             "| INFO     | The fourth-highest severity, should be used for problems that are not necessarily problems, but might be interesting to the user. | INFO     | WHITE  |\n"
             "| VERBOSE  | The second-lowest severity, should be used for when INFO is too noisy.                                                            |          | GRAY   |\n"
             "| TRACE    | The lowest severity.                                                                                                              |          | GRAY   |\n"
             "\n"
             "## diagnostic\n"
             "\n"
             "```sqf\n"
             "[\n"
             "    severity,      // <severity>\n"
             "    error_code,    // string\n"
             "    content,       // string\n"
             "    message,       // string\n"
             "    line,          // scalar\n"
             "    column,        // scalar\n"
             "    offset,        // scalar\n"
             "    length,        // scalar\n"
             "    file_id,       // scalar\n"
             "]\n"
             "```\n"
             "\n"
             "A diagnostic is a problem that is found in the code. It is represented by an\n"
             "array of the above structure. The fields are as follows:\n"
             "\n"
             "| Field      | Description                             | Type                  |\n"
             "|------------|-----------------------------------------|-----------------------|\n"
             "| severity   | The severity of the diagnostic.         | [severity](#severity) |\n"
             "| error_code | A unique identifier for the diagnostic. | string                |\n"
             "| content    | The content of the diagnostic.          | string                |\n"
             "| message    | The message of the diagnostic.          | string                |\n"
             "| line       | The line of the diagnostic.             | scalar                |\n"
             "| column     | The column of the diagnostic.           | scalar                |\n"
             "| offset     | The offset of the diagnostic.           | scalar                |\n"
             "| length     | The length of the diagnostic.           | scalar                |\n"
             "| file_id    | The file id of the diagnostic.          | scalar                |\n"
             "\n"
             "## file\n"
             "\n"
             "```sqf\n"
             "[\n"
             "    file_id,       // scalar\n"
             "    file_name,     // string\n"
             "    file_contents, // string\n"
             "]\n"
             "```\n"
             "\n"
             "A file is a file that is being analyzed. It is represented by an array of the\n"
             "above structure. The fields are as follows:\n"
             "\n"
             "| Field         | Description                    | Type   |\n"
             "|---------------|--------------------------------|--------|\n"
             "| file_id       | The file id of the file.       | scalar |\n"
             "| file_name     | The file name of the file.     | string |\n"
             "| file_contents | The file contents of the file. | string |\n"
             "\n"
             "## ast_node_type\n"
             "\n"
             "```sqf\n"
             "// One of the following:\n"
             "\"ENDOFFILE\"\n"
             "\"INVALID\"\n"
             "\"__TOKEN\"\n"
             "\"NA\"\n"
             "\"STATEMENTS\"\n"
             "\"STATEMENT\"\n"
             "\"IDENT\"\n"
             "\"NUMBER\"\n"
             "\"HEXNUMBER\"\n"
             "\"STRING\"\n"
             "\"BOOLEAN_TRUE\"\n"
             "\"BOOLEAN_FALSE\"\n"
             "\"EXPRESSION_LIST\"\n"
             "\"CODE\"\n"
             "\"ARRAY\"\n"
             "\"ASSIGNMENT\"\n"
             "\"ASSIGNMENT_LOCAL\"\n"
             "\"EXPN\"\n"
             "\"EXP0\"\n"
             "\"EXP1\"\n"
             "\"EXP2\"\n"
             "\"EXP3\"\n"
             "\"EXP4\"\n"
             "\"EXP5\"\n"
             "\"EXP6\"\n"
             "\"EXP7\"\n"
             "\"EXP8\"\n"
             "\"EXP9\"\n"
             "\"EXPU\"\n"
             "\"EXP_GROUP\"\n"
             "```\n"
             "\n"
             "The type of AST node. The AST is a tree representation of the code. It is\n"
             "used to analyze the code. The type of node is represented by one of the above\n"
             "strings.\n"
             "\n"
             "## ast_node\n"
             "\n"
             "```sqf\n"
             "[\n"
             "\n"
             "    reference,     // scalar\n"
             "    line,          // scalar\n"
             "    column,        // scalar\n"
             "    offset,        // scalar\n"
             "    path,          // string\n"
             "    type,          // <ast_node_type>\n"
             "]\n"
             "```\n"
             "\n"
             "An AST node is a node in the AST. It is represented by an array of the above\n"
             "structure.\n"
             "To get the children of an AST node, use the `SLS_fnc_getChildren` function.\n"
             "To get the content of an AST node, use the `SLS_fnc_getContent` function.\n"
             "\n"
             "| Field     | Description                    | Type                            |\n"
             "|-----------|--------------------------------|---------------------------------|\n"
             "| reference | The reference of the AST node. | scalar                          |\n"
             "| line      | The line of the AST node.      | scalar                          |\n"
             "| column    | The column of the AST node.    | scalar                          |\n"
             "| offset    | The offset of the AST node.    | scalar                          |\n"
             "| path      | The path of the AST node.      | string                          |\n"
             "| type      | The type of the AST node.      | [ast_node_type](#ast_node_type) |\n"
             "\n"
             "# Functions\n"
             "\n"
             "The following are the functions that are available to the analyzers.\n"
             "They will be referred in the documentation by writing `SLS_fnc_<function>`\n"
             "where `<function>` is the name of the function.\n"
             "\n"
             "Note that the functions listed here expect the arguments to always be passed in as an array\n"
             "and call directly into native functions.\n"
             "\n"
             "## SLS_fnc_getFile\n"
             "\n"
             "```sqf\n"
             "[\n"
             "    path // string\n"
             "] call SLS_fnc_getFile // -> <file>\n"
             "```\n"
             "\n"
             "Gets the file with the given path.\n"
             "\n"
             "## SLS_fnc_getChildren\n"
             "\n"
             "```sqf\n"
             "[\n"
             "    node // <ast_node>\n"
             "] call SLS_fnc_getChildren // -> [<ast_node>]\n"
             "```\n"
             "\n"
             "Gets the children of the given AST node.\n"
             "\n"
             "## SLS_fnc_getContent\n"
             "\n"
             "```sqf\n"
             "[\n"
             "    node // <ast_node>\n"
             "] call SLS_fnc_getContent // -> string\n"
             "```\n"
             "\n"
             "Gets the content of the given AST node.\n"
             "\n"
             "## SLS_fnc_reportDiagnostic\n"
             "\n"
             "```sqf\n"
             "[\n"
             "    diagnostic // <diagnostic>\n"
             "] call SLS_fnc_reportDiagnostic // -> nil\n"
             "```\n"
             "\n"
             "Reports the given diagnostic to the client.\n";
        f.flush();
    }

    // this is "dangerous" but we never leak the storage type into the runtime, so it should be fine
    auto &s = runtime->storage<storage>();
    s.m_visitor = this;
    s.m_ast_analyzer = &a;
    runtime->register_sqfop(
            sqfop::unary(
                    "lineOf",
                    t_astnode(),
                    "Returns the line of the given AST node.",
                    [](auto &runtime, auto &right) -> sqf::runtime::value {
                        auto d_node = right.data<d_astnode>();
                        return {d_node->node().token.line};
                    }
            ));
    runtime->register_sqfop(
            sqfop::unary(
                    "columnOf",
                    t_astnode(),
                    "Returns the column of the given AST node.",
                    [](auto &runtime, auto &right) -> sqf::runtime::value {
                        auto d_node = right.data<d_astnode>();
                        return {d_node->node().token.column};
                    }
            ));
    runtime->register_sqfop(
            sqfop::unary(
                    "offsetOf",
                    t_astnode(),
                    "Returns the offset of the given AST node.",
                    [](auto &runtime, auto &right) -> sqf::runtime::value {
                        auto d_node = right.data<d_astnode>();
                        return {d_node->node().token.offset};
                    }
            ));
    runtime->register_sqfop(
            sqfop::unary(
                    "contentOf",
                    t_astnode(),
                    "Returns the content of the given AST node.",
                    [](auto &runtime, auto &right) -> sqf::runtime::value {
                        auto d_node = right.data<d_astnode>();
                        return {std::string(d_node->node().token.contents.begin(),
                                            d_node->node().token.contents.end())};
                    }
            ));
    runtime->register_sqfop(
            sqfop::unary(
                    "pathOf",
                    t_astnode(),
                    "Returns the path of the given AST node.",
                    [](auto &runtime, auto &right) -> sqf::runtime::value {
                        auto d_node = right.data<d_astnode>();
                        return {*d_node->node().token.path};
                    }
            ));
    runtime->register_sqfop(
            sqfop::unary(
                    "typeOf",
                    t_astnode(),
                    "Returns the type of the given AST node.",
                    [](auto &runtime, auto &right) -> sqf::runtime::value {
                        auto d_node = right.data<d_astnode>();
                        auto kind = to_string_view(d_node->node().kind);
                        return {std::string(kind.begin(), kind.end())};
                    }
            ));
    runtime->register_sqfop(
            sqfop::unary(
                    "childrenOf",
                    t_astnode(),
                    "Returns the children of the given AST node.",
                    [](auto &runtime, auto &right) -> sqf::runtime::value {
                        auto d_node = right.data<d_astnode>();
                        auto children = d_node->node().children;
                        std::vector<sqf::runtime::value> result;
                        result.reserve(children.size());
                        for (auto &child: children) {
                            auto ast_node = std::make_shared<d_astnode>();
                            ast_node->node(child);
                            result.emplace_back(ast_node);
                        }
                        return {std::move(result)};
                    }
            ));
    runtime->register_sqfop(
            sqfop::unary(
                    "fileOf",
                    t_astnode(),
                    "Returns the file for the AST node.",
                    [](auto &runtime, auto &right) -> sqf::runtime::value {
                        // file is an array [id, path]
                        auto d_node = right.data<d_astnode>();
                        auto& s = runtime.storage<storage>();
                        auto file = s.m_visitor->file_of(*s.m_ast_analyzer);
                        return std::vector<value>{file.id_pk, file.path};
                    }
            ));
    runtime->register_sqfop(
            sqfop::unary(
                    "reportDiagnostic",
                    t_array(),
                    "Reports the given diagnostic to the client.",
                    [](auto &runtime, auto &right) -> sqf::runtime::value {
                        auto array = right.data<d_array, std::vector<sqf::runtime::value>>();
                        auto array_size = array.size();
                        if (array_size == 9) {
                            auto severity_string = array[0].data<d_string, std::string>();
                            auto severity = iequal(severity_string, "error")
                                            ? t_diagnostic::error
                                            : iequal(severity_string, "fatal")
                                              ? t_diagnostic::fatal
                                              : iequal(severity_string, "error")
                                                ? t_diagnostic::error
                                                : iequal(severity_string, "warning")
                                                  ? t_diagnostic::warning
                                                  : iequal(severity_string, "info")
                                                    ? t_diagnostic::info
                                                    : iequal(severity_string, "verbose")
                                                      ? t_diagnostic::verbose
                                                      : iequal(severity_string, "trace")
                                                        ? t_diagnostic::trace
                                                        : t_diagnostic::error;

                            auto error_code = array[1].data<d_string, std::string>();
                            auto content = array[2].data<d_string, std::string>();
                            auto message = array[3].data<d_string, std::string>();
                            auto line = array[4].data<d_scalar, uint64_t>();
                            auto column = array[5].data<d_scalar, uint64_t>();
                            auto offset = array[6].data<d_scalar, uint64_t>();
                            auto length = array[7].data<d_scalar, uint64_t>();
                            auto file_id = array[8].data<d_scalar, uint64_t>();
                            auto &s = runtime.storage<storage>();
                            s.m_visitor->m_diagnostics.push_back(
                                    {
                                            .id_pk = {},
                                            .file_fk = file_id,
                                            .source_file_fk = {},
                                            .line =   line,
                                            .column = column,
                                            .offset = offset,
                                            .length = length,
                                            .severity = severity,
                                            .message = message,
                                    }
                            );
                        }
                        return {};
                    }));
}

void sqfvm::language_server::analysis::sqf_ast::visitors::scripted_visitor::initialize_start_script(
        std::shared_ptr<sqf::runtime::runtime> runtime,
        std::filesystem::path file) {
    if (!exists(file)) {
        auto f = std::ofstream(file);
        f << std::endl;
    }
    auto contents = runtime->fileio().read_file_from_disk(file.string());
    if (!contents) {
        return;
    }
    auto pp_result = runtime->parser_preprocessor().preprocess(
            *runtime,
            contents.value(),
            {file.string(), {}});
    if (!pp_result) {
        return;
    }
    auto parse_result = runtime->parser_sqf().parse(
            *runtime,
            pp_result.value(),
            {file.string(), {}});
    if (!parse_result) {
        return;
    }
    m_start_script = std::move(parse_result);
}

void sqfvm::language_server::analysis::sqf_ast::visitors::scripted_visitor::initialize_enter_script(
        std::shared_ptr<sqf::runtime::runtime> runtime,
        std::filesystem::path file) {
    if (!exists(file)) {
        auto f = std::ofstream(file);
        f << "params [\"_node\", \"_parents\"];" << std::endl;
    }
    auto contents = runtime->fileio().read_file_from_disk(file.string());
    if (!contents) {
        return;
    }
    auto pp_result = runtime->parser_preprocessor().preprocess(
            *runtime,
            contents.value(),
            {file.string(), {}});
    if (!pp_result) {
        return;
    }
    auto parse_result = runtime->parser_sqf().parse(
            *runtime,
            pp_result.value(),
            {file.string(), {}});
    if (!parse_result) {
        return;
    }
    m_start_script = std::move(parse_result);
}

void sqfvm::language_server::analysis::sqf_ast::visitors::scripted_visitor::initialize_exit_script(
        std::shared_ptr<sqf::runtime::runtime> runtime,
        std::filesystem::path file) {
    if (!exists(file)) {
        auto f = std::ofstream(file);
        f << "params [\"_node\", \"_parents\"];" << std::endl;
    }
    auto contents = runtime->fileio().read_file_from_disk(file.string());
    if (!contents) {
        return;
    }
    auto pp_result = runtime->parser_preprocessor().preprocess(
            *runtime,
            contents.value(),
            {file.string(), {}});
    if (!pp_result) {
        return;
    }
    auto parse_result = runtime->parser_sqf().parse(
            *runtime,
            pp_result.value(),
            {file.string(), {}});
    if (!parse_result) {
        return;
    }
    m_start_script = std::move(parse_result);
}

void sqfvm::language_server::analysis::sqf_ast::visitors::scripted_visitor::initialize_end_script(
        std::shared_ptr<sqf::runtime::runtime> runtime,
        std::filesystem::path file) {
    if (!exists(file)) {
        auto f = std::ofstream(file);
        f << std::endl;
    }
    auto contents = runtime->fileio().read_file_from_disk(file.string());
    if (!contents) {
        return;
    }
    auto pp_result = runtime->parser_preprocessor().preprocess(
            *runtime,
            contents.value(),
            {file.string(), {}});
    if (!pp_result) {
        return;
    }
    auto parse_result = runtime->parser_sqf().parse(
            *runtime,
            pp_result.value(),
            {file.string(), {}});
    if (!parse_result) {
        return;
    }
    m_start_script = std::move(parse_result);
}

void sqfvm::language_server::analysis::sqf_ast::visitors::scripted_visitor::initialize_analyze_script(
        std::shared_ptr<sqf::runtime::runtime> runtime,
        std::filesystem::path file) {
    if (!exists(file)) {
        auto f = std::ofstream(file);
        f << std::endl;
    }
    auto contents = runtime->fileio().read_file_from_disk(file.string());
    if (!contents) {
        return;
    }
    auto pp_result = runtime->parser_preprocessor().preprocess(
            *runtime,
            contents.value(),
            {file.string(), {}});
    if (!pp_result) {
        return;
    }
    auto parse_result = runtime->parser_sqf().parse(
            *runtime,
            pp_result.value(),
            {file.string(), {}});
    if (!parse_result) {
        return;
    }
    m_start_script = std::move(parse_result);
}

void sqfvm::language_server::analysis::sqf_ast::visitors::scripted_visitor::enter(
        sqfvm::language_server::analysis::sqf_ast::sqf_ast_analyzer &a,
        const sqf::parser::sqf::bison::astnode &node,
        const std::vector<const ::sqf::parser::sqf::bison::astnode *> &parent_nodes) {
    auto d_node = std::make_shared<d_astnode>();
    d_node->node(node);
    auto d_parent_nodes = std::make_shared<d_array>();
    for (auto &p: parent_nodes) {
        auto d_p = std::make_shared<d_astnode>();
        d_p->node(*p);
        d_parent_nodes->push_back(d_p);
    }
    call(runtime_of(a), m_enter_script, {d_node, d_parent_nodes});
}

void sqfvm::language_server::analysis::sqf_ast::visitors::scripted_visitor::exit(
        sqfvm::language_server::analysis::sqf_ast::sqf_ast_analyzer &a,
        const sqf::parser::sqf::bison::astnode &node,
        const std::vector<const ::sqf::parser::sqf::bison::astnode *> &parent_nodes) {
    auto d_node = std::make_shared<d_astnode>();
    d_node->node(node);
    auto d_parent_nodes = std::make_shared<d_array>();
    for (auto &p: parent_nodes) {
        auto d_p = std::make_shared<d_astnode>();
        d_p->node(*p);
        d_parent_nodes->push_back(d_p);
    }
    call(runtime_of(a), m_exit_script, {d_node, d_parent_nodes});
}

void sqfvm::language_server::analysis::sqf_ast::visitors::scripted_visitor::end(
        sqfvm::language_server::analysis::sqf_ast::sqf_ast_analyzer &a) {
    call(runtime_of(a), m_exit_script, {});
}

void sqfvm::language_server::analysis::sqf_ast::visitors::scripted_visitor::analyze(
        sqfvm::language_server::analysis::sqf_ast::sqf_ast_analyzer &sqf_ast_analyzer,
        const sqfvm::language_server::database::context &context) {
    ast_visitor::analyze(sqf_ast_analyzer, context);
    call(runtime_of(sqf_ast_analyzer), m_analyze_script, {});
}

void sqfvm::language_server::analysis::sqf_ast::visitors::scripted_visitor::call(
        std::shared_ptr<sqf::runtime::runtime> runtime,
        std::optional<::sqf::runtime::instruction_set> &instruction_set,
        std::vector<value> this_values) {
    if (!instruction_set.has_value())
        return;
    auto &is = instruction_set.value();
    auto context_weak = runtime->context_create();
    auto context = context_weak.lock();
    if (!context)
        return;
    sqf::runtime::frame f(runtime->default_value_scope(), is);
    f["_this"] = {this_values};
    context->push_frame(f);
    auto result = runtime->execute(sqf::runtime::runtime::action::start);
    if (result != sqf::runtime::runtime::result::ok)
        runtime->context_remove(context);
}
