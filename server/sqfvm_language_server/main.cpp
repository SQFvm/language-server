#include "main.hpp"
#include "sqlite.hpp"
#include "lspsqf.hpp"
#include "parser/sqf/astnode.hpp"
#include <iostream>

using namespace std;


int main()
{
#ifdef _DEBUG
    _CrtDbgReport(_CRT_ASSERT, "", 0, "", "Waiting for vs.");
#endif // _DEBUG
    sqfvm::lsp::lssqf lssqf;
    lssqf.listen();

    
    //sqlite::database db;
    //db.open("my.db");

    //if (db.good())
    //{
    //    std::cout << "SQLite3 DB opened" << std::endl;
    //    auto [stmnt_, result] = db.create_statement("CREATE TABLE IF NOT EXISTS system__ ("
    //                                                "    id INTEGER PRIMARY KEY,"
    //                                                "    version INTEGER NOT NULL"
    //                                                "); ");
    //    if (stmnt_.has_value())
    //    {
    //        auto result = stmnt_->next();
    //        if (result == sqlite::result::OK)
    //        {
    //            std::cout << "Table created" << std::endl;
    //        }
    //        else
    //        {
    //            std::cout << "Failed to create table (0x" << std::hex << static_cast<int>(result) << "/" << std::dec << static_cast<int>(result) << "): " << db.last_error() << std::endl;
    //        }
    //        std::cout << "Closing statement (" << static_cast<int>(stmnt_->close()) << "): " << db.last_error() << std::endl;
    //    }
    //    else
    //    {
    //        std::cout << "Failed to create statement (0x" << std::hex << static_cast<int>(result) << "/" << std::dec << static_cast<int>(result) << "): " << db.last_error() << std::endl;
    //    }
    //}
    //else
    //{
    //    std::cout << "Failed to open SQLite3 DB" << std::endl;
    //}
    //std::cout << "Closing db (" << static_cast<int>(db.close()) << "): " << db.last_error() << std::endl;
    return 0;
}
