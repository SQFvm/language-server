# CMakeList.txt : CMake project for sqf-vm-language-server, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable(sqfvm_language_server "main.cpp" "main.hpp" "jsonrpc.hpp" "lspserver.hpp"
                                     "sqlite.hpp" "sqlite.cpp" "lspsqf.hpp" "uri.hpp"
                                     "sqfanalyzer.hpp" "analyzer.hpp" "git_sha1.h"
                                     "sqfanalyzer.cpp" "lspsqf.cpp" "migration/v0.cpp" "migration/migrator.hpp" "migration/v1.cpp" "dbutil.hpp" "repositories/t_files.hpp")

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
target_include_directories(sqfvm_language_server PRIVATE "${PROJECT_SOURCE_DIR}/extern/sqlite-amalgamation-3340100")
target_sources(sqfvm_language_server PRIVATE "${PROJECT_SOURCE_DIR}/extern/sqlite-amalgamation-3340100/sqlite3.c")

# Add sqfvm
target_link_libraries(sqfvm_language_server PRIVATE slibsqfvm_sqc)
# target_include_directories(sqfvm_language_server PRIVATE "${PROJECT_SOURCE_DIR}/extern/runtime/src")


# TODO: Add tests and install targets if needed.
