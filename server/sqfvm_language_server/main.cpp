#include "main.hpp"
#include "parser/sqf/astnode.hpp"
#include <iostream>

using namespace std;


int main(int argc, char **argv)
{
#ifdef _DEBUG
    _CrtDbgReport(_CRT_ASSERT, "", 0, "", "Waiting for vs.");
#endif // _DEBUG
    sqfvm::lsp::lssqf lssqf;
    lssqf.listen();
    return 0;
}
