#ifndef SQFVM_LANGUAGE_SERVER_ANALYSIS_ASSEMBLY_ASSEMBLY_ANALYZER_HPP
#define SQFVM_LANGUAGE_SERVER_ANALYSIS_ASSEMBLY_ASSEMBLY_ANALYZER_HPP

#include "../analyzer.hpp"
#include <runtime/instruction_set.h>

namespace sqfvm::language_server::analysis::assembly {
    class assembly_analyzer : public analyzer {
    protected:
        std::vector<database::tables::t_diagnostic> m_diagnostics;


        void analyze_instruction_set(const ::sqf::runtime::instruction_set &instruction_set);
    };
}

#endif //SQFVM_LANGUAGE_SERVER_ANALYSIS_ASSEMBLY_ASSEMBLY_ANALYZER_HPP
