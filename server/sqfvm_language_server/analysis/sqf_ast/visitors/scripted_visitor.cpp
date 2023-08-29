//
// Created by marco.silipo on 17.08.2023.
//

#include "scripted_visitor.hpp"
#include <fstream>
#include "runtime/d_array.h"
#include "runtime/d_string.h"
#include "runtime/d_scalar.h"

namespace err = logmessage::runtime;
using namespace ::sqf::runtime;
using namespace ::sqf::types;
using namespace ::sqfvm::language_server::database::tables;

struct storage : public runtime::datastorage {
    sqfvm::language_server::analysis::sqf_ast::visitors::scripted_visitor *m_visitor;
    sqfvm::language_server::analysis::sqf_ast::sqf_ast_analyzer *m_ast_analyzer;
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
        f << "<!-- TOC -->\n"
             "* [Welcome to scripted analyzers](#welcome-to-scripted-analyzers)\n"
             "* [What are scripted analyzers?](#what-are-scripted-analyzers)\n"
             "* [Data structures, types and enums](#data-structures-types-and-enums)\n"
             "  * [Enum: `SEVERITY`](#enum-severity)\n"
             "  * [Enum: `CODEACTIONKIND`](#enum-codeactionkind)\n"
             "  * [Enum: `CODEACTIONCHANGEKIND`](#enum-codeactionchangekind)\n"
             "  * [Array: `CODEACTION`](#array-codeaction)\n"
             "  * [Array: `CODEACTIONCHANGE` (`CHANGE`)](#array-codeactionchange-change)\n"
             "  * [Array: `CODEACTIONCHANGE` (`CREATE`)](#array-codeactionchange-create)\n"
             "  * [Array: `CODEACTIONCHANGE` (`DELETE`)](#array-codeactionchange-delete)\n"
             "  * [Array: `CODEACTIONCHANGE` (`RENAME`)](#array-codeactionchange-rename)\n"
             "  * [Array: `DIAGNOSTIC`](#array-diagnostic)\n"
             "  * [Array: `file`](#array-file)\n"
             "  * [Enum: `ASTNODETYPE`](#enum-astnodetype)\n"
             "  * [Type: `ASTNODE`](#type-astnode)\n"
             "* [New operators](#new-operators)\n"
             "  * [Operator: `lineOf`](#operator-lineof)\n"
             "  * [Operator: `columnOf`](#operator-columnof)\n"
             "  * [Operator: `offsetOf`](#operator-offsetof)\n"
             "  * [Operator: `contentOf`](#operator-contentof)\n"
             "  * [Operator: `pathOf`](#operator-pathof)\n"
             "  * [Operator: `typeOf`](#operator-typeof)\n"
             "  * [Operator: `childrenOf`](#operator-childrenof)\n"
             "  * [Operator: `fileOf`](#operator-fileof)\n"
             "  * [Operator: `reportDiagnostic`](#operator-reportdiagnostic)\n"
             "  * [Operator: `reportCodeAction`](#operator-reportcodeaction)\n"
             "<!-- TOC -->\n"
             "\n"
             "# Welcome to scripted analyzers\n"
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
             "# Data structures, types and enums\n"
             "\n"
             "The following are the data structures, types and enums that are available to the analyzers.\n"
             "They will be referred in the documentation by writing `<type>` where type is\n"
             "the name of the data structure.\n"
             "\n"
             "## Enum: `SEVERITY`\n"
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
             "## Enum: `CODEACTIONKIND`\n"
             "\n"
             "```sqf\n"
             "// One of the following:\n"
             "\"GENERIC\"\n"
             "\"QUICK_FIX\"\n"
             "\"REFACTOR\"\n"
             "\"EXTRACT_REFACTOR\"\n"
             "\"INLINE_REFACTOR\"\n"
             "\"REWRITE_REFACTOR\"\n"
             "\"WHOLE_FILE\"\n"
             "```\n"
             "\n"
             "The kind of [Array: `CODEACTION`](#array-codeaction). The kind groups the different code actions\n"
             "into categories.\n"
             "\n"
             "| Kind             | Description                                                                                                                   |\n"
             "|------------------|-------------------------------------------------------------------------------------------------------------------------------|\n"
             "| GENERIC          | A generic code action. Use only when the code action does not fit into any of the other categories.                           |\n"
             "| QUICK_FIX        | A quick fix code action.                                                                                                      |\n"
             "| REFACTOR         | A refactoring code action. Use only when the code action is a refactoring, but does not fit into any of the other categories. |\n"
             "| EXTRACT_REFACTOR | Extracting modification of code to eg. extract a method out of a block of code.                                               |\n"
             "| INLINE_REFACTOR  | Inline eg. a function call.                                                                                                   |\n"
             "| REWRITE_REFACTOR | Modifications which change how something is written, eg. adding/removing parameters, rewriting array access to variables, ... |\n"
             "| WHOLE_FILE       | Modifications span entire files.                                                                                              |\n"
             "\n"
             "## Enum: `CODEACTIONCHANGEKIND`\n"
             "\n"
             "```sqf\n"
             "// One of the following:\n"
             "\"CHANGE\"\n"
             "\"CREATE\"\n"
             "\"DELETE\"\n"
             "\"RENAME\"\n"
             "```\n"
             "\n"
             "The kind of `CODEACTIONCHANGE`. The kind describes what kind of change\n"
             "the code action will perform. Each kind has a different set of fields.\n"
             "\n"
             "## Array: `CODEACTION`\n"
             "\n"
             "```sqf\n"
             "[\n"
             "    identifier,   // string\n"
             "    kind,         // CODEACTIONKIND\n"
             "]\n"
             "```\n"
             "\n"
             "A code action is a modification that can be performed on the code. It is represented by an array\n"
             "of the above structure. The fields are as follows:\n"
             "\n"
             "| Field      | Description                                                                                                   | Type                                   |\n"
             "|------------|---------------------------------------------------------------------------------------------------------------|----------------------------------------|\n"
             "| identifier | An identifier for the code action with the purpose of grouping same code actions together for bulk execution. | string                                 |\n"
             "| kind       | The kind of the code action.                                                                                  | [CODEACTIONKIND](#enum-codeactionkind) |\n"
             "\n"
             "## Array: `CODEACTIONCHANGE` (`CHANGE`)\n"
             "\n"
             "```sqf\n"
             "[\n"
             "    \"CHANGE\",\n"
             "    path,         // string\n"
             "    start_line,   // scalar\n"
             "    start_column, // scalar\n"
             "    end_line,     // scalar\n"
             "    end_column,   // scalar\n"
             "    content,      // string\n"
             "]\n"
             "```\n"
             "\n"
             "A change code action is a modification that changes the code. It is represented by an array\n"
             "of the above structure. Note that the first element of the array is\n"
             "always [Enum: `\"CHANGE\"`](#enum-codeactionchangekind).\n"
             "The fields are as follows:\n"
             "\n"
             "| Field        | Description                             | Type                                                       |\n"
             "|--------------|-----------------------------------------|------------------------------------------------------------|\n"
             "| \"CHANGE\"     | The kind of the code action.            | [Enum: `CODEACTIONCHANGEKIND`](#enum-codeactionchangekind) |\n"
             "| path         | The path of the file to change.         | string                                                     |\n"
             "| start_line   | The line of the start of the change.    | scalar                                                     |\n"
             "| start_column | The column of the start of the change.  | scalar                                                     |\n"
             "| end_line     | The line of the end of the change.      | scalar                                                     |\n"
             "| end_column   | The column of the end of the change.    | scalar                                                     |\n"
             "| content      | The content to replace the change with. | string                                                     |\n"
             "\n"
             "## Array: `CODEACTIONCHANGE` (`CREATE`)\n"
             "\n"
             "```sqf\n"
             "[\n"
             "    \"CREATE\",\n"
             "    path,         // string\n"
             "    content,      // string\n"
             "]\n"
             "```\n"
             "\n"
             "A create code action is a modification that creates a new file. It is represented by an array\n"
             "of the above structure. Note that the first element of the array is\n"
             "always [Enum: `\"CREATE\"`](#enum-codeactionchangekind).\n"
             "The fields are as follows:\n"
             "\n"
             "| Field    | Description                        | Type                                                       |\n"
             "|----------|------------------------------------|------------------------------------------------------------|\n"
             "| \"CREATE\" | The kind of the code action.       | [Enum: `CODEACTIONCHANGEKIND`](#enum-codeactionchangekind) |\n"
             "| path     | The path of the file to create.    | string                                                     |\n"
             "| content  | The content of the file to create. | string                                                     |\n"
             "\n"
             "## Array: `CODEACTIONCHANGE` (`DELETE`)\n"
             "\n"
             "```sqf\n"
             "[\n"
             "    \"DELETE\",\n"
             "    path,         // string\n"
             "]\n"
             "```\n"
             "\n"
             "A delete code action is a modification that deletes a file. It is represented by an array\n"
             "of the above structure. Note that the first element of the array is\n"
             "always [Enum: `\"DELETE\"`](#enum-codeactionchangekind).\n"
             "The fields are as follows:\n"
             "\n"
             "| Field    | Description                     | Type                                                       |\n"
             "|----------|---------------------------------|------------------------------------------------------------|\n"
             "| \"DELETE\" | The kind of the code action.    | [Enum: `CODEACTIONCHANGEKIND`](#enum-codeactionchangekind) |\n"
             "| path     | The path of the file to delete. | string                                                     |\n"
             "\n"
             "## Array: `CODEACTIONCHANGE` (`RENAME`)\n"
             "\n"
             "```sqf\n"
             "[\n"
             "    \"RENAME\",\n"
             "    path,         // string\n"
             "    new_path,     // string\n"
             "]\n"
             "```\n"
             "\n"
             "A rename code action is a modification that renames a file. It is represented by an array\n"
             "of the above structure. Note that the first element of the array is\n"
             "always [Enum: `\"RENAME\"`](#enum-codeactionchangekind).\n"
             "The fields are as follows:\n"
             "\n"
             "| Field    | Description                     | Type                                                       |\n"
             "|----------|---------------------------------|------------------------------------------------------------|\n"
             "| \"RENAME\" | The kind of the code action.    | [Enum: `CODEACTIONCHANGEKIND`](#enum-codeactionchangekind) |\n"
             "| path     | The path of the file to rename. | string                                                     |\n"
             "| new_path | The new path of the file.       | string                                                     |\n"
             "\n"
             "## Array: `DIAGNOSTIC`\n"
             "\n"
             "```sqf\n"
             "[\n"
             "    severity,      // SEVERITY\n"
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
             "| Field      | Description                             | Type                       |\n"
             "|------------|-----------------------------------------|----------------------------|\n"
             "| severity   | The severity of the diagnostic.         | [SEVERITY](#enum-severity) |\n"
             "| error_code | A unique identifier for the diagnostic. | string                     |\n"
             "| content    | The content of the diagnostic.          | string                     |\n"
             "| message    | The message of the diagnostic.          | string                     |\n"
             "| line       | The line of the diagnostic.             | scalar                     |\n"
             "| column     | The column of the diagnostic.           | scalar                     |\n"
             "| offset     | The offset of the diagnostic.           | scalar                     |\n"
             "| length     | The length of the diagnostic.           | scalar                     |\n"
             "| file_id    | The file id of the diagnostic.          | scalar                     |\n"
             "\n"
             "## Array: `file`\n"
             "\n"
             "```sqf\n"
             "[\n"
             "    file_id,       // scalar\n"
             "    file_name      // string\n"
             "]\n"
             "```\n"
             "\n"
             "A file is a file that is being analyzed. It is represented by an array of the\n"
             "above structure. The fields are as follows:\n"
             "\n"
             "| Field     | Description                | Type   |\n"
             "|-----------|----------------------------|--------|\n"
             "| file_id   | The file id of the file.   | scalar |\n"
             "| file_name | The file name of the file. | string |\n"
             "\n"
             "## Enum: `ASTNODETYPE`\n"
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
             "## Type: `ASTNODE`\n"
             "\n"
             "A new type introduced to allow introspection of the AST for SQF.\n"
             "\n"
             "## Array: `HOVER`\n"
             "\n"
             "```sqf\n"
             "[\n"
             "    start_line,   // scalar\n"
             "    start_column, // scalar\n"
             "    end_line,     // scalar\n"
             "    end_column,   // scalar\n"
             "    markdown      // string\n"
             "]\n"
             "```\n"
             "\n"
             "A hover is a piece of text that is displayed when the user hovers over a piece\n"
             "of code. It is represented by an array of the above structure. The fields are\n"
             "as follows:\n"
             "\n"
             "| Field        | Description                             | Type   |\n"
             "|--------------|-----------------------------------------|--------|\n"
             "| start_line   | The line of the start of the hover.     | scalar |\n"
             "| start_column | The column of the start of the hover.   | scalar |\n"
             "| end_line     | The line of the end of the hover.       | scalar |\n"
             "| end_column   | The column of the end of the hover.     | scalar |\n"
             "| markdown     | The markdown of the hover.              | string |\n"
             "\n"
             "\n"
             "# New operators\n"
             "\n"
             "The following are the operators that are available to the analyzers.\n"
             "\n"
             "## Operator: `lineOf`\n"
             "\n"
             "```sqf\n"
             "lineOf ASTNODE\n"
             "```\n"
             "\n"
             "Returns the line of the given AST node ([Type: `ASTNODE`](#type-astnode)).\n"
             "\n"
             "## Operator: `columnOf`\n"
             "\n"
             "```sqf\n"
             "columnOf ASTNODE\n"
             "```\n"
             "\n"
             "Returns the column of the given AST node ([Type: `ASTNODE`](#type-astnode)).\n"
             "\n"
             "## Operator: `offsetOf`\n"
             "\n"
             "```sqf\n"
             "offsetOf ASTNODE\n"
             "```\n"
             "\n"
             "Returns the offset of the given AST node ([Type: `ASTNODE`](#type-astnode)).\n"
             "\n"
             "## Operator: `contentOf`\n"
             "\n"
             "```sqf\n"
             "contentOf ASTNODE\n"
             "```\n"
             "\n"
             "Returns the content of the given AST node ([Type: `ASTNODE`](#type-astnode)).\n"
             "\n"
             "## Operator: `pathOf`\n"
             "\n"
             "```sqf\n"
             "pathOf ASTNODE\n"
             "```\n"
             "\n"
             "Returns the path of the given AST node ([Type: `ASTNODE`](#type-astnode)).\n"
             "\n"
             "## Operator: `typeOf`\n"
             "\n"
             "```sqf\n"
             "typeOf ASTNODE\n"
             "```\n"
             "\n"
             "Returns the [Enum: `ASTNODETYPE`](#enum-astnodetype) of the given AST node ([Type: `ASTNODE`](#type-astnode)).\n"
             "\n"
             "## Operator: `childrenOf`\n"
             "\n"
             "```sqf\n"
             "childrenOf ASTNODE\n"
             "```\n"
             "\n"
             "Returns the children (`[ASTNODE]`) of the given AST node.\n"
             "\n"
             "## Operator: `fileOf`\n"
             "\n"
             "```sqf\n"
             "fileOf ASTNODE\n"
             "```\n"
             "\n"
             "Returns the [Array: `file`](#array-file) of the given AST node ([Type: `ASTNODE`](#type-astnode)).\n"
             "\n"
             "## Operator: `reportDiagnostic`\n"
             "\n"
             "```sqf\n"
             "reportDiagnostic DIAGNOSTIC\n"
             "```\n"
             "\n"
             "Reports the given [Array: `DIAGNOSTIC`](#array-diagnostic) to the client.\n"
             "\n"
             "## Operator: `reportCodeAction`\n"
             "\n"
             "```sqf\n"
             "CODEACTION reportCodeAction [CODEACTIONCHANGE]\n"
             "```\n"
             "\n"
             "Reports the given [Array: `CODEACTION`](#array-codeaction) to the client,\n"
             "containing the given `CODEACTIONCHANGE`s.\n"
             "The `CODEACTIONCHANGE`s are one of their corresponding subtypes:\n"
             "\n"
             "- [Array: `CODEACTIONCHANGE` (`CHANGE`)](#array-codeactionchange-change)\n"
             "- [Array: `CODEACTIONCHANGE` (`RENAME`)](#array-codeactionchange-rename)\n"
             "- [Array: `CODEACTIONCHANGE` (`CREATE`)](#array-codeactionchange-create)\n"
             "- [Array: `CODEACTIONCHANGE` (`DELETE`)](#array-codeactionchange-delete)\n"
             "\n"
             "## Operator: `reportHover`\n"
             "\n"
             "```sqf\n"
             "reportHover HOVER\n"
             "```\n"
             "\n"
             "Reports the given [Array: `HOVER`](#array-hover) to the client.\n";
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
                        auto &s = runtime.storage<storage>();
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
                        auto array = right.template data<d_array>();

