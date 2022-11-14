#include "migrator.hpp"

sqlite::result sqfvm::lsp::migrator::migrate_v0(sqlite::database &db)
{
    sqlite::result result;
    if ((result = db.execute(
            "CREATE TABLE IF NOT EXISTS tMigration ("
            "    id INTEGER PRIMARY KEY,"
            "    version INTEGER NOT NULL"
            "); ")) != sqlite::result::OK)
    {
        return result;
    }

    if ((result = db.execute(
            "INSERT INTO tMigration "
            "(version)"
            " VALUES "
            "(1);")) != sqlite::result::OK)
    {
        return result;
    }
    return sqlite::result::OK;
}
