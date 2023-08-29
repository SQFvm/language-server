<!-- TOC -->
* [Welcome to scripted analyzers](#welcome-to-scripted-analyzers)
* [What are scripted analyzers?](#what-are-scripted-analyzers)
* [Data structures, types and enums](#data-structures-types-and-enums)
  * [Enum: `SEVERITY`](#enum-severity)
  * [Enum: `CODEACTIONKIND`](#enum-codeactionkind)
  * [Enum: `CODEACTIONCHANGEKIND`](#enum-codeactionchangekind)
  * [Array: `CODEACTION`](#array-codeaction)
  * [Array: `CODEACTIONCHANGE` (`CHANGE`)](#array-codeactionchange-change)
  * [Array: `CODEACTIONCHANGE` (`CREATE`)](#array-codeactionchange-create)
  * [Array: `CODEACTIONCHANGE` (`DELETE`)](#array-codeactionchange-delete)
  * [Array: `CODEACTIONCHANGE` (`RENAME`)](#array-codeactionchange-rename)
  * [Array: `DIAGNOSTIC`](#array-diagnostic)
  * [Array: `file`](#array-file)
  * [Enum: `ASTNODETYPE`](#enum-astnodetype)
  * [Type: `ASTNODE`](#type-astnode)
* [New operators](#new-operators)
  * [Operator: `lineOf`](#operator-lineof)
  * [Operator: `columnOf`](#operator-columnof)
  * [Operator: `offsetOf`](#operator-offsetof)
  * [Operator: `contentOf`](#operator-contentof)
  * [Operator: `pathOf`](#operator-pathof)
  * [Operator: `typeOf`](#operator-typeof)
  * [Operator: `childrenOf`](#operator-childrenof)
  * [Operator: `fileOf`](#operator-fileof)
  * [Operator: `reportDiagnostic`](#operator-reportdiagnostic)
  * [Operator: `reportCodeAction`](#operator-reportcodeaction)
<!-- TOC -->

# Welcome to scripted analyzers

This is a short introduction to scripted analyzers.
It will cover the basics of how to write your own analyzer and how to use it.

# What are scripted analyzers?

Scripted analyzers are a way to extend the functionality of the language server
by writing your own analyzers in SQF. This allows you to write e.g. your own
diagnostics. The analyzers are run on the server and the results are sent to
the client, which will display them in the editor.

# Data structures, types and enums

The following are the data structures, types and enums that are available to the analyzers.
They will be referred in the documentation by writing `<type>` where type is
the name of the data structure.

## Enum: `SEVERITY`

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

## Enum: `CODEACTIONKIND`

```sqf
// One of the following:
"GENERIC"
"QUICK_FIX"
"REFACTOR"
"EXTRACT_REFACTOR"
"INLINE_REFACTOR"
"REWRITE_REFACTOR"
"WHOLE_FILE"
```

The kind of [Array: `CODEACTION`](#array-codeaction). The kind groups the different code actions
into categories.

| Kind             | Description                                                                                                                   |
|------------------|-------------------------------------------------------------------------------------------------------------------------------|
| GENERIC          | A generic code action. Use only when the code action does not fit into any of the other categories.                           |
| QUICK_FIX        | A quick fix code action.                                                                                                      |
| REFACTOR         | A refactoring code action. Use only when the code action is a refactoring, but does not fit into any of the other categories. |
| EXTRACT_REFACTOR | Extracting modification of code to eg. extract a method out of a block of code.                                               |
| INLINE_REFACTOR  | Inline eg. a function call.                                                                                                   |
| REWRITE_REFACTOR | Modifications which change how something is written, eg. adding/removing parameters, rewriting array access to variables, ... |
| WHOLE_FILE       | Modifications span entire files.                                                                                              |

## Enum: `CODEACTIONCHANGEKIND`

```sqf
// One of the following:
"CHANGE"
"CREATE"
"DELETE"
"RENAME"
```

The kind of `CODEACTIONCHANGE`. The kind describes what kind of change
the code action will perform. Each kind has a different set of fields.

## Array: `CODEACTION`

```sqf
[
    identifier,   // string
    kind,         // CODEACTIONKIND
]
```

A code action is a modification that can be performed on the code. It is represented by an array
of the above structure. The fields are as follows:

| Field      | Description                                                                                                   | Type                                   |
|------------|---------------------------------------------------------------------------------------------------------------|----------------------------------------|
| identifier | An identifier for the code action with the purpose of grouping same code actions together for bulk execution. | string                                 |
| kind       | The kind of the code action.                                                                                  | [CODEACTIONKIND](#enum-codeactionkind) |

## Array: `CODEACTIONCHANGE` (`CHANGE`)

```sqf
[
    "CHANGE",
    path,         // string
    start_line,   // scalar
    start_column, // scalar
    end_line,     // scalar
    end_column,   // scalar
    content,      // string
]
```

A change code action is a modification that changes the code. It is represented by an array
of the above structure. Note that the first element of the array is
always [Enum: `"CHANGE"`](#enum-codeactionchangekind).
The fields are as follows:

| Field        | Description                             | Type                                                       |
|--------------|-----------------------------------------|------------------------------------------------------------|
| "CHANGE"     | The kind of the code action.            | [Enum: `CODEACTIONCHANGEKIND`](#enum-codeactionchangekind) |
| path         | The path of the file to change.         | string                                                     |
| start_line   | The line of the start of the change.    | scalar                                                     |
| start_column | The column of the start of the change.  | scalar                                                     |
| end_line     | The line of the end of the change.      | scalar                                                     |
| end_column   | The column of the end of the change.    | scalar                                                     |
| content      | The content to replace the change with. | string                                                     |

## Array: `CODEACTIONCHANGE` (`CREATE`)

```sqf
[
    "CREATE",
    path,         // string
    content,      // string
]
```

A create code action is a modification that creates a new file. It is represented by an array
of the above structure. Note that the first element of the array is
always [Enum: `"CREATE"`](#enum-codeactionchangekind).
The fields are as follows:

| Field    | Description                        | Type                                                       |
|----------|------------------------------------|------------------------------------------------------------|
| "CREATE" | The kind of the code action.       | [Enum: `CODEACTIONCHANGEKIND`](#enum-codeactionchangekind) |
| path     | The path of the file to create.    | string                                                     |
| content  | The content of the file to create. | string                                                     |

## Array: `CODEACTIONCHANGE` (`DELETE`)

```sqf
[
    "DELETE",
    path,         // string
]
```

A delete code action is a modification that deletes a file. It is represented by an array
of the above structure. Note that the first element of the array is
always [Enum: `"DELETE"`](#enum-codeactionchangekind).
The fields are as follows:

| Field    | Description                     | Type                                                       |
|----------|---------------------------------|------------------------------------------------------------|
| "DELETE" | The kind of the code action.    | [Enum: `CODEACTIONCHANGEKIND`](#enum-codeactionchangekind) |
| path     | The path of the file to delete. | string                                                     |

## Array: `CODEACTIONCHANGE` (`RENAME`)

```sqf
[
    "RENAME",
    path,         // string
    new_path,     // string
]
```

A rename code action is a modification that renames a file. It is represented by an array
of the above structure. Note that the first element of the array is
always [Enum: `"RENAME"`](#enum-codeactionchangekind).
The fields are as follows:

| Field    | Description                     | Type                                                       |
|----------|---------------------------------|------------------------------------------------------------|
| "RENAME" | The kind of the code action.    | [Enum: `CODEACTIONCHANGEKIND`](#enum-codeactionchangekind) |
| path     | The path of the file to rename. | string                                                     |
| new_path | The new path of the file.       | string                                                     |

## Array: `DIAGNOSTIC`

```sqf
[
    severity,      // SEVERITY
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

| Field      | Description                             | Type                       |
|------------|-----------------------------------------|----------------------------|
| severity   | The severity of the diagnostic.         | [SEVERITY](#enum-severity) |
| error_code | A unique identifier for the diagnostic. | string                     |
| content    | The content of the diagnostic.          | string                     |
| message    | The message of the diagnostic.          | string                     |
| line       | The line of the diagnostic.             | scalar                     |
| column     | The column of the diagnostic.           | scalar                     |
| offset     | The offset of the diagnostic.           | scalar                     |
| length     | The length of the diagnostic.           | scalar                     |
| file_id    | The file id of the diagnostic.          | scalar                     |

## Array: `file`

```sqf
[
    file_id,       // scalar
    file_name      // string
]
```

A file is a file that is being analyzed. It is represented by an array of the
above structure. The fields are as follows:

| Field     | Description                | Type   |
|-----------|----------------------------|--------|
| file_id   | The file id of the file.   | scalar |
| file_name | The file name of the file. | string |

## Enum: `ASTNODETYPE`

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

## Type: `ASTNODE`

A new type introduced to allow introspection of the AST for SQF.

## Array: `HOVER`

```sqf
[
    start_line,   // scalar
    start_column, // scalar
    end_line,     // scalar
    end_column,   // scalar
    markdown      // string
]
```

A hover is a piece of text that is displayed when the user hovers over a piece
of code. It is represented by an array of the above structure. The fields are
as follows:

| Field        | Description                             | Type   |
|--------------|-----------------------------------------|--------|
| start_line   | The line of the start of the hover.     | scalar |
| start_column | The column of the start of the hover.   | scalar |
| end_line     | The line of the end of the hover.       | scalar |
| end_column   | The column of the end of the hover.     | scalar |
| markdown     | The markdown of the hover.              | string |


# New operators

The following are the operators that are available to the analyzers.

## Operator: `lineOf`

```sqf
lineOf ASTNODE
```

Returns the line of the given AST node ([Type: `ASTNODE`](#type-astnode)).

## Operator: `columnOf`

```sqf
columnOf ASTNODE
```

Returns the column of the given AST node ([Type: `ASTNODE`](#type-astnode)).

## Operator: `offsetOf`

```sqf
offsetOf ASTNODE
```

Returns the offset of the given AST node ([Type: `ASTNODE`](#type-astnode)).

## Operator: `contentOf`

```sqf
contentOf ASTNODE
```

Returns the content of the given AST node ([Type: `ASTNODE`](#type-astnode)).

## Operator: `pathOf`

```sqf
pathOf ASTNODE
```

Returns the path of the given AST node ([Type: `ASTNODE`](#type-astnode)).

## Operator: `typeOf`

```sqf
typeOf ASTNODE
```

Returns the [Enum: `ASTNODETYPE`](#enum-astnodetype) of the given AST node ([Type: `ASTNODE`](#type-astnode)).

## Operator: `childrenOf`

```sqf
childrenOf ASTNODE
```

Returns the children (`[ASTNODE]`) of the given AST node.

## Operator: `fileOf`

```sqf
fileOf ASTNODE
```

Returns the [Array: `file`](#array-file) of the given AST node ([Type: `ASTNODE`](#type-astnode)).

## Operator: `reportDiagnostic`

```sqf
reportDiagnostic DIAGNOSTIC
```

Reports the given [Array: `DIAGNOSTIC`](#array-diagnostic) to the client.

## Operator: `reportCodeAction`

```sqf
CODEACTION reportCodeAction [CODEACTIONCHANGE]
```

Reports the given [Array: `CODEACTION`](#array-codeaction) to the client,
containing the given `CODEACTIONCHANGE`s.
The `CODEACTIONCHANGE`s are one of their corresponding subtypes:

- [Array: `CODEACTIONCHANGE` (`CHANGE`)](#array-codeactionchange-change)
- [Array: `CODEACTIONCHANGE` (`RENAME`)](#array-codeactionchange-rename)
- [Array: `CODEACTIONCHANGE` (`CREATE`)](#array-codeactionchange-create)
- [Array: `CODEACTIONCHANGE` (`DELETE`)](#array-codeactionchange-delete)

## Operator: `reportHover`

```sqf
reportHover HOVER
```

Reports the given [Array: `HOVER`](#array-hover) to the client.