#include "migrator.hpp"

sqlite::result sqfvm::lsp::migrator::migrate_v0(sqlite::database& db)
{
    auto result = db.execute(
        "CREATE TABLE IF NOT EXISTS __system ("
        "    id INTEGER PRIMARY KEY,"
        "    version INTEGER NOT NULL"
        "); ");
    if (result != sqlite::result::OK) { return result; }

    auto result = db.execute(
        "INSERT INTO __system "
        "(version)"
        " VALUES "
        "(1);");
    if (result != sqlite::result::OK) { return result; }
    return sqlite::result::OK;
}
