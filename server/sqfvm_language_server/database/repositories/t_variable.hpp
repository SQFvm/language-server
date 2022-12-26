#pragma once

#include "../sqlite.hpp"
#include "t_file.hpp"
#include <utility>
#include <filesystem>
#include <chrono>
#include <vector>

namespace sqfvm::lsp::repositories
{
    struct variable
    {
        enum estate
        {
            unset = 0, same, differs
        };
        std::filesystem::path path;
        uint64_t last_changed;
        estate state;

        static variable get(sqlite::database &db, file& file)
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
    };
}