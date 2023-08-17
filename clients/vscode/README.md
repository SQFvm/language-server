Provides a Language Server for the Scripting Language SQF, that is used in the `Arma` Series and `Virtual Battle Space` Simulation.


![](https://raw.githubusercontent.com/SQFvm/vscode/master/clients/vscode/assets/readme/variable_not_defined.gif)
![](https://raw.githubusercontent.com/SQFvm/vscode/master/clients/vscode/assets/readme/symbol_lookup.gif)

# Features
* Linting
    * PreProcessor *Note that the preprocessor expects files to exist. Recommended to setup a `$PBOPREFIX$`*
    * Syntax Checking
* Code Analysis
    * Unused variable warning
    * Unused value warning
* Syntax Highlighting
* Reference (Symbol) lookup
* Disable reported problems and informations *Note that disabling syntactical errors will not fix the file*


# Planned Features
* Basic Auto Completion (Macros, Variables, Operators, ...)
* Extended Auto Completion (Read params of methods and automagically complete on call, ...)
* Debugger (SQF-VM powered)
* Resolve macro on hover
* PreProcess file and display it
* Official Linux support (Deployment related lack of, you can compile your own from the sources)

# How to report errors
Head over to https://github.com/SQFvm/vscode/issues and create a new issue.

# How to use
Just install the extension and you should be able to run it from the get-go.

# Social Channels
* [Discord](https://discord.gg/5uQYWQu)

# FAQ
## Enable/Disable a problem
Note: `ERROR-CODE` in the following samples is the code reported by the language server (eg. `VV-001`)

To disable a warning for the line following, use
`#pragma sls disable line ERROR-CODE`
This will surpress the error code for the next line.
To disable (and later enable) an error code on a long span, use
```sqf
#pragma sls disable ERROR-CODE
// [...]
#pragma sls enable ERROR-CODE
```

Note that while this feature is file-scoped.