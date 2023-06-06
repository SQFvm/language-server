#pragma once

#include "../sqlite.hpp"
#include <utility>
#include <filesystem>
#include <chrono>
#include <vector>

namespace sqfvm::language_server::repositories
{
    struct file
    {
        enum estate
        {
            unset = 0, same, differs
        };
        uint64_t id;
        std::filesystem::path path;
        uint64_t last_changed;
        estate state;

        static file get(sqlite::database &db, std::filesystem::path filepath)
        {
            sqlite::result res;
            auto [stmnt_, result_1] = db.create_statement(
                    "SELECT timestamp, state FROM tFiles WHERE filepath = ?");
            if ((res = result_1) != sqlite::result::OK)
            {
                return { filepath, 0, { } };
            }
            res = stmnt_->bind_text(0, filepath.string());
            if (res != sqlite::result::OK)
            {
                return { filepath, 0 };
            }
            res = stmnt_->next();
            if (res != sqlite::result::OK)
            {
                return { filepath, 0 };
            }
            auto [timestamp, result_2] = stmnt_->get_int64(0);
            if ((res = result_2) != sqlite::result::OK)
            {
                return { filepath, 0, { } };
            }
            auto [state, result_3] = stmnt_->get_int(1);
            if ((res = result_3) != sqlite::result::OK)
            {
                return { filepath, 0, { } };
            }
            return { filepath, static_cast<uint64_t>(timestamp), static_cast<estate>(state) };
        }

        static sqlite::result set(sqlite::database &db, const file &f)
        {
            sqlite::result res;
            auto [stmnt_, result_] = db.create_statement(
                    "INSERT OR REPLACE INTO tFiles (filepath, timestamp, state) VALUES (?, ?, ?)");
            if ((res = result_) != sqlite::result::OK)
            {
                return res;
            }

            if ((res = stmnt_->bind_text(0, f.path.string())) != sqlite::result::OK)
            {
                return res;
            }
            if ((res = stmnt_->bind_int64(1, f.last_changed)) != sqlite::result::OK)
            {
                return res;
            }
            if ((res = stmnt_->bind_int(2, static_cast<int>(f.state))) != sqlite::result::OK)
            {
                return res;
            }

            return stmnt_->next();
        }

        static sqlite::result set_all_state(sqlite::database &db, estate state)
        {
            sqlite::result res;
            auto [stmnt_, result_] = db.create_statement(
                    "UPDATE tFiles SET state = ?");
            if ((res = result_) != sqlite::result::OK)
            {
                return res;
            }
            if ((res = stmnt_->bind_int(0, static_cast<int>(state))) != sqlite::result::OK)
            {
                return res;
            }
            return stmnt_->next();
        }

        static sqlite::result drop_all(sqlite::database &db)
        {
            sqlite::result res;
            auto [stmnt_, result_] = db.create_statement(
                    "DELETE FROM tFiles");
            if ((res = result_) != sqlite::result::OK)
            {
                return res;
            }
            return stmnt_->next();
        }

        static sqlite::result drop_all(sqlite::database &db, estate state)
        {
            sqlite::result res;
            auto [stmnt_, result_] = db.create_statement(
                    "DELETE FROM tFiles WHERE state = ?");
            if ((res = result_) != sqlite::result::OK)
            {
                return res;
            }
            if ((res = stmnt_->bind_int(0, static_cast<int>(state))) != sqlite::result::OK)
            {
                return res;
            }
            return stmnt_->next();
        }

        static sqlite::result drop(sqlite::database &db, const file &f)
        {
            sqlite::result res;
            auto [stmnt_, result_] = db.create_statement(
                    "DELETE FROM tFiles WHERE filepath = ?");
            if ((res = result_) != sqlite::result::OK)
            {
                return res;
            }
            if ((res = stmnt_->bind_text(0, f.path.string())) != sqlite::result::OK)
            {
                return res;
            }
            return stmnt_->next();
        }

        static sqlite::result all(sqlite::database &db, std::vector<file> &ref_files)
        {
            sqlite::result res;
            auto [stmnt_, result_1] = db.create_statement(
                    "SELECT filepath, timestamp, state FROM tFiles");
            if ((res = result_1) != sqlite::result::OK)
            {
                return res;
            }
            while ((res = stmnt_->next()) == sqlite::result::OK)
            {
                auto [value_0, result_0] = stmnt_->get_text(0);
                auto [value_1, result_1] = stmnt_->get_int64(1);
                auto [value_2, result_2] = stmnt_->get_int(2);
                if ((res = result_0) != sqlite::result::OK
                    || (res = result_1) != sqlite::result::OK
                    || (res = result_2) != sqlite::result::OK)
                {
                    return res;
                }
                std::filesystem::path filepath(value_0.value());
                uint64_t timestamp(value_1);
                estate state(static_cast<estate>(value_2));
                ref_files.push_back({ filepath, timestamp, state });
            }
            return res;
        }

        static sqlite::result all(sqlite::database &db, std::vector<file> &ref_files, estate state)
        {
            sqlite::result res;
            auto [stmnt_, result_1] = db.create_statement(
                    "SELECT filepath, timestamp FROM tFiles WHERE state = ?");
            if ((res = result_1) != sqlite::result::OK)
            {
                return res;
            }
            if ((res = stmnt_->bind_int(0, static_cast<int>(state))) != sqlite::result::OK)
            {
                return res;
            }
            while ((res = stmnt_->next()) == sqlite::result::OK)
            {
                auto [value_0, result_0] = stmnt_->get_text(0);
                auto [value_1, result_1] = stmnt_->get_int64(1);
                auto [value_2, result_2] = stmnt_->get_int(2);
                if ((res = result_0) != sqlite::result::OK
                    || (res = result_1) != sqlite::result::OK
                    || (res = result_2) != sqlite::result::OK)
                {
                    return res;
                }
                std::filesystem::path filepath(value_0.value());
                uint64_t timestamp(value_1);
                estate state(static_cast<estate>(value_2));
                ref_files.push_back({ filepath, timestamp, state });
            }
            return res;
        }
    };
}