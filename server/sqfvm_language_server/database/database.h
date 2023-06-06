
#ifndef SQFVM_LANGUAGE_SERVER_DATABASE_DATABASE_H
#define SQFVM_LANGUAGE_SERVER_DATABASE_DATABASE_H
#include <filesystem>
#include <utility>
#include "sqlite/sqlite.hpp"

class database {
    std::filesystem::path m_db_path;
    sqlite::database m_db;
    bool m_bad;


    // Maps the database connection out
    void map_database();

public:
    explicit database(std::filesystem::path db_path) : m_db_path(std::move(db_path)), m_bad(true) {
        auto res = m_db.open(m_db_path);
        if (res != sqlite::result::OK)
            m_bad = true;
    }

    // Whether the connection to the database could not be established
    [[nodiscard]] bool bad() const { return m_bad; }

    // Whether the connection to the database could be established
    [[nodiscard]] bool good() const { return !m_bad; }


};


#endif //SQFVM_LANGUAGE_SERVER_DATABASE_DATABASE_H
