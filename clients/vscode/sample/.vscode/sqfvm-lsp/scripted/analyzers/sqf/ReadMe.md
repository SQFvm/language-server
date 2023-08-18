# Welcome to scripted analyzers

This is a short introduction to scripted analyzers.
It will cover the basics of how to write your own analyzer and how to use it.

# What are scripted analyzers?

Scripted analyzers are a way to extend the functionality of the language server
by writing your own analyzers in SQF. This allows you to write e.g. your own
diagnostics. The analyzers are run on the server and the results are sent to
the client, which will display them in the editor.

# Data structures

The following are the data structures that are available to the analyzers.
They will be referred in the documentation by writing `<type>` where type is
the name of the data structure.

## severity

```sqf
// One of the following:
"FATAL";
"ERROR";
"WARNING";
"INFO";
"VERBOSE";
"TRACE";
```

The severity of a diagnostic. The higher the severity, the more important the
diagnostic is.
FATAL is the highest severity and TRACE is the lowest.

| Severity | Description                                                                                                                       | Problems | Editor |
|----------|-----------------------------------------------------------------------------------------------------------------------------------|----------|--------|
| FATAL    | The highest severity, should be used sparingly.                                                                                   | ERROR    | RED    |
| ERROR    | The second-highest severity, should be used for problems that prevent the code from running.                                      | ERROR    | RED    |
| WARNING  | The third-highest severity, should be used for problems that might cause unexpected behavior.                                     | WARNING  | YELLOW |
| INFO     | The fourth-highest severity, should be used for problems that are not necessarily problems, but might be interesting to the user. | INFO     | WHITE  |
| VERBOSE  | The second-lowest severity, should be used for when INFO is too noisy.                                                            |          | GRAY   |
| TRACE    | The lowest severity.                                                                                                              |          | GRAY   |

## diagnostic

```sqf
[
    severity,      // <severity>
    error_code,    // string
    content,       // string
    message,       // string
    line,          // scalar
    column,        // scalar
    offset,        // scalar
    length,        // scalar
    file_id,       // scalar
]
```

A diagnostic is a problem that is found in the code. It is represented by an
array of the above structure. The fields are as follows:

| Field      | Description                             | Type                  |
|------------|-----------------------------------------|-----------------------|
| severity   | The severity of the diagnostic.         | [severity](#severity) |
| error_code | A unique identifier for the diagnostic. | string                |
| content    | The content of the diagnostic.          | string                |
| message    | The message of the diagnostic.          | string                |
| line       | The line of the diagnostic.             | scalar                |
| column     | The column of the diagnostic.           | scalar                |
| offset     | The offset of the diagnostic.           | scalar                |
| length     | The length of the diagnostic.           | scalar                |
| file_id    | The file id of the diagnostic.          | scalar                |

## file

```sqf
[
    file_id,       // scalar
    file_name,     // string
    file_contents, // string
]
```

A file is a file that is being analyzed. It is represented by an array of the
above structure. The fields are as follows:

| Field         | Description                    | Type   |
|---------------|--------------------------------|--------|
| file_id       | The file id of the file.       | scalar |
| file_name     | The file name of the file.     | string |
| file_contents | The file contents of the file. | string |

## ast_node_type

```sqf
// One of the following:
"ENDOFFILE"
"INVALID"
"__TOKEN"
"NA"
"STATEMENTS"
"STATEMENT"
"IDENT"
"NUMBER"
"HEXNUMBER"
"STRING"
"BOOLEAN_TRUE"
"BOOLEAN_FALSE"
"EXPRESSION_LIST"
"CODE"
"ARRAY"
"ASSIGNMENT"
"ASSIGNMENT_LOCAL"
"EXPN"
"EXP0"
"EXP1"
"EXP2"
"EXP3"
"EXP4"
"EXP5"
"EXP6"
"EXP7"
"EXP8"
"EXP9"
"EXPU"
"EXP_GROUP"
```

The type of AST node. The AST is a tree representation of the code. It is
used to analyze the code. The type of node is represented by one of the above
strings.

## ast_node

```sqf
[

    reference,     // scalar
    line,          // scalar
    column,        // scalar
    offset,        // scalar
    path,          // string
    type,          // <ast_node_type>
]
```

An AST node is a node in the AST. It is represented by an array of the above
structure.
To get the children of an AST node, use the `SLS_fnc_getChildren` function.
To get the content of an AST node, use the `SLS_fnc_getContent` function.

| Field     | Description                    | Type                            |
|-----------|--------------------------------|---------------------------------|
| reference | The reference of the AST node. | scalar                          |
| line      | The line of the AST node.      | scalar                          |
| column    | The column of the AST node.    | scalar                          |
| offset    | The offset of the AST node.    | scalar                          |
| path      | The path of the AST node.      | string                          |
| type      | The type of the AST node.      | [ast_node_type](#ast_node_type) |

# Functions

The following are the functions that are available to the analyzers.
They will be referred in the documentation by writing `SLS_fnc_<function>`
where `<function>` is the name of the function.

Note that the functions listed here expect the arguments to always be passed in as an array
and call directly into native functions.

## SLS_fnc_getFile

```sqf
[
    path // string
] call SLS_fnc_getFile // -> <file>
```

Gets the file with the given path.

## SLS_fnc_getChildren

```sqf
[
    node // <ast_node>
] call SLS_fnc_getChildren // -> [<ast_node>]
```

Gets the children of the given AST node.

## SLS_fnc_getContent

```sqf
[
    node // <ast_node>
] call SLS_fnc_getContent // -> string
```

Gets the content of the given AST node.

## SLS_fnc_reportDiagnostic

```sqf
[
    diagnostic // <diagnostic>
] call SLS_fnc_reportDiagnostic // -> nil
```

Reports the given diagnostic to the client.
