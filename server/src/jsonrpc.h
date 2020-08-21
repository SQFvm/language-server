#pragma once
#include <iostream>
#include <unordered_map>
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>
#include <vector>
#include <algorithm>
#include <string>
#include <string_view>
#include <charconv>
#include <functional>
#include <variant>
#include <optional>

#include <nlohmann/json.hpp>

class jsonrpc
{
    static constexpr size_t buffersize = 1;
    static constexpr const char* newline = "\n";
    static constexpr const char* double_newline = "\n\n";
public:
    struct rpcmessage
    {
        std::string protocol_version;
        std::string id;
        std::string method;
        std::optional<nlohmann::json> result;
        std::optional<nlohmann::json> params;

        rpcmessage() : protocol_version("2.0"), id({}), method({}), result(), params() {}
        rpcmessage(std::string id, std::string method) : protocol_version("2.0"), id(id), method(method), result(), params() {}
        rpcmessage(std::string id, std::string method, nlohmann::json params) : protocol_version("2.0"), id(id), method(method), result(), params(params) {}
        rpcmessage(std::string id, nlohmann::json result) : protocol_version("2.0"), id(id), method({}), result(result), params() {}

        nlohmann::json serialize() const
        {
            nlohmann::json res = { { "jsonrpc", protocol_version } };
            if (result.has_value())
            {
                res["result"] = *result;
            }
            if (params.has_value())
            {
                res["params"] = *params;
            }
            if (!id.empty())
            {
                res["id"] = id;
            }
            if (!method.empty())
            {
                res["method"] = method;
            }
            return res;
        }
        static rpcmessage deserialize(const nlohmann::json& json)
        {
            rpcmessage res;
            res.protocol_version = json.contains("jsonrpc") ? json["jsonrpc"].get<std::string>() : std::string();
            res.id = json.contains("id") ? (json["id"].is_string() ? json["id"].get<std::string>() : json["id"].dump()) : std::string();
            res.method = json.contains("method") ? json["method"].get<std::string>() : std::string();
            res.result = json.contains("result") ? json["result"] : std::optional<nlohmann::json>{};
            res.params = json.contains("params") ? json["params"] : std::optional<nlohmann::json>{};
            return res;
        }
    };
    enum destruct_strategy
    {
        detach,
        join
    };
    enum parse_error_strategy
    {
        exception,
        skip
    };
    using mthd = std::function<void(jsonrpc& jsonrpc, const rpcmessage& msg)>;
private:
    std::mutex m_read_mutex;
    std::mutex m_write_mutex;
    std::istream& m_in;
    std::ostream& m_out;
    std::atomic<size_t> m_counter;
    std::thread m_read_thread;
    bool* m_read_terminate;
    bool* m_write_terminate;
    std::thread m_write_thread;
    destruct_strategy m_destruct_strategy;
    parse_error_strategy m_parse_error_strategy;

    std::queue<std::string> m_qin;
    std::queue<std::string> m_qout;
    std::unordered_map<std::string, mthd> m_methods;
public:
    jsonrpc(std::istream& sin, std::ostream& sout, destruct_strategy destruct, parse_error_strategy parse_error) :
        m_in(sin),
        m_out(sout),
        m_read_terminate(new bool(false)),
        m_write_terminate(new bool(false)),
        m_read_thread(&jsonrpc::method_read, this),
        m_write_thread(&jsonrpc::method_write, this),
        m_destruct_strategy(destruct),
        m_parse_error_strategy(parse_error)
    {
    }
    ~jsonrpc()
    {
        *m_read_terminate = true;
        *m_write_terminate = true;
        switch (m_destruct_strategy)
        {
        case detach:
            m_read_thread.detach();
            m_write_thread.detach();
            break;
        case join:
            m_read_thread.join();
            m_write_thread.join();
            break;
        }
    }

    void register_method(std::string name, mthd callback)
    {
        m_methods[name] = callback;
    }