                        if (!array->check_type(
                                runtime,
                                std::array<sqf::runtime::type, 9>{
                                        t_string(),
                                        t_string(),
                                        t_string(),
                                        t_string(),
                                        t_scalar(),
                                        t_scalar(),
                                        t_scalar(),
                                        t_scalar(),
                                        t_scalar(),
                                }))
                            return {};


                        auto severity_string = array->template data<0, d_string, std::string>();
                        auto error_code = array->template data<1, d_string, std::string>();
                        auto content = array->template data<2, d_string, std::string>();
                        auto message = array->template data<3, d_string, std::string>();
                        auto line = array->template data<4, d_scalar, uint64_t>();
                        auto column = array->template data<5, d_scalar, uint64_t>();
                        auto offset = array->template data<6, d_scalar, uint64_t>();
                        auto length = array->template data<7, d_scalar, uint64_t>();
                        auto file_id = array->template data<8, d_scalar, uint64_t>();


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
                        auto &s = runtime.template storage<storage>();
                        s.m_visitor->m_diagnostics.push_back(
                                {
                                        .id_pk = s.m_visitor->m_diagnostics.size(),
                                        .file_fk = file_id,
                                        .source_file_fk = {},
                                        .line =   line,
                                        .column = column,
                                        .offset = offset,
                                        .length = length,
                                        .severity = severity,
                                        .message = message,
                                        .code = error_code
                                }
                        );
                        return {};
                    }));
    runtime->register_sqfop(
            sqfop::binary(
                    4,
                    "reportCodeAction",
                    t_array(),
                    t_array(),
                    "Reports the given code action to the client.",
                    [](auto &runtime, auto &left, auto &right) -> sqf::runtime::value {
                        auto code_action_array = left.template data<d_array>();
                        if (!code_action_array->check_type(
                                runtime,
                                std::array<sqf::runtime::type, 2>{t_string(), t_string()}))
                            return {};
                        std::vector<t_code_action_change> changes;
                        auto code_action_identifier = code_action_array->template data<0, d_string, std::string>();
                        auto code_action_kind_string = code_action_array->template data<1, d_string, std::string>();
                        auto right_array = right.template data<d_array, std::vector<sqf::runtime::value>>();
                        bool flag = false;
                        for (size_t i = 0; i < right_array.size(); i++) {
                            auto &it = right_array.at(i);
                            if (!it.template is<t_array>()) {
                                runtime.__logmsg(
                                        err::ExpectedSubArrayTypeMissmatch(
                                                runtime.context_active().current_frame().diag_info_from_position(),
                                                std::array<size_t, 1>{i},
                                                t_array(),
                                                it.type()));
                                flag = true;
                                continue;
                            }
                            auto it_array = it.template data<d_array>();

                            if (it_array->size() < 1)
                                continue;
                            auto code_action_change_operation = it_array->template data<0, d_string, std::string>();
                            if (iequal(code_action_change_operation, "change")) {
                                if (!it_array->check_type(
                                        runtime,
                                        std::array<sqf::runtime::type, 7>{
                                                t_string(),
                                                t_string(),
                                                t_scalar(),
                                                t_scalar(),
                                                t_scalar(),
                                                t_scalar(),
                                                t_string()
                                        })) {
                                    flag = true;
                                    continue;
                                }
                                auto change = t_code_action_change{
                                        .operation = t_code_action_change::file_change,
                                        .path = it_array->template data<1, d_string, std::string>(),
                                        .start_line = it_array->template data<2, d_scalar, uint64_t>(),
                                        .start_column = it_array->template data<3, d_scalar, uint64_t>(),
                                        .end_line = it_array->template data<4, d_scalar, uint64_t>(),
                                        .end_column = it_array->template data<5, d_scalar, uint64_t>(),
                                        .content = it_array->template data<6, d_string, std::string>()
                                };
                                changes.push_back(change);
                            } else if (iequal(code_action_change_operation, "create")) {

                                if (!it_array->check_type(
                                        runtime,
                                        std::array<sqf::runtime::type, 3>{
                                                t_string(),
                                                t_string(),
                                                t_string()})) {
                                    flag = true;
                                    continue;
                                }
                                auto change = t_code_action_change{
                                        .operation = t_code_action_change::file_create,
                                        .path = it_array->template data<1, d_string, std::string>(),
                                        .content = it_array->template data<2, d_string, std::string>()
                                };
                                changes.push_back(change);
                            } else if (iequal(code_action_change_operation, "delete")) {
                                if (!it_array->check_type(
                                        runtime,
                                        std::array<sqf::runtime::type, 2>{
                                                t_string(),
                                                t_string()})) {
                                    flag = true;
                                    continue;
                                }
                                auto change = t_code_action_change{
                                        .operation = t_code_action_change::file_delete,
                                        .path = it_array->template data<1, d_string, std::string>()
                                };
                                changes.push_back(change);
                            } else if (iequal(code_action_change_operation, "rename")) {
                                if (!it_array->check_type(
                                        runtime,
                                        std::array<sqf::runtime::type, 3>{
                                                t_string(),
                                                t_string(),
                                                t_string()})) {
                                    flag = true;
                                    continue;
                                }
                                auto change = t_code_action_change{
                                        .operation = t_code_action_change::file_change,
                                        .old_path = it_array->template data<1, d_string, std::string>(),
                                        .path = it_array->template data<2, d_string, std::string>(),
                                };
                                changes.push_back(change);
                            } else {
                                runtime.__logmsg(
                                        err::ExpectedSubArrayTypeMissmatch(
                                                runtime.context_active().current_frame().diag_info_from_position(),
                                                std::array<size_t, 1>{i},
                                                t_array(),
                                                it.type()));
                                flag = true;
                                continue;
                            }
                        }
                        if (flag)
                            return {};

                        auto code_action_kind = iequal(code_action_kind_string, "generic")
                                                ? t_code_action::generic
                                                : iequal(code_action_kind_string, "quick_fix")
                                                  ? t_code_action::quick_fix
                                                  : iequal(code_action_kind_string, "refactor")
                                                    ? t_code_action::refactor
                                                    : iequal(code_action_kind_string, "extract_refactor")
                                                      ? t_code_action::extract_refactor
                                                      : iequal(code_action_kind_string, "inline_refactor")
                                                        ? t_code_action::inline_refactor
                                                        : iequal(code_action_kind_string, "rewrite_refactor")
                                                          ? t_code_action::rewrite_refactor
                                                          : iequal(code_action_kind_string, "whole_file")
                                                            ? t_code_action::whole_file
                                                            : t_code_action::generic;

                        auto &s = runtime.template storage<storage>();
                        s.m_visitor->m_code_actions.push_back(
                                {
                                        .code_action = t_code_action{
                                                .identifier = code_action_identifier,
                                                .kind = code_action_kind,
                                        },
                                        .changes = changes
                                }
                        );

                        return {};
                    }));
    runtime->register_sqfop(
            sqfop::unary(
                    "reportHover",
                    t_array(),
                    "Reports a hover information to the client.",
                    [](auto &runtime, auto &right) -> sqf::runtime::value {
                        // SQF input: [start_line: scalar, start_column: scalar, end_line: scalar, end_column: scalar, markdown: string]

                        auto array = right.template data<d_array>();
                        if (!array->check_type(
                                runtime,
                                std::array<sqf::runtime::type, 5>{t_scalar(), t_scalar(), t_scalar(), t_scalar(),
                                                                  t_string()}))
                            return {};

                        auto start_line = array->template data<0, d_scalar, uint64_t>();
                        auto start_column = array->template data<1, d_scalar, uint64_t>();
                        auto end_line = array->template data<2, d_scalar, uint64_t>();
                        auto end_column = array->template data<3, d_scalar, uint64_t>();
                        auto markdown = array->template data<4, d_string, std::string>();

                        auto &s = runtime.template storage<storage>();
                        s.m_visitor->m_hovers.push_back(
                                {
                                        .start_line = start_line,
                                        .start_column = start_column,
                                        .end_line = end_line,
                                        .end_column = end_column,
                                        .markdown = markdown
                                }
                        );
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
