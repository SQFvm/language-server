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

#ifdef _DEBUG
#define JSONRPC_DUMP_CHAT_TO_FILE
#endif
#ifdef JSONRPC_DUMP_CHAT_TO_FILE
#include <fstream>
#include <filesystem>
#include <stdexcept>
#endif

class jsonrpc
{
    static constexpr size_t buffersize = 1024 * 4;
    static constexpr const char* newline = "\n";
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
    struct rpcframe
    {
        enum class header_kind
        {
            other,
            content_length
        };
        struct header_value_pair
        {
            std::string key;
            std::string value;
            header_kind kind;
        };
        std::vector<header_value_pair> headers;
        rpcmessage message;
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

    std::queue<rpcframe> m_qin;
    std::queue<rpcframe> m_qout;
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


    // Attempts to handle a single input frame.
    // Returns true if a frame was dequeued and false if not.
    bool handle_single_message()
    {
        rpcframe frame;
        {
            std::lock_guard ____lock(m_read_mutex);
            if (m_qin.empty())
            {
                return false;
            }
            frame = m_qin.front();
            m_qin.pop();
        }
        
        auto iter = m_methods.find(frame.message.method);
        if (iter == m_methods.end()) { return true; }
        auto mthd = iter->second;
        mthd(*this, frame.message);
        return true;
    }
    void send(const rpcmessage& msg)
    {
        rpcframe frame;
        frame.message = msg;
        frame.headers.push_back({ "Content-Type", "application/json-rpc;charset=utf-8", rpcframe::header_kind::other });
        {
            std::lock_guard ____lock(m_write_mutex);
            m_qout.push(frame);
        }
    }

private:
    void method_read()
    {
        bool* terminate = m_read_terminate;
        char buffer[buffersize];
        std::vector<char> message_buffer;
        size_t content_length = 0;

#ifdef JSONRPC_DUMP_CHAT_TO_FILE
        std::filesystem::path p("jsonrpc-read-dump.txt");
        p = std::filesystem::absolute(p);
        std::fstream __dbg_dump(p, std::fstream::out);
        if (!__dbg_dump.good())
        {
            throw std::runtime_error("Failed to open dump file");
        }
#endif
        rpcframe frame;
        rpcframe::header_value_pair header;
        enum estate { read_header, read_header_content, read_content };
        estate state = read_header;

        while (!*terminate)
        {
            switch (state)
            {
            case read_header: {
                m_in.read(buffer, 1);
                auto c = buffer[0];
#ifdef JSONRPC_DUMP_CHAT_TO_FILE
                __dbg_dump << c;
                __dbg_dump.flush();
#endif
                if (c == ':')
                {
                    state = read_header_content;
                    header.key = std::string(message_buffer.begin(), message_buffer.end());
                    message_buffer.clear();
                    if (header.key == "Content-Length")
                    {
                        header.kind = rpcframe::header_kind::content_length;
                    }
                    else
                    {
                        header.kind = rpcframe::header_kind::other;
                    }
                }
                else if (c == '\n')
                {
                    auto findres = std::find_if(frame.headers.begin(), frame.headers.end(), [](rpcframe::header_value_pair& header) -> bool { return header.kind == rpcframe::header_kind::content_length; });
                    if (findres == frame.headers.end())
                    {
                        switch (m_parse_error_strategy)
                        {
                        case jsonrpc::exception:
                            throw std::runtime_error("No Content-Length header passed");
                        case jsonrpc::skip:
                            frame = {};
                            continue;
                        }
                    }
                    auto content_length_res = std::from_chars(findres->value.data(), findres->value.data() + findres->value.size(), content_length);
                    if (content_length_res.ec == std::errc::invalid_argument)
                    {
                        switch (m_parse_error_strategy)
                        {
                        case jsonrpc::exception:
                            throw std::runtime_error("Failed to parse Content-Length");
                        case jsonrpc::skip:
                            frame = {};
                            continue;
                        }
                    }
                    state = read_content;
                    message_buffer.clear();
                }
                else
                {
                    message_buffer.push_back(c);
                }
            } break;
            case read_header_content: {
                m_in.read(buffer, 1);
                auto c = buffer[0];
#ifdef JSONRPC_DUMP_CHAT_TO_FILE
                __dbg_dump << c;
                __dbg_dump.flush();
#endif
                if (c == '\n')
                {
                    state = read_header;
                    size_t off = 0;
                    for (; off < message_buffer.size() && std::isspace(message_buffer[off]); off++);
                    header.value = std::string(message_buffer.begin() + off, message_buffer.end());
                    frame.headers.push_back(header);
                    message_buffer.clear();
                }
                else
                {
                    message_buffer.push_back(c);
                }

            } break;
            case read_content: {
                if (content_length == 0)
                {
                    auto data = std::string(message_buffer.begin(), message_buffer.end());
                    message_buffer.clear();
                    state = read_header;
                    frame.message = rpcmessage::deserialize(nlohmann::json::parse(data, nullptr, true, false));
                    {
                        std::lock_guard ____lock(m_read_mutex);
                        m_qin.push(frame);
                    }
                    frame = {};
                }
                else
                {
                    auto read = m_in.read(buffer, std::min(content_length, buffersize)).gcount();
                    message_buffer.insert(message_buffer.end(), buffer, buffer + read);
                    content_length -= read;
#ifdef JSONRPC_DUMP_CHAT_TO_FILE
                    __dbg_dump << std::string_view(buffer, read);
                    __dbg_dump.flush();
#endif
                }
            } break;
            }
        }

        delete terminate;
    }
    void method_write()
    {
        bool* terminate = m_write_terminate;

#ifdef JSONRPC_DUMP_CHAT_TO_FILE
        std::filesystem::path p("jsonrpc-write-dump.txt");
        p = std::filesystem::absolute(p);
        std::fstream __dbg_dump(p, std::fstream::out);
        if (!__dbg_dump.good())
        {
            throw std::runtime_error("Failed to open dump file");
        }
#endif

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
                rpcframe frame;

                // Dequeue single frame
                {
                    std::lock_guard ____lock(m_write_mutex);
                    frame = m_qout.front();
                    m_qout.pop();
                }

                auto json = frame.message.serialize();
                auto dumped = json.dump();

                // Send frame over m_out
                m_out << "Content-Length: " << dumped.size() << newline;
#ifdef JSONRPC_DUMP_CHAT_TO_FILE
                __dbg_dump << "Content-Length: " << dumped.size() << newline;
                __dbg_dump.flush();
#endif
                for (auto header : frame.headers)
                {
                    if (header.kind == rpcframe::header_kind::content_length)
                    {
                        continue;
                    }
                    m_out << header.key << ": " << header.value << newline;
#ifdef JSONRPC_DUMP_CHAT_TO_FILE
                    __dbg_dump << header.key << ": " << header.value << newline;
                    __dbg_dump.flush();
#endif
                }
                m_out << newline << dumped;
#ifdef JSONRPC_DUMP_CHAT_TO_FILE
                __dbg_dump << newline << dumped;
                __dbg_dump.flush();
#endif
                m_out.flush();
            }
        }

        delete terminate;
    }
};