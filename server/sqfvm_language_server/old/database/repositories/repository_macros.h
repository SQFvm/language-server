#pragma once
#ifndef SQLITE_RESULT_GUARD
#endif
#define SQLITE_RESULT_GUARD(CODE) { auto ___sqlite_result_guard___ = CODE; if (___sqlite_result_guard___ != sqlite::result::OK) { return ___sqlite_result_guard___; }}
