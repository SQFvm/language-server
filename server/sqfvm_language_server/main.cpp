#include "main.hpp"
#include "language_server.hpp"
#include <iostream>

using namespace std;


int main(int argc, char **argv)
{
#ifdef _DEBUG
    _CrtDbgReport(_CRT_ASSERT, "", 0, "", "Waiting for debugger.");
#endif // _DEBUG
    sqfvm::language_server::language_server lssqf;
    lssqf.listen();
    return 0;
}
