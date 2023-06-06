#pragma once

namespace sqfvm::lsp
{
    struct position
    {
        size_t line;
        size_t column;
        size_t length;
        size_t offset;
    };
}