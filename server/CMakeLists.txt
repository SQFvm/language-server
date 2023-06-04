cmake_minimum_required(VERSION 2.8.12)
project(sqfvm_language_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MACOSX_RPATH 1)

# Enable multithreaded compilation in MSVC
if (MSVC)
  add_definitions(/MP)
  add_definitions(/wd4100)
endif()

# Add the filesystem library if we are building on Clang or GCC
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(ST_CXXFS_LIBS stdc++fs)
endif()

set(ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/output)
set(LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/output)
set(RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/output)

include_directories("${PROJECT_SOURCE_DIR}/extern/json/include")


set(SQFVM_ENABLE_SQC_SUPPORT "ENABLE SQC SUPPORT IN BUILD" ON)
add_subdirectory("${PROJECT_SOURCE_DIR}/extern/vm")

file(GLOB sqfvm_language_server_src 
    "${PROJECT_SOURCE_DIR}/src/*.h" "${PROJECT_SOURCE_DIR}/src/*.cpp" "${PROJECT_SOURCE_DIR}/src/*.c"
    "${CMAKE_CURRENT_BINARY_DIR}/git_sha1.cpp"
)

# Get the local git revision hash and put it into a header we can use
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

configure_file("${PROJECT_SOURCE_DIR}/cmake/git_sha1.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/git_sha1.cpp" @ONLY)
list(APPEND sqfvm_language_server_src "${CMAKE_CURRENT_BINARY_DIR}/git_sha1.cpp")



find_package(Threads)
option(SQFVM_LANGUAGE_SERVER_BUILD_EXECUTABLE "BUILD EXECUTABLE" ON)


if (SQFVM_LANGUAGE_SERVER_BUILD_EXECUTABLE)
    add_executable(sqfvm_language_server ${sqfvm_language_server_src})
    target_link_libraries(sqfvm_language_server PRIVATE slibsqfvm)
    target_link_libraries(sqfvm_language_server ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ${ST_CXXFS_LIBS})
    target_include_directories(sqfvm_language_server PUBLIC ${PROJECT_SOURCE_DIR}/src)
    
    target_compile_options(sqfvm_language_server PRIVATE
     $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
          -Wall -Wno-unknown-pragmas>
     $<$<CXX_COMPILER_ID:MSVC>:
          /W4>)
endif ()