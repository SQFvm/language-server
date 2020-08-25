#include "sqf_language_server.h"


int main(int argc, char** argv)
{
#ifdef _DEBUG
    _CrtDbgReport(_CRT_ASSERT, "", 0, "", "Waiting for vs.");
#endif // _DEBUG

    // x39::uri a("aba:///aba:aba@aba:aba/aba?aba#aba");
    // x39::uri b("aba:///aba:aba@aba:aba?aba#aba");
    // x39::uri c("aba:///aba:aba@aba?aba#aba");
    // x39::uri d("aba:///aba@aba?aba#aba");
    // x39::uri e("aba:///aba?aba#aba");
    // x39::uri f("aba:///aba?aba");
    // x39::uri g("aba:///aba");
    // x39::uri h("file:///D%3A/Git/Sqfvm/vm/tests/");
    // x39::uri i("file:///c%3A/%40X39/vscode/clients/vscode/sample/sample.sqf");
    // x39::uri j("https://www.google.com/search?rlz=1C1CHBF_deDE910DE910&sxsrf=ALeKk02J_XcmnGpP0UfYPa2S-usVtUnZXw%3A1597937338384&ei=upY-X4TzFpHikgWc7pXwBQ&q=file%3A%2F%2F%2FD%3A%2Fasdasd");
    sqf_language_server ls;
    ls.listen();
    return 0;
}