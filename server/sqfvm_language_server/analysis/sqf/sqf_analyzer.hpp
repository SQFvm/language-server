#ifndef SQFVM_LANGUAGE_SERVER_ANALYSIS_SQF_SQF_ANALYZER_HPP
#define SQFVM_LANGUAGE_SERVER_ANALYSIS_SQF_SQF_ANALYZER_HPP

#include "../assembly/assembly_analyzer.hpp"
#include <string_view>
#include <utility>

namespace sqfvm::language_server::analysis::sqf {
    class sqf_analyzer : public assembly::assembly_analyzer {
        database::tables::t_file m_file;
        std::string m_text;

    public:
        sqf_analyzer(
                database::tables::t_file file,
                std::string &text)
                : m_file(std::move(file)),
                  m_text(text) {}

        // Perform an abstract analysis of the document, gathering references of variables, functions, etc.
        // to be committed to the database in the next step.
        void analyze(database::context &) override;

        // Commit the analysis to the database.
        void commit(database::context &) override;
    };
}


#endif //SQFVM_LANGUAGE_SERVER_ANALYSIS_SQF_SQF_ANALYZER_HPP
