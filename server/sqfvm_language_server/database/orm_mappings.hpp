//
// Created by marco.silipo on 06.06.2023.
//

#ifndef SQFVM_LANGUAGE_SERVER_DATABASE_ORM_MAPPINGS_HPP
#define SQFVM_LANGUAGE_SERVER_DATABASE_ORM_MAPPINGS_HPP

#include "tables/t_diagnostic.h"
#include "tables/t_file.h"
#include "tables/t_reference.h"
#include "tables/t_scope.h"
#include "tables/t_variable.h"

#define ORM_ENUM_MAPPING(TYPE, PRIMITIVE) \
namespace sqlite_orm { \
    template<> \
    struct type_printer<TYPE> : public integer_printer {}; \
    template<> \
    struct statement_binder<TYPE> { \
        int bind(sqlite3_stmt* stmt, int index, const TYPE& value) { \
            return statement_binder<PRIMITIVE>().bind(stmt, index, static_cast<PRIMITIVE>(value)); \
        } \
    }; \
    template<> \
    struct field_printer<TYPE> { \
        std::string operator()(const TYPE& value) const { \
            return std::to_string(static_cast<PRIMITIVE>(value)); \
        } \
    }; \
    template<> \
    struct row_extractor<TYPE> { \
        TYPE extract(sqlite3_stmt* stmt, int column_index) { \
            auto value = sqlite3_column_int64(stmt, column_index); \
            return static_cast<TYPE>(value); \
        } \
    }; \
    inline TYPE operator|(TYPE a, TYPE b) \
    { \
        return static_cast<TYPE>(static_cast<PRIMITIVE>(a) | static_cast<PRIMITIVE>(b)); \
    } \
    inline TYPE operator&(TYPE a, TYPE b) \
    { \
        return static_cast<TYPE>(static_cast<PRIMITIVE>(a) & static_cast<PRIMITIVE>(b)); \
    } \
    inline TYPE operator~(TYPE a) \
    { \
        return static_cast<TYPE>(~static_cast<PRIMITIVE>(a)); \
    } \
}

ORM_ENUM_MAPPING(sqfvm::language_server::database::tables::t_diagnostic::severity_level, int)
ORM_ENUM_MAPPING(sqfvm::language_server::database::tables::t_file::file_flags, int)
ORM_ENUM_MAPPING(sqfvm::language_server::database::tables::t_reference::type_flags, int)
ORM_ENUM_MAPPING(sqfvm::language_server::database::tables::t_reference::access_flags, int)
#endif //SQFVM_LANGUAGE_SERVER_DATABASE_ORM_MAPPINGS_HPP
