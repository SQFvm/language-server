//
// Created by marco.silipo on 28.08.2023.
//

#ifndef SQFVM_LANGUAGE_SERVER_SQFVM_ANALYZER_HPP
#define SQFVM_LANGUAGE_SERVER_SQFVM_ANALYZER_HPP

#include "slspp_context.hpp"
#include "file_analyzer.hpp"
#include "../sqfvm_factory.hpp"
#include <parser/preprocessor/default.h>


namespace sqfvm::language_server::analysis {
    class sqfvm_analyzer : public file_analyzer {
        struct offset_pair {
            enum kind {
                start,
                end,
            };
            ::sqf::parser::preprocessor::impl_default::macro_resolved_data raw;
            size_t preprocessed_offset;
            enum kind kind;
        };
        std::vector<offset_pair> m_offset_pairs;
    protected:
        std::string m_text;
        std::string m_preprocessed_text;
        std::shared_ptr<sqf::runtime::runtime> m_runtime;
        std::shared_ptr<slspp_context> m_slspp_context;

        virtual void report_diagnostic(const database::tables::t_diagnostic &diagnostic) = 0;

        virtual void macro_resolved(
                ::sqf::parser::preprocessor::impl_default::macro_resolved_data orig_start,
                ::sqf::parser::preprocessor::impl_default::macro_resolved_data orig_end,
                size_t pp_start,
                size_t pp_end,
                ::sqf::runtime::runtime &runtime,
                ::sqf::runtime::parser::preprocessor::context &local_fileinfo,
                ::sqf::runtime::parser::preprocessor::context &original_fileinfo,
                const ::sqf::runtime::parser::macro &m,
                const std::unordered_map<std::string, std::string> &param_map) {};

        virtual void file_included(
                ::sqf::runtime::parser::preprocessor::context &included_fileinfo,
                ::sqf::runtime::parser::preprocessor::context &source_fileinfo) {};

        struct decoded_offset {
            ::sqf::parser::preprocessor::impl_default::macro_resolved_data resolved;
            size_t length;
        };

        [[nodiscard]] bool is_offset_in_macro(size_t offset) const {
            if (m_offset_pairs.empty()) {
                return false;
            }
            auto it = std::lower_bound(
                    m_offset_pairs.begin(), m_offset_pairs.end(), offset,
                    [](auto &pair, auto offset) {
                        return pair.preprocessed_offset < offset;
                    });
            if (it == m_offset_pairs.end()) {
                return false;
            }
            if (it->kind == offset_pair::kind::start) {
                auto next = std::next(it);
                if (next == m_offset_pairs.end()) {
                    return false;
                }
                if (next->preprocessed_offset > offset) {
                    return true;
                }
            }
            return false;
        }

        [[nodiscard]] std::optional<decoded_offset> decode_preprocessed_offset(size_t offset) const {
            if (m_offset_pairs.empty()) {
                return {};
            }
            auto it = std::lower_bound(
                    m_offset_pairs.begin(), m_offset_pairs.end(), offset,
                    [](auto &pair, auto offset) {
                        return pair.preprocessed_offset < offset;
                    });
            if (it == m_offset_pairs.end()) {
                return {};
            }
            if (it->kind == offset_pair::kind::start) {
                auto next = std::next(it);
                if (next == m_offset_pairs.end()) {
                    // This is an error, technically, as a start pair must always be followed by an end pair
                    return {};
                }
                if (next->preprocessed_offset > offset) {
                    // we are in the middle of a macro, return full macro as result
                    return decoded_offset{it->raw, next->raw.offset - it->raw.offset};
                }
            }
            // we are after a macro, return the delta to the end of that macro
            return decoded_offset{it->raw, 0};
        }

    public:
        sqfvm_analyzer(
                const std::filesystem::path &db_path,
                database::tables::t_file file,
                sqfvm_factory &factory,
                std::string text)
                : file_analyzer(db_path, std::move(file)),
                  m_slspp_context(std::make_shared<slspp_context>()),
                  m_text(std::move(text)),
                  m_preprocessed_text({}) {
            m_runtime = factory.create([&](auto &msg) {
                auto copy = msg;
                copy.source_file_fk = m_file.id_pk;
                if (!m_preprocessed_text.empty()) {
                    auto decoded = decode_preprocessed_offset(msg.offset);
                    if (decoded.has_value()) {
                        copy.line = decoded->resolved.line - /* lsp garbage random offset */ 1;
                        copy.column = decoded->resolved.column;
                        copy.offset = decoded->resolved.offset;
                        copy.length = decoded->length;
                    }
                }
                report_diagnostic(copy);
            }, m_context, m_slspp_context);
        };

        void analyze() final;

        virtual void analyze(sqf::runtime::runtime &runtime) = 0;
    };
}

#endif //SQFVM_LANGUAGE_SERVER_SQFVM_ANALYZER_HPP
