# CMakeList.txt : CMake project for sqf-vm-language-server, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (sqfvm_language_server "sqf-vm-language-server.cpp" "sqf-vm-language-server.h" "jsonrpc.hpp" "lspserver.hpp" "sqlite.hpp" "sqlite.cpp")

# Add json
target_include_directories(sqfvm_language_server PRIVATE "${PROJECT_SOURCE_DIR}/extern/json/include")

# Add sqlite3
target_include_directories(sqfvm_language_server PRIVATE "${PROJECT_SOURCE_DIR}/extern/sqlite-amalgamation-3340100")
target_sources(sqfvm_language_server PRIVATE "${PROJECT_SOURCE_DIR}/extern/sqlite-amalgamation-3340100/sqlite3.c")


# TODO: Add tests and install targets if needed.
