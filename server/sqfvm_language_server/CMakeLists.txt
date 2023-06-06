# CMakeList.txt : CMake project for sqf-vm-language-server, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable(sqfvm_language_server
        "old/analyzers/analyzer.hpp"
        "old/analyzers/sqf/analyzer_sqf.hpp"
        "old/analyzers/sqf/analyzer_sqf.cpp"
        "old/analyzers/sqf/visitors/sqf/variables_visitor.hpp"
        "old/database/migration/v0.cpp"
        "old/database/migration/migrator.hpp"
        "old/database/migration/v1.cpp"
        "old/database/repositories/t_file.hpp"
        "old/database/dbutil.hpp"
        "old/database/sqlite.hpp"
        "old/database/sqlite.cpp"
        "old/vscode/jsonrpc.hpp"
        "old/vscode/lspserver.hpp"
        "old/vscode/lspsqf.hpp"
        "old/vscode/lspsqf.cpp"
        "main.cpp"
        "main.hpp"
        "uri.hpp"
        "git_sha1.h"
        old/database/repositories/t_reference.hpp old/database/repositories/t_variable.hpp old/database/repositories/repository_macros.h old/database/repositories/repositories.hpp database/tables/t_file.h database/tables/t_variable.h database/tables/t_reference.h database/tables/t_scope.h database/database.cpp database/database.h database/sqlite/sqlite.cpp database/sqlite/sqlite.hpp)

# Set C++ Version
target_compile_features(sqfvm_language_server PUBLIC cxx_std_17)



# Add local git revision header
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)
configure_file("${PROJECT_SOURCE_DIR}/cmake/git_sha1.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/git_sha1.cpp" @ONLY)
target_sources(sqfvm_language_server PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/git_sha1.cpp")

# Add json
target_include_directories(sqfvm_language_server PRIVATE "${PROJECT_SOURCE_DIR}/extern/json/include")

# Add sqlite3
target_include_directories(sqfvm_language_server PRIVATE SQLite3_LIBRARY_INCLUDE_DIR)
target_link_libraries(sqfvm_language_server PRIVATE SQLite3_LIBRARY)

# Add sqfvm
target_link_libraries(sqfvm_language_server PRIVATE slibsqfvm_sqc)
target_include_directories(sqfvm_language_server PRIVATE sqfvm_INCLUDE_DIR)

# Add sqlite_orm
target_link_libraries(sqfvm_language_server PRIVATE sqlite_orm::sqlite_orm)

# TODO: Add tests and install targets if needed.
