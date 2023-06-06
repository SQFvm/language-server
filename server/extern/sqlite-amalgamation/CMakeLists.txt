add_library(sqlite3 STATIC sqlite3.c sqlite3.h sqlite3ext.h)
SET(SQLite3_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "SQLite3 include directory")
SET(SQLite3_LIBRARY sqlite3 CACHE INTERNAL "SQLite3 library")