# What is this?
This is the **SQF-VM** *Language Server* implementation, aimed to provide full SQF support
for IDEs and editors like `vscode`

# How to install?
ToDo

# How to use?
ToDo

# Development

## Quick Start Guide
0.  Clone this repository using `git clone REPOSITORY --recursive`
1.  Download NodeJS
2.  Download Visual Studio Code
3.  Download Visual Studio Community 2019 (or later) on Windows or vcpkg on Linux
4.  Download CMake
5.  Open a terminal and execute `npm install yarn --global` (installs yarn globally)
6.  Open `/client/vscode` in Visual Studio Code
7.  Inside Visual Studio Code, open the terminal (View --> Terminal) and execute `yarn install`
8.  Execute `yarn run compile` inside that terminal
9.  Open a terminal inside of `server/`
10. (On Linux) Run `vcpkg install`
11. Run `cmake CMakeLists.txt`
12. Open `sqfvm_language_server.sln`
13. Right click `sqfvm_language_server` project inside the `Solution Explorer` and hit Build and wait for it to finish
14. Inside Visual Studio Code, hit F5

# Credits
[Armitxes](https://github.com/Armitxes/VSCode_SQF) - SQF Grammar file