    // Attempts to handle a single input message.
    // Returns true if a message was dequeued and false if not.
    bool handle_single_message()
    {
        std::string message;
        {
            std::lock_guard ____lock(m_read_mutex);
            if (m_qin.empty())
            {
                return false;
            }
            message = m_qin.back();
            m_qin.pop();
        }
        auto res = rpcmessage::deserialize(nlohmann::json::parse(message, nullptr, true, false));
        
        auto iter = m_methods.find(res.method);
        if (iter == m_methods.end()) { return true; }
        auto mthd = iter->second;
        mthd(*this, res);
        return true;
    }
    void send(const rpcmessage& msg)
    {
        auto json = msg.serialize();
        auto dumped = json.dump();

        {
            std::lock_guard ____lock(m_read_mutex);
            m_qout.push(dumped);
        }
    }

private:
    void method_read()
    {
        bool* terminate = m_read_terminate;
        char buffer[buffersize];
        std::vector<char> message_buffer;
        long content_length;



        while (!*terminate)
        {
            // Read Message
            auto read = m_in.read(buffer, buffersize).gcount();
            if (read == 0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            message_buffer.insert(message_buffer.end(), buffer, buffer + read);

            // Attempt to parse message
            {
                // Read message into string_view
                std::string_view whole(message_buffer.data(), message_buffer.size());
                // Try to find Content-Length Header
                {
                    auto content_length_find_res = whole.find("Content-Length:");
                    if (std::string_view::npos == content_length_find_res) { /* abort and read more */ continue; }
                    auto content_length_end_find_res = whole.find("\n", content_length_find_res);
                    if (std::string_view::npos == content_length_end_find_res) { /* abort and read more */ continue; }

                    auto content_length_str_start = content_length_find_res + std::strlen("Content-Length:");
                    for (; content_length_str_start < whole.size() && whole[content_length_str_start] == ' '; content_length_str_start++) { /* find number start */ }
                    auto content_length_str_end = content_length_str_start;
                    for (; content_length_str_end < whole.size() && whole[content_length_str_end] >= '0' && whole[content_length_str_end] <= '9'; content_length_str_end++) { /* find number end */ }
                    auto content_length_str = whole.substr(content_length_str_start, content_length_str_end - content_length_str_start);
                    auto content_length_res = std::from_chars(content_length_str.data(), content_length_str.data() + content_length_str.size(), content_length);
                    if (content_length_res.ec == std::errc::invalid_argument)
                    {
                        switch (m_parse_error_strategy)
                        {
                        case jsonrpc::exception:
                            throw std::runtime_error("Failed to parse Content-Length");
                        case jsonrpc::skip:
                            message_buffer.erase(message_buffer.begin() + content_length_find_res, message_buffer.begin() + content_length_str_end);
                            continue;
                        }
                    }
                }

                // Find content-start (indicated by double-newline)
                {
                    auto endlendl_find_res = whole.find(double_newline);
                    if (std::string_view::npos == endlendl_find_res) { /* abort and read more */ continue; }
                    whole = whole.substr(endlendl_find_res + 2);
                }

                // Read the message & erase from buffer
                {
                    if (whole.size() < content_length) { /* abort and read more */ continue; }
                    auto message = whole.substr(0, content_length);
                    {
                        std::lock_guard ____lock(m_read_mutex);
                        m_qin.push(std::string(message));
                    }
                    message_buffer.erase(message_buffer.begin(), message_buffer.begin() + (whole.data() + whole.size() - message_buffer.data()));
                }
            }
        }

        delete terminate;
    }
    void method_write()
    {
        bool* terminate = m_write_terminate;
        char buffer[buffersize];

        while (!*terminate)
        {
            bool queue_empty;

            {
                std::lock_guard ____lock(m_write_mutex);
                queue_empty = m_qout.empty();
            }
            if (queue_empty)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            else
            {
                std::string msg;

                // Dequeue single message
                {
                    std::lock_guard ____lock(m_write_mutex);
                    msg = m_qout.back();
                    m_qout.pop();
                }

                // Send message over m_out
                m_out << 
                    "Content-Length: " << msg.length() << newline <<
                    "Content-Type: application/json-rpc;charset=utf-8" << double_newline <<
                    msg;
                m_out.flush();
            }
        }

        delete terminate;
    }
};