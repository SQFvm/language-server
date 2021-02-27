#pragma once

#include <filesystem>
#include <optional>
#include <utility>
#include <string_view>
#include <numbers>

namespace sqlite
{
    enum class result
    {
        /* Successful result */
        OK = 0,
        /* Generic error */
        ERROR = 1,
        /* Internal logic error in SQLite */
        INTERNAL = 2,
        /* Access permission denied */
        PERM = 3,
        /* Callback routine requested an abort */
        ABORT = 4,
        /* The database file is locked */
        BUSY = 5,
        /* A table in the database is locked */
        LOCKED = 6,
        /* A malloc() failed */
        NOMEM = 7,
        /* Attempt to write a readonly database */
        READONLY = 8,
        /* Operation terminated by sqlite3_interrupt()*/
        INTERRUPT = 9,
        /* Some kind of disk I/O error occurred */
        IOERR = 10,
        /* The database disk image is malformed */
        CORRUPT = 11,
        /* Unknown opcode in sqlite3_file_control() */
        NOTFOUND = 12,
        /* Insertion failed because database is full */
        FULL = 13,
        /* Unable to open the database file */
        CANTOPEN = 14,
        /* Database lock protocol error */
        PROTOCOL = 15,
        /* Internal use only */
        EMPTY = 16,
        /* The database schema changed */
        SCHEMA = 17,
        /* String or BLOB exceeds size limit */
        TOOBIG = 18,
        /* Abort due to constraint violation */
        CONSTRAINT = 19,
        /* Data type mismatch */
        MISMATCH = 20,
        /* Library used incorrectly */
        MISUSE = 21,
        /* Uses OS features not supported on host */
        NOLFS = 22,
        /* Authorization denied */
        AUTH = 23,
        /* Not used */
        FORMAT = 24,
        /* 2nd parameter to sqlite3_bind out of range */
        RANGE = 25,
        /* File opened that is not a database file */
        NOTADB = 26,
        /* Notifications from sqlite3_log() */
        NOTICE = 27,
        /* Warnings from sqlite3_log() */
        WARNING = 28,
        /* sqlite3_step() has another row ready */
        ROW = 100,
        /* sqlite3_step() has finished executing */
        DONE = 101,
        /* */
        ERROR_MISSING_COLLSEQ = (ERROR | (1 << 8)),
        /* */
        ERROR_RETRY = (ERROR | (2 << 8)),
        /* */
        ERROR_SNAPSHOT = (ERROR | (3 << 8)),
        /* */
        IOERR_READ = (IOERR | (1 << 8)),
        /* */
        IOERR_SHORT_READ = (IOERR | (2 << 8)),
        /* */
        IOERR_WRITE = (IOERR | (3 << 8)),
        /* */
        IOERR_FSYNC = (IOERR | (4 << 8)),
        /* */
        IOERR_DIR_FSYNC = (IOERR | (5 << 8)),
        /* */
        IOERR_TRUNCATE = (IOERR | (6 << 8)),
        /* */
        IOERR_FSTAT = (IOERR | (7 << 8)),
        /* */
        IOERR_UNLOCK = (IOERR | (8 << 8)),
        /* */
        IOERR_RDLOCK = (IOERR | (9 << 8)),
        /* */
        IOERR_DELETE = (IOERR | (10 << 8)),
        /* */
        IOERR_BLOCKED = (IOERR | (11 << 8)),
        /* */
        IOERR_NOMEM = (IOERR | (12 << 8)),
        /* */
        IOERR_ACCESS = (IOERR | (13 << 8)),
        /* */
        IOERR_CHECKRESERVEDLOCK = (IOERR | (14 << 8)),
        /* */
        IOERR_LOCK = (IOERR | (15 << 8)),
        /* */
        IOERR_CLOSE = (IOERR | (16 << 8)),
        /* */
        IOERR_DIR_CLOSE = (IOERR | (17 << 8)),
        /* */
        IOERR_SHMOPEN = (IOERR | (18 << 8)),
        /* */
        IOERR_SHMSIZE = (IOERR | (19 << 8)),
        /* */
        IOERR_SHMLOCK = (IOERR | (20 << 8)),
        /* */
        IOERR_SHMMAP = (IOERR | (21 << 8)),
        /* */
        IOERR_SEEK = (IOERR | (22 << 8)),
        /* */
        IOERR_DELETE_NOENT = (IOERR | (23 << 8)),
        /* */
        IOERR_MMAP = (IOERR | (24 << 8)),
        /* */
        IOERR_GETTEMPPATH = (IOERR | (25 << 8)),
        /* */
        IOERR_CONVPATH = (IOERR | (26 << 8)),
        /* */
        IOERR_VNODE = (IOERR | (27 << 8)),
        /* */
        IOERR_AUTH = (IOERR | (28 << 8)),
        /* */
        IOERR_BEGIN_ATOMIC = (IOERR | (29 << 8)),
        /* */
        IOERR_COMMIT_ATOMIC = (IOERR | (30 << 8)),
        /* */
        IOERR_ROLLBACK_ATOMIC = (IOERR | (31 << 8)),
        /* */
        IOERR_DATA = (IOERR | (32 << 8)),
        /* */
        IOERR_CORRUPTFS = (IOERR | (33 << 8)),
        /* */
        LOCKED_SHAREDCACHE = (LOCKED | (1 << 8)),
        /* */
        LOCKED_VTAB = (LOCKED | (2 << 8)),
        /* */
        BUSY_RECOVERY = (BUSY | (1 << 8)),
        /* */
        BUSY_SNAPSHOT = (BUSY | (2 << 8)),
        /* */
        BUSY_TIMEOUT = (BUSY | (3 << 8)),
        /* */
        CANTOPEN_NOTEMPDIR = (CANTOPEN | (1 << 8)),
        /* */
        CANTOPEN_ISDIR = (CANTOPEN | (2 << 8)),
        /* */
        CANTOPEN_FULLPATH = (CANTOPEN | (3 << 8)),
        /* */
        CANTOPEN_CONVPATH = (CANTOPEN | (4 << 8)),
        /* Not Used */
        CANTOPEN_DIRTYWAL = (CANTOPEN | (5 << 8)),
        /* */
        CANTOPEN_SYMLINK = (CANTOPEN | (6 << 8)),
        /* */
        CORRUPT_VTAB = (CORRUPT | (1 << 8)),
        /* */
        CORRUPT_SEQUENCE = (CORRUPT | (2 << 8)),
        /* */
        CORRUPT_INDEX = (CORRUPT | (3 << 8)),
        /* */
        READONLY_RECOVERY = (READONLY | (1 << 8)),
        /* */
        READONLY_CANTLOCK = (READONLY | (2 << 8)),
        /* */
        READONLY_ROLLBACK = (READONLY | (3 << 8)),
        /* */
        READONLY_DBMOVED = (READONLY | (4 << 8)),
        /* */
        READONLY_CANTINIT = (READONLY | (5 << 8)),
        /* */
        READONLY_DIRECTORY = (READONLY | (6 << 8)),
        /* */
        ABORT_ROLLBACK = (ABORT | (2 << 8)),
        /* */
        CONSTRAINT_CHECK = (CONSTRAINT | (1 << 8)),
        /* */
        CONSTRAINT_COMMITHOOK = (CONSTRAINT | (2 << 8)),
        /* */
        CONSTRAINT_FOREIGNKEY = (CONSTRAINT | (3 << 8)),
        /* */
        CONSTRAINT_FUNCTION = (CONSTRAINT | (4 << 8)),
        /* */
        CONSTRAINT_NOTNULL = (CONSTRAINT | (5 << 8)),
        /* */
        CONSTRAINT_PRIMARYKEY = (CONSTRAINT | (6 << 8)),
        /* */
        CONSTRAINT_TRIGGER = (CONSTRAINT | (7 << 8)),
        /* */
        CONSTRAINT_UNIQUE = (CONSTRAINT | (8 << 8)),
        /* */
        CONSTRAINT_VTAB = (CONSTRAINT | (9 << 8)),
        /* */
        CONSTRAINT_ROWID = (CONSTRAINT | (10 << 8)),
        /* */
        CONSTRAINT_PINNED = (CONSTRAINT | (11 << 8)),
        /* */
        NOTICE_RECOVER_WAL = (NOTICE | (1 << 8)),
        /* */
        NOTICE_RECOVER_ROLLBACK = (NOTICE | (2 << 8)),
        /* */
        WARNING_AUTOINDEX = (WARNING | (1 << 8)),
        /* */
        AUTH_USER = (AUTH | (1 << 8)),
        /* */
        OK_LOAD_PERMANENTLY = (OK | (1 << 8)),
        /* */
        OK_SYMLINK = (OK | (2 << 8)),
    };
    enum class type
    {
        INVALID = 0,
        T_INTEGER = 1,
        T_FLOAT = 2,
        T_TEXT = 3,
        T_BLOB = 4,
        T_NULL = 5
    };
    class database;
    class prepared
    {
        friend class database;
        void* m_statement;
        bool m_bindable;
        bool m_done;
        prepared(void* statement) : m_statement(statement), m_done(false), m_bindable(true) { }
    public:
        prepared(const prepared&) = delete;
        prepared& operator=(const prepared&) = delete;
        prepared(prepared&& p) : m_statement(p.m_statement), m_bindable(p.m_bindable), m_done(p.m_done)
        {
            p.m_statement = nullptr;
        }
        prepared& operator=(prepared&& p)
        {
            m_statement = m_statement;
            m_bindable = m_bindable;
            m_done = m_done;
            p.m_statement = nullptr;
        }
        ~prepared();
        sqlite::result close();
        sqlite::result bind_blob(int index, const char* data, size_t length);
        sqlite::result bind_double(int index, double value);
        sqlite::result bind_int(int index, int32_t value);
        sqlite::result bind_int64(int index, int64_t value);
        sqlite::result bind_null(int index);
        sqlite::result bind_text(int index, std::string_view value);
        size_t parameters() const;
        sqlite::result next();

        size_t columns() const;
        type type(int index) const;
        std::pair<std::optional<std::vector<char>>, result> get_blob(int index);
        std::pair<std::optional<double>, result> get_double(int index);
        std::pair<std::optional<int32_t>, result> get_int(int index);
        std::pair<std::optional<int64_t>, result> get_int64(int index);
        std::pair<std::optional<std::string>, result> get_text(int index);
    };
    class database
    {
        void* m_db;
        bool m_good;
        std::filesystem::path m_path;
    public:
        database(const database&) = delete;
        database& operator=(const database&) = delete;
        database(database&& d) : m_db(d.m_db), m_good(d.m_good), m_path(d.m_path) { d.m_db = nullptr; }
        database& operator=(database&& d)
        {
            m_good = m_good;
            m_db = m_db;
            m_path = m_path;
            d.m_db = nullptr;
        }
        database() : m_db(nullptr), m_good(false) { }
        database(std::filesystem::path path) : m_db(nullptr), m_good(false) { open(path); }
        ~database();
        result open(std::filesystem::path path);
        result close();
        bool good() const { return m_good; }
        std::pair<std::optional<sqlite::prepared>, result> create_statement(std::string_view view);
        std::string_view last_error() const;
    };
}

