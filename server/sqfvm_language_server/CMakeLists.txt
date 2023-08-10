# CMakeList.txt : CMake project for sqf-vm-language-server, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.8)

# Add source to this project's executable.
add_executable(sqfvm_language_server
        lsp/jsonrpc.hpp
        lsp/lspserver.hpp
        language_server.hpp
        language_server.cpp
        main.cpp
        main.hpp
        uri.hpp
        git_sha1.h
        analysis/sqf_ast/ast_visitor.hpp
        analysis/sqf_ast/sqf_ast_analyzer.cpp
        analysis/sqf_ast/sqf_ast_analyzer.hpp
        analysis/sqf_ast/visitors/variables_visitor.hpp
        analysis/analyzer.hpp
        database/tables/t_diagnostic.h
        database/tables/t_file.h
        database/tables/t_file_history.h
        database/tables/t_reference.h
        database/tables/t_variable.h
        database/context.hpp
        database/orm_mappings.hpp
        database/orm_mappings.hpp
        util.hpp
        analysis/sqf_ast/visitors/variables_visitor.cpp
        sqfvm_factory.cpp
        sqfvm_factory.hpp
        runtime_logger.hpp
)

# Set C++ Version
target_compile_features(sqfvm_language_server PUBLIC cxx_std_17)

# Add local git revision header
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)
configure_file("${PROJECT_SOURCE_DIR}/cmake/git_sha1.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/git_sha1.cpp" @ONLY)
target_sources(sqfvm_language_server PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/git_sha1.cpp")

# Find packages (use vcpkg to setup packages and pass -DCMAKE_TOOLCHAIN_FILE=[...]\vcpkg\scripts\buildsystems\vcpkg.cmake into cmake)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(SqliteOrm CONFIG REQUIRED)
find_package(Poco REQUIRED COMPONENTS Foundation)

target_link_libraries(sqfvm_language_server PRIVATE slibsqfvm)
target_link_libraries(sqfvm_language_server PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(sqfvm_language_server PRIVATE unofficial::sqlite3::sqlite3)
target_link_libraries(sqfvm_language_server PRIVATE sqlite_orm::sqlite_orm)
target_link_libraries(sqfvm_language_server PRIVATE Poco::Foundation)


# TODO: Add tests and install targets if needed.
