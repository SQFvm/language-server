//
// Created by marco.silipo on 28.08.2023.
//

#ifndef SQFVM_LANGUAGE_SERVER_FILE_ANALYZER_HPP
#define SQFVM_LANGUAGE_SERVER_FILE_ANALYZER_HPP

#include "db_analyzer.hpp"

namespace sqfvm::language_server::analysis {
    class file_analyzer : public db_analyzer {
    protected:
        database::tables::t_file m_file;

    public:
        file_analyzer(
                const std::filesystem::path &db_path,
                database::tables::t_file file)
                : db_analyzer(db_path),
                m_file(std::move(file)) {};
    };

}

#endif //SQFVM_LANGUAGE_SERVER_FILE_ANALYZER_HPP
