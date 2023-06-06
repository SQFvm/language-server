#include "database.h"
#include "tables/t_file.h"
#include "tables/t_reference.h"
#include "tables/t_scope.h"
#include "tables/t_variable.h"
#include <sqlite_orm/sqlite_orm.h>

#define ORM_ENUM_MAPPING(TYPE) \
namespace sqlite_orm { \
    template<> \
    struct type_printer<TYPE> : public text_printer {}; \
    template<> \
    struct statement_binder<TYPE> { \
        int bind(sqlite3_stmt* stmt, int index, const TYPE& value) { \
            return statement_binder<uint8_t>().bind(stmt, index, static_cast<uint8_t>(value)); \
        } \
    }; \
    template<> \
    struct field_printer<TYPE> { \
        uint8_t operator()(const TYPE& value) const { \
            return static_cast<uint8_t>(value); \
        } \
    }; \
    template<> \
    struct row_extractor<TYPE> { \
        TYPE extract(uint8_t row_value) { \
            return static_cast<TYPE>(row_value); \
        } \
         \
        TYPE extract(sqlite3_stmt* stmt, int column_index) { \
            auto value = sqlite3_column_int(stmt, column_index); \
            return this->extract(value); \
        } \
    }; \
}

ORM_ENUM_MAPPING(sqfvm::lsp::database::tables::t_file::file_flags)
ORM_ENUM_MAPPING(sqfvm::lsp::database::tables::t_reference::type_flags)
ORM_ENUM_MAPPING(sqfvm::lsp::database::tables::t_reference::access_flags)


void database::map_database() {
    using namespace sqfvm::lsp::database::tables;
    using namespace sqlite_orm;
    auto path = absolute(m_db_path).string();
    auto storage = make_storage(path,
                                make_table(t_file::table_name,
                                           make_column("id_pk", &t_file::id_pk, primary_key().autoincrement()),
                                           make_column("flags", &t_file::flags),
                                           make_column("path", &t_file::path),
                                           make_column("last_changed", &t_file::last_changed)),
                                make_table(t_reference::table_name,
                                           make_column("id_pk", &t_reference::id_pk, primary_key().autoincrement()),
                                           make_column("file_fk", &t_reference::file_fk),
                                           make_column("variable_fk", &t_reference::variable_fk),
                                           make_column("access", &t_reference::access),
                                           make_column("line", &t_reference::line),
                                           make_column("column", &t_reference::column),
                                           make_column("offset", &t_reference::offset),
                                           make_column("length", &t_reference::length),
                                           make_column("types", &t_reference::types)),
                                make_table(t_scope::table_name,
                                           make_column("id_pk", &t_scope::id_pk, primary_key().autoincrement()),
                                           make_column("file_fk", &t_scope::file_fk),
                                           make_column("variable_fk", &t_scope::opt_parent_scope_fk)),
                                make_table(t_variable::table_name,
                                           make_column("id_pk", &t_variable::id_pk, primary_key().autoincrement()),
                                           make_column("name", &t_variable::name),
                                           make_column("scope_fk", &t_variable::scope_fk)));
    storage.sync_schema(false);
}
