#pragma once
#include "lspserver.h"
#include <string>
#include <filesystem>
#include <algorithm>

// Method to get a clear & clean uri string out of the uri provided by vscode.
static std::string sanitize(const lsp::data::uri& uri)
{
    std::string dpath;
    dpath.reserve(uri.path().length());
    dpath.append(uri.path());
    std::filesystem::path data_path(dpath);
    data_path = data_path.lexically_normal();
    dpath = data_path.string();
    std::replace(dpath.begin(), dpath.end(), '\\', '/');
    return dpath;
}
// Method to get a clear & clean uri out of the string provided by sqfvm.
static lsp::data::uri sanitize(const std::string str) { return sanitize(std::string_view(str)); }
// Method to get a clear & clean uri out of the string provided by sqfvm.
static lsp::data::uri sanitize(const std::string_view sv)
{
    auto path = std::filesystem::path(sv).lexically_normal();
    auto str = path.string();
    std::replace(str.begin(), str.end(), '\\', '/');
    return lsp::data::uri("file", {}, {}, {}, {}, str, {}, {});
}