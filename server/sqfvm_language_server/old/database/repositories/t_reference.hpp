#pragma once

#include "../sqlite.hpp"
#include "t_file.hpp"
#include <utility>
#include <filesystem>
#include <chrono>
#include <vector>

#include "repository_macros.h"
#include "../../analyzers/position.hpp"

namespace sqfvm::language_server::repositories
{
    struct reference
    {
        enum etype
        {
            invalid,
            method_set,
            method_used,
            variable_set,
            variable_used,
        };
        uint64_t id;
        uint64_t file_fk;
        etype type;
        std::string name;
        position position;

        static sqlite::result get(sqlite::database &db, file &file, std::vector<reference> &ref_references)
        {
            sqlite::result res;
            auto [stmnt_, result_1] = db.create_statement(
                    "SELECT id, fileFk, type, name FROM tVariable WHERE fileFk = ?");
            if ((res = result_1) != sqlite::result::OK)
            {
                return res;
            }
            SQLITE_RESULT_GUARD(stmnt_->bind_int64(0, file.id))
            while ((res = stmnt_->next()) == sqlite::result::OK)
            {
                auto [value, bind_result] = bind(stmnt_.value());
                if (bind_result != sqlite::result::OK)
                {
                    return bind_result;
                }
                ref_references.push_back(value);
            }
            return res;
        }

        static sqlite::result get(sqlite::database &db, const std::string &name, std::vector<reference> &ref_references)
        {
            sqlite::result res;
            auto [stmnt_, result_1] = db.create_statement(
                    "SELECT id, fileFk, type, name FROM tVariable WHERE name = ?");
            if ((res = result_1) != sqlite::result::OK)
            {
                return res;
            }
            SQLITE_RESULT_GUARD(stmnt_->bind_text(0, name))
            while ((res = stmnt_->next()) == sqlite::result::OK)
            {
                auto [value, bind_result] = bind(stmnt_.value());
                if (bind_result != sqlite::result::OK)
                {
                    return bind_result;
                }
                ref_references.push_back(value);
            }
            return res;
        }

        static sqlite::result get(sqlite::database &db, uint64_t id, reference &ref_reference)
        {
            sqlite::result res;
            auto [stmnt_, result_1] = db.create_statement(
                    "SELECT id, fileFk, type, name FROM tVariable WHERE id = ?");
            if ((res = result_1) != sqlite::result::OK)
            {
                return res;
            }
            SQLITE_RESULT_GUARD(stmnt_->bind_int64(0, id));
            SQLITE_RESULT_GUARD(stmnt_->next());
            auto [value, bind_result] = bind(stmnt_.value());
            if (bind_result != sqlite::result::OK)
            {
                return bind_result;
            }
            ref_reference = value;
            return res;
        }

        static sqlite::result remove(sqlite::database &db, reference &reference)
        {
            sqlite::result res;
            auto [stmnt_, result_1] = db.create_statement(
                    "DELETE FROM tVariable WHERE id = ?");
            if ((res = result_1) != sqlite::result::OK)
            {
                return res;
            }
            SQLITE_RESULT_GUARD(stmnt_->bind_int64(0, reference.id));
            SQLITE_RESULT_GUARD(stmnt_->next());
            reference.id = 0;
            return res;
        }

        static sqlite::result update(sqlite::database &db, const reference &reference)
        {
            sqlite::result res;
            auto [stmnt_, result_1] = db.create_statement(
                    "UPDATE tVariable SET id = ?, fileFk = ?, type = ?, name = ? WHERE id = ?");
            if ((res = result_1) != sqlite::result::OK)
            {
                return res;
            }
            SQLITE_RESULT_GUARD(stmnt_->bind_int64(0, reference.id));
            SQLITE_RESULT_GUARD(stmnt_->bind_int64(1, reference.file_fk));
            SQLITE_RESULT_GUARD(stmnt_->bind_int64(2, reference.type));
            SQLITE_RESULT_GUARD(stmnt_->bind_text(3, reference.name));
            SQLITE_RESULT_GUARD(stmnt_->bind_int64(4, reference.id));
            return stmnt_->next();
        }

        static sqlite::result add(sqlite::database &db, reference &reference)
        {
            sqlite::result res;
            auto [stmnt_, result_1] = db.create_statement(
                    "INSERT INTO tVariable (fileFk, type, name) VALUES (?, ?, ?)");
            if ((res = result_1) != sqlite::result::OK)
            {
                return res;
            }
            SQLITE_RESULT_GUARD(stmnt_->bind_int64(0, reference.file_fk));
            SQLITE_RESULT_GUARD(stmnt_->bind_int64(1, reference.type));
            SQLITE_RESULT_GUARD(stmnt_->bind_text(2, reference.name));
            SQLITE_RESULT_GUARD(stmnt_->next());
            reference.id = db.last_inserted_row_id();
            return res;
        }

    private:
        static std::pair<reference, sqlite::result> bind(sqlite::prepared &prepared)
        {
            sqlite::result res;
            auto [value_0, result_0] = prepared.get_int64(0);
            auto [value_1, result_1] = prepared.get_int64(1);
            auto [value_2, result_2] = prepared.get_int(2);
            auto [value_3, result_3] = prepared.get_text(3);
            if ((res = result_0) != sqlite::result::OK
                || (res = result_1) != sqlite::result::OK
                || (res = result_2) != sqlite::result::OK
                || (res = result_3) != sqlite::result::OK)
            {
                return std::make_pair(reference { }, res);
            }
            return std::make_pair(
                    reference { (uint64_t) value_0, (uint64_t) value_1, static_cast<etype>(value_2), value_3.value() },
                    res);
        }
    };
}