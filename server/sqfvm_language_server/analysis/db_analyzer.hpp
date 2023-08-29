//
// Created by marco.silipo on 28.08.2023.
//

#ifndef SQFVM_LANGUAGE_SERVER_DB_ANALYZER_HPP
#define SQFVM_LANGUAGE_SERVER_DB_ANALYZER_HPP

#include "analyzer.hpp"

namespace sqfvm::language_server::analysis {
    class db_analyzer : public analyzer {
    protected:
        database::context m_context;
    public:
        db_analyzer(
                const std::filesystem::path &db_path)
                : m_context(db_path) {};
    };
}

#endif //SQFVM_LANGUAGE_SERVER_DB_ANALYZER_HPP
