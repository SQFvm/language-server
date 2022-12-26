#include "sqlite.hpp"

#include <sqlite3.h>

#define DB reinterpret_cast<sqlite3*>(m_db)
#define STATEMENT reinterpret_cast<sqlite3_stmt*>(m_statement)

sqlite::database::~database()
{
    if (!m_db)
    {
        return;
    }
    sqlite3_close_v2(DB);
}

sqlite::result sqlite::database::open(const std::filesystem::path &path)
{
    m_path = path;
    auto str = path.string();
    auto res = static_cast<result>(sqlite3_open(str.c_str(), reinterpret_cast<sqlite3 **>(&m_db)));
    if (res == result::OK)
    {
        m_good = true;
    }
    else
    {
        m_db = nullptr;
    }
    return res;
}

int64_t sqlite::database::last_inserted_row_id() const
{
    return (int64_t) sqlite3_last_insert_rowid(DB);
}


sqlite::result sqlite::database::close()
{
    if (!m_db)
    {
        return result::OK;
    }
    auto res = static_cast<result>(sqlite3_close(DB));
    if (res == result::OK)
    {
        m_good = false;
        m_db = nullptr;
    }
    return res;
}

std::pair<std::optional<sqlite::prepared>, sqlite::result> sqlite::database::create_statement(std::string_view sql)
{
    if (sql.length() >= INT32_MAX)
    {
        return std::make_pair(std::optional<sqlite::prepared>(), result::TOOBIG);
    }
    const char *pzTail;
    sqlite3_stmt *statement;
    auto res = static_cast<result>(sqlite3_prepare_v2(DB, sql.data(), (int) sql.length(), &statement, &pzTail));
    if (res == result::OK)
    {
        return std::make_pair(std::optional<sqlite::prepared>(sqlite::prepared { statement }), res);
    }
    else
    {
        return std::make_pair(std::optional<sqlite::prepared>(), res);
    }
}

sqlite::result sqlite::database::execute(std::string_view sql)
{
    auto [stmnt_, result] = create_statement(sql);
    if (result != sqlite::result::OK)
    {
        return result;
    }
    return stmnt_->next();
}

std::string_view sqlite::database::last_error() const
{
    auto msg = sqlite3_errmsg(DB);
    return { msg };
}

sqlite::prepared::~prepared()
{
    if (!m_statement)
    {
        return;
    }
    close();
}

sqlite::result sqlite::prepared::close()
{
    if (!m_statement)
    {
        return result::OK;
    }
    auto res = static_cast<result>(sqlite3_finalize(STATEMENT));
    if (res == result::OK)
    {
        m_statement = nullptr;
    }
    return res;
}

sqlite::result sqlite::prepared::bind_blob(int index, const char *data, size_t length)
{
    if (!m_statement)
    {
        return result::MISUSE;
    }
    if (!m_bindable)
    {
        return result::MISUSE;
    }
    auto res = static_cast<result>(sqlite3_bind_blob64(STATEMENT, index, data, length, SQLITE_TRANSIENT));
    return res;
}

sqlite::result sqlite::prepared::bind_double(int index, double value)
{
    if (!m_statement)
    {
        return result::MISUSE;
    }
    if (!m_bindable)
    {
        return result::MISUSE;
    }
    auto res = static_cast<result>(sqlite3_bind_double(STATEMENT, index, value));
    return res;
}

sqlite::result sqlite::prepared::bind_int(int index, int32_t value)
{
    if (!m_statement)
    {
        return result::MISUSE;
    }
    if (!m_bindable)
    {
        return result::MISUSE;
    }
    auto res = static_cast<result>(sqlite3_bind_int(STATEMENT, index, value));
    return res;
}

sqlite::result sqlite::prepared::bind_int64(int index, int64_t value)
{
    if (!m_statement)
    {
        return result::MISUSE;
    }
    if (!m_bindable)
    {
        return result::MISUSE;
    }
    auto res = static_cast<result>(sqlite3_bind_int64(STATEMENT, index, value));
    return res;
}

sqlite::result sqlite::prepared::bind_null(int index)
{
    if (!m_statement)
    {
        return result::MISUSE;
    }
    if (!m_bindable)
    {
        return result::MISUSE;
    }
    auto res = static_cast<result>(sqlite3_bind_null(STATEMENT, index));
    return res;
}

sqlite::result sqlite::prepared::bind_text(int index, std::string_view value)
{
    if (!m_statement)
    {
        return result::MISUSE;
    }
    if (!m_bindable)
    {
        return result::MISUSE;
    }
    if (value.length() >= INT32_MAX)
    {
        return result::TOOBIG;
    }
    auto res = static_cast<result>(sqlite3_bind_text(
            STATEMENT, index, value.data(), (int) value.length(), SQLITE_TRANSIENT));
    return res;
}

