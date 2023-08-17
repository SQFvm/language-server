#ifndef SQFVM_LANGUAGE_SERVER_UTIL_HPP
#define SQFVM_LANGUAGE_SERVER_UTIL_HPP

#include "lsp/lspserver.hpp"
#include "database/context.hpp"
#include "parser/sqf/astnode.hpp"
#include <string>
#include <filesystem>
#include <algorithm>
#include <chrono>
#include <string_view>

inline static std::string_view to_string_view(sqf::parser::sqf::bison::astkind kind) {
    using namespace std::string_view_literals;
    switch (kind) {
        case sqf::parser::sqf::bison::astkind::ENDOFFILE:
            return "ENDOFFILE"sv;
        case sqf::parser::sqf::bison::astkind::INVALID:
            return "INVALID"sv;
        case sqf::parser::sqf::bison::astkind::__TOKEN:
            return "__TOKEN"sv;
        case sqf::parser::sqf::bison::astkind::NA:
            return "NA"sv;
        case sqf::parser::sqf::bison::astkind::STATEMENTS:
            return "STATEMENTS"sv;
        case sqf::parser::sqf::bison::astkind::STATEMENT:
            return "STATEMENT"sv;
        case sqf::parser::sqf::bison::astkind::IDENT:
            return "IDENT"sv;
        case sqf::parser::sqf::bison::astkind::NUMBER:
            return "NUMBER"sv;
        case sqf::parser::sqf::bison::astkind::HEXNUMBER:
            return "HEXNUMBER"sv;
        case sqf::parser::sqf::bison::astkind::STRING:
            return "STRING"sv;
        case sqf::parser::sqf::bison::astkind::BOOLEAN_TRUE:
            return "BOOLEAN_TRUE"sv;
        case sqf::parser::sqf::bison::astkind::BOOLEAN_FALSE:
            return "BOOLEAN_FALSE"sv;
        case sqf::parser::sqf::bison::astkind::EXPRESSION_LIST:
            return "EXPRESSION_LIST"sv;
        case sqf::parser::sqf::bison::astkind::CODE:
            return "CODE"sv;
        case sqf::parser::sqf::bison::astkind::ARRAY:
            return "ARRAY"sv;
        case sqf::parser::sqf::bison::astkind::ASSIGNMENT:
            return "ASSIGNMENT"sv;
        case sqf::parser::sqf::bison::astkind::ASSIGNMENT_LOCAL:
            return "ASSIGNMENT_LOCAL"sv;
        case sqf::parser::sqf::bison::astkind::EXPN:
            return "EXPN"sv;
        case sqf::parser::sqf::bison::astkind::EXP0:
            return "EXP0"sv;
        case sqf::parser::sqf::bison::astkind::EXP1:
            return "EXP1"sv;
        case sqf::parser::sqf::bison::astkind::EXP2:
            return "EXP2"sv;
        case sqf::parser::sqf::bison::astkind::EXP3:
            return "EXP3"sv;
        case sqf::parser::sqf::bison::astkind::EXP4:
            return "EXP4"sv;
        case sqf::parser::sqf::bison::astkind::EXP5:
            return "EXP5"sv;
        case sqf::parser::sqf::bison::astkind::EXP6:
            return "EXP6"sv;
        case sqf::parser::sqf::bison::astkind::EXP7:
            return "EXP7"sv;
        case sqf::parser::sqf::bison::astkind::EXP8:
            return "EXP8"sv;
        case sqf::parser::sqf::bison::astkind::EXP9:
            return "EXP9"sv;
        case sqf::parser::sqf::bison::astkind::EXPU:
            return "EXPU"sv;
        default:
            "UNKNOWN"sv;
    }
}

inline static std::string sqf_destringify(std::string_view input) {
    if (input.length() < 2)
        return std::string(input);
    std::vector<char> buff{};
    buff.reserve(input.length() - 2);
    char quote_char = input[0];
    bool was_quote = false;
    for (size_t i = 1; i < input.length() - 1; i++) {
        if (input[i] == quote_char && !was_quote) {
            was_quote = true;
            continue;
        }
        buff.push_back(input[i]);
        was_quote = false;
    }
    return {buff.begin(), buff.end()};
}

inline static bool iequal(std::string_view left, std::string_view right) {
    return std::equal(left.begin(), left.end(), right.begin(), right.end(), [](auto a, auto b) {
        return std::tolower(a) == std::tolower(b);
    });
}

inline static uint64_t unix_timestamp() {
    return (uint64_t) std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
}

inline static bool is_subpath(const std::filesystem::path &path, const std::filesystem::path &base) {
    const auto mismatch_pair = std::mismatch(path.begin(), path.end(), base.begin(), base.end());
    return mismatch_pair.second == base.end();
}

// Method to get a clear & clean uri string out of the uri provided by vscode.
inline static std::string sanitize_to_string(const lsp::data::uri &uri) {
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
inline static lsp::data::uri sanitize_to_uri(const std::string_view sv) {
    auto path = std::filesystem::path(sv).lexically_normal();
    auto str = path.string();
    std::replace(str.begin(), str.end(), '\\', '/');
    return lsp::data::uri("file", {}, {}, {}, {}, str, {}, {});
}

// Method to get a clear & clean uri out of the string provided by sqfvm.
inline static lsp::data::uri sanitize_to_uri(const std::string str) { return sanitize_to_uri(std::string_view(str)); }


#endif //SQFVM_LANGUAGE_SERVER_UTIL_HPP
