#include "main.hpp"
#include "language_server.hpp"
#include <iostream>

using namespace std;


int main(int argc, char **argv)
{
#ifdef _DEBUG
    _CrtDbgReport(_CRT_ASSERT, "", 0, "", "Waiting for debugger.");
    if (std::filesystem::exists("replay.rpc.json"))
    {
        auto file = std::ifstream("replay.rpc.json");
        sqfvm::language_server::language_server lssqf(jsonrpc(file, std::cout, jsonrpc::detach, jsonrpc::skip));
        lssqf.listen();
    }
    else {
#endif // _DEBUG
        sqfvm::language_server::language_server lssqf;
        lssqf.listen();
#ifdef _DEBUG
    }
#endif // _DEBUG
    return 0;
}