size_t sqlite::prepared::parameters() const
{
    if (!m_statement)
    {
        return 0;
    }
    auto res = sqlite3_bind_parameter_count(STATEMENT);
    if (res > 0)
    {
        return res;
    }
    else
    {
        return 0;
    }
}

sqlite::result sqlite::prepared::next()
{
    if (!m_statement)
    {
        return result::MISUSE;
    }
    if (m_done)
    {
        return result::DONE;
    }
    m_bindable = false;
    auto res = static_cast<result>(sqlite3_step(STATEMENT));
    if (res == result::DONE)
    {
        m_done = true;
    }
    return res;
}

size_t sqlite::prepared::columns() const
{
    if (!m_statement)
    {
        return 0;
    }
    if (m_bindable)
    {
        return 0;
    }
    auto res = sqlite3_column_count(STATEMENT);
    if (res > 0)
    {
        return res;
    }
    else
    {
        return 0;
    }
}

sqlite::type sqlite::prepared::type(int index) const
{
    if (!m_statement)
    {
        return sqlite::type::INVALID;
    }
    if (m_bindable)
    {
        return sqlite::type::INVALID;
    }
    if (m_done)
    {
        return sqlite::type::INVALID;
    }
    auto res = static_cast<sqlite::type>(sqlite3_column_type(STATEMENT, index));
    return res;
}

std::pair<std::optional<std::vector<char>>, sqlite::result> sqlite::prepared::get_blob(int index)
{
    if (!m_statement)
    {
        return std::make_pair(std::optional<std::vector<char>>(), result::MISUSE);
    }
    if (m_bindable)
    {
        return std::make_pair(std::optional<std::vector<char>>(), result::MISUSE);
    }
    if (m_done)
    {
        return std::make_pair(std::optional<std::vector<char>>(), result::MISUSE);
    }
    auto value = static_cast<const char *>(sqlite3_column_blob(STATEMENT, index));
    if (value == nullptr)
    {
        return std::make_pair(std::optional<std::vector<char>>(), result::ERROR);
    }
    else
    {
        auto len = sqlite3_column_bytes(STATEMENT, index);
        std::vector<char> vec(value, value + len);
        return std::make_pair(std::optional<std::vector<char>>(vec), result::OK);
    }
}

std::pair<double, sqlite::result> sqlite::prepared::get_double(int index)
{
    if (!m_statement)
    {
        return std::make_pair(double(0), result::MISUSE);
    }
    if (m_bindable)
    {
        return std::make_pair(double(0), result::MISUSE);
    }
    if (m_done)
    {
        return std::make_pair(double(0), result::MISUSE);
    }
    auto value = sqlite3_column_double(STATEMENT, index);
    return std::make_pair(value, result::OK);
}

std::pair<int32_t, sqlite::result> sqlite::prepared::get_int(int index)
{
    if (!m_statement)
    {
        return std::make_pair(int32_t(0), result::MISUSE);
    }
    if (m_bindable)
    {
        return std::make_pair(int32_t(0), result::MISUSE);
    }
    if (m_done)
    {
        return std::make_pair(int32_t(0), result::MISUSE);
    }
    auto value = sqlite3_column_int(STATEMENT, index);
    return std::make_pair(value, result::OK);
}

std::pair<int64_t, sqlite::result> sqlite::prepared::get_int64(int index)
{
    if (!m_statement)
    {
        return std::make_pair(int64_t(0), result::MISUSE);
    }
    if (m_bindable)
    {
        return std::make_pair(int64_t(0), result::MISUSE);
    }
    if (m_done)
    {
        return std::make_pair(int64_t(0), result::MISUSE);
    }
    auto value = sqlite3_column_int64(STATEMENT, index);
    return std::make_pair(value, result::OK);
}

std::pair<std::optional<std::string>, sqlite::result> sqlite::prepared::get_text(int index)
{
    if (!m_statement)
    {
        return std::make_pair(std::optional<std::string>(), result::MISUSE);
    }
    if (m_bindable)
    {
        return std::make_pair(std::optional<std::string>(), result::MISUSE);
    }
    if (m_done)
    {
        return std::make_pair(std::optional<std::string>(), result::MISUSE);
    }
    auto value = sqlite3_column_text(STATEMENT, index);
    if (value == nullptr)
    {
        return std::make_pair(std::optional<std::string>(), result::ERROR);
    }
    else
    {
        auto len = sqlite3_column_bytes(STATEMENT, index);
        std::string vec(value, value + len);
        return std::make_pair(std::optional<std::string>(vec), result::OK);
    }
}
