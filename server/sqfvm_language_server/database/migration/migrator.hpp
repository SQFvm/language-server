#pragma once
#include "../sqlite.hpp"

#include <utility>


namespace sqfvm::lsp
{
    class migrator
    {
        static std::pair<int, sqlite::result> get_version(sqlite::database& db)
        {
            auto [stmnt_, res1] = db.create_statement(
                "SELECT value FROM system__ ORDER BY Id DESC LIMIT 1"
            );
            if (res1 != sqlite::result::OK) { return std::make_pair(-1, res1); }
            auto res2 = stmnt_->next();
            if (res2 != sqlite::result::OK) { return std::make_pair(-1, res2); }
            auto [value, res3] = stmnt_->get_int(0);
            if (res3 != sqlite::result::OK) { return std::make_pair(-1, res3); }
            return std::make_pair(value, sqlite::result::OK);
        }
        static sqlite::result migrate_v0(sqlite::database& db);
        static sqlite::result migrate_v1(sqlite::database& db);
    public:
        static sqlite::result migrate(sqlite::database& db)
        {
            sqlite::result res;
            if ((res = migrate_v0(db)) != sqlite::result::OK) { return res; }
            auto [v, r] = get_version(db);
            if ((res = r) != sqlite::result::OK) { return res; }


            if (v < 1 && (res = migrate_v1(db)) != sqlite::result::OK) { return res; }
            return sqlite::result::OK;
        }
    };
}