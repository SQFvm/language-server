//
// Created by marco.silipo on 17.08.2023.
//

#ifndef SQFVM_LANGUAGE_SERVER_SLSPP_CONTEXT_HPP
#define SQFVM_LANGUAGE_SERVER_SLSPP_CONTEXT_HPP

#include <string_view>
#include <utility>
#include <vector>
#include <filesystem>

namespace sqfvm::language_server::analysis {
    // Context for the SQFVM-Language-Server preprocessor step, storing user-defined preprocessor instructions.
    class slspp_context {
    public:
        enum disable_mode {
            enable,
            disable,
            disable_line,
            disable_file,
        };

        struct disable_context {
            disable_mode kind;
            size_t line;
            std::string error_code;
            std::filesystem::path file_name;

            disable_context(disable_mode kind, const std::string& file_name, size_t line, std::string error_code)
                    : kind(kind), file_name(file_name), line(line), error_code(std::move(error_code)) {}
        };

    private:
        std::vector<disable_context> m_disable_contexts;
    public:
        void push_disable_line(const std::string& file_name, size_t line, std::string_view error_code) {
            m_disable_contexts.emplace_back(
                    disable_mode::disable_line,
                    file_name,
                    line,
                    std::string(error_code.begin(), error_code.end()));
        }

        void push_disable(const std::string& file_name, size_t line, std::string_view error_code) {
            m_disable_contexts.emplace_back(
                    disable_mode::disable,
                    file_name,
                    line,
                    std::string(error_code.begin(), error_code.end()));
        }

        void push_disable_file(std::string_view error_code) {
            m_disable_contexts.emplace_back(
                    disable_mode::disable_file,
                    "",
                    0,
                    std::string(error_code.begin(), error_code.end()));
        }

        void push_enable(const std::string& file_name, size_t line, std::string_view error_code) {
            m_disable_contexts.emplace_back(
                    disable_mode::enable,
                    file_name,
                    line,
                    std::string(error_code.begin(), error_code.end()));
        }

        bool can_report(std::string_view error_code, const std::string& path, size_t line) {
            bool can_report = true;
            auto fs_path = std::filesystem::path(path);
            for (const auto &disable_context: m_disable_contexts) {
                if (disable_context.kind != disable_mode::disable_file) {
                    if (disable_context.file_name != fs_path) {
                        continue;
                    }
                    if (disable_context.line >= line) {
                        break;
                    }
                }
                if (disable_context.error_code == error_code) {
                    switch (disable_context.kind) {
                        case disable_mode::enable:
                            can_report = true;
                            break;
                        case disable_mode::disable:
                            can_report = false;
                            break;
                        case disable_mode::disable_file:
                            return false;
                        case disable_mode::disable_line:
                            if (disable_context.line + 1 == line)
                                return false;
                            break;
                    }
                }
            }
            return can_report;
        }
    };
}


#endif //SQFVM_LANGUAGE_SERVER_SLSPP_CONTEXT_HPP
