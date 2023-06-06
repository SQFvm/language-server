# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required(VERSION 3.8)
project(sqfvm_language_server)
set(CMAKE_CXX_STANDARD 20)

# Add SQLite3
add_subdirectory("${PROJECT_SOURCE_DIR}/extern/sqlite-amalgamation")
# Add sqlite_orm
add_subdirectory(${PROJECT_SOURCE_DIR}/extern/fetch_content)

# Add SQF-VM
add_subdirectory("${PROJECT_SOURCE_DIR}/extern/runtime")

# Include sub-projects.
add_subdirectory(sqfvm_language_server)