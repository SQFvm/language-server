#pragma once
#include <runtime/logging.h>

class sqf_language_server;

class language_server_logger : public Logger {
    sqf_language_server& language_server;
public:
    language_server_logger(sqf_language_server& ref) : Logger(), language_server(ref) {}

    virtual void log(const LogMessageBase& base) override;
};