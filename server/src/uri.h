#pragma once
#include <string>
#include <string_view>
#include <array>
#include <sstream>

namespace x39
{
    class uri
    {
    private:
        std::string m_data;
        size_t m_schema_start;
        size_t m_schema_length;
        size_t m_user_start;
        size_t m_user_length;
        size_t m_password_start;
        size_t m_password_length;
        size_t m_host_start;
        size_t m_host_length;
        size_t m_port_start;
        size_t m_port_length;
        size_t m_path_start;
        size_t m_path_length;
        size_t m_query_start;
        size_t m_query_length;
        size_t m_fragment_start;
        size_t m_fragment_length;
    protected:
        uri() :
            m_data(),
            m_schema_start(0),
            m_schema_length(0),
            m_user_start(0),
            m_user_length(0),
            m_password_start(0),
            m_password_length(0),
            m_host_start(0),
            m_host_length(0),
            m_port_start(0),
            m_port_length(0),
            m_path_start(0),
            m_path_length(0),
            m_query_start(0),
            m_query_length(0),
            m_fragment_start(0),
            m_fragment_length(0) {}
    public:
        uri(const char* input) : uri(std::string_view(input)) {}
        uri(const std::string& input) : uri(std::string_view(input)) {}
        uri(std::string_view input) : uri()
        {
            auto& output = m_data;
            output.reserve(input.size()); // parsed string is always at max as long as input

            enum estate { schema_read, schema_wait_length, schema_wait_length_slash, user, password, host, port, path, query, fragment };
            estate state = schema_read;
            size_t start = 0;
            size_t current = 0;
            int specialCharacter = 0;
            char specialBuffer[4] = { ' ', '\0', '\0', '\0' };
            for (auto c : input)
            {
                if (specialCharacter != 0)
                {
                    specialBuffer[specialCharacter++] = c;
                    if (specialCharacter == 3)
                    {
                        specialCharacter = 0;
                        switch (specialBuffer[1])
                        {
                        case '0':
                            c = 0x00;
                            break;
                        case '1':
                            c = 0x10;
                            break;
                        case '2':
                            c = 0x20;
                            break;
                        case '3':
                            c = 0x30;
                            break;
                        case '4':
                            c = 0x40;
                            break;
                        case '5':
                            c = 0x50;
                            break;
                        case '6':
                            c = 0x60;
                            break;
                        case '7':
                            c = 0x70;
                            break;
                        case '8':
                            c = 0x80;
                            break;
                        case '9':
                            c = 0x90;
                            break;
                        case 'a': case 'A':
                            c = 0xA0;
                            break;
                        case 'b': case 'B':
                            c = 0xB0;
                            break;
                        case 'c': case 'C':
                            c = 0xC0;
                            break;
                        case 'd': case 'D':
                            c = 0xD0;
                            break;
                        case 'e': case 'E':
                            c = 0xE0;
                            break;
                        case 'f': case 'F':
                            c = 0xF0;
                            break;
                        }
                        switch (specialBuffer[2])
                        {
                        case '0':
                            c |= 0x0;
                            break;
                        case '1':
                            c |= 0x1;
                            break;
                        case '2':
                            c |= 0x2;
                            break;
                        case '3':
                            c |= 0x3;
                            break;
                        case '4':
                            c |= 0x4;
                            break;
                        case '5':
                            c |= 0x5;
                            break;
                        case '6':
                            c |= 0x6;
                            break;
                        case '7':
                            c |= 0x7;
                            break;
                        case '8':
                            c |= 0x8;
                            break;
                        case '9':
                            c |= 0x9;
                            break;
                        case 'a': case 'A':
                            c |= 0xA;
                            break;
                        case 'b': case 'B':
                            c |= 0xB;
                            break;
                        case 'c': case 'C':
                            c |= 0xC;
                            break;
                        case 'd': case 'D':
                            c |= 0xD;
                            break;
                        case 'e': case 'E':
                            c |= 0xE;
                            break;
                        case 'f': case 'F':
                            c |= 0xF;
                            break;
                        }
                        current++;
                        output.append(&c, &c + 1);
                        continue;
                    }
                    else
                    {
                        continue;
                    }
                }
                else if (c == '%')
                {
                    specialCharacter++;
                    continue;
                }
                current++;
                output.append(&c, &c + 1);
                switch (state)
                {
                case schema_read:
                    if (c == ':')
                    {
                        state = schema_wait_length;
                        m_schema_start = start;
                        m_schema_length = current - start - 1;
                    }
                    break;
                case schema_wait_length:
                    if (c == '/')
                    {
                        state = schema_wait_length_slash;
                    }
                    else if (c != ':')
                    { // invalid uri. Still try to parse
                        state = user;
                        start = current - 1;
                    }
                    break;
                case schema_wait_length_slash:
                    if (c != '/')
                    { // invalid uri. Still try to parse
                        state = user;
                        start = current - 1;
                        goto l_user;
                    }
                    else // char c is '/' here
                    {
                        state = user;
                        start = current; // Skip current '/'
                    }
                    break;
                l_user:
                case user:
                    if (c == ':')
                    {
                        auto atLocation = input.find("@");
                        if (atLocation == std::string::npos)
                        { // no user present, pass to host state
                            state = host;
                            goto case_host;
                        }
                        m_user_start = start;
                        m_user_length = current - start - 1;
                        state = password;
                        start = current;
                    }
                    else if (c == '@')
                    {
                        m_user_start = start;
                        m_user_length = current - start - 1;
                        state = host;
                        start = current;
                    }
                    else if (c == '/')
                    {
                        m_host_start = start;
                        m_host_length = current - start - 1;
                        state = path;
                        start = current;
                    }
                    else if (c == '?')
                    {
                        m_host_start = start;
                        m_host_length = current - start - 1;
                        state = query;
                        start = current;
                    }
                    else if (c == '#')
                    {
                        m_host_start = start;
                        m_host_length = current - start - 1;
                        state = fragment;
                        start = current;
                    }
                    else if (c == ':')
                    {
                        m_host_start = start;
                        m_host_length = current - start - 1;
                        state = port;
                        start = current;
                    }
                    break;
                case password:
                    if (c == '@')
                    {
                        m_password_start = start;
                        m_password_length = current - start - 1;
                        state = host;
                        start = current;
                    }
                    else if (c == '/')
                    {
                        m_host_start = start;
                        m_host_length = current - start - 1;
                        state = path;
                        start = current;
                    }
                    else if (c == '?')
                    {
                        m_host_start = start;
                        m_host_length = current - start - 1;
                        state = query;
                        start = current;
                    }
                    else if (c == '#')
                    {
                        m_host_start = start;
                        m_host_length = current - start - 1;
                        state = fragment;
                        start = current;
                    }
                    else if (c == ':')
                    {
                        m_host_start = start;
                        m_host_length = current - start - 1;
                        state = port;
                        start = current;
                    }
                    break;
                case host:
                case_host:
                    if (c == '/')
                    {
                        m_host_start = start;
                        m_host_length = current - start - 1;
                        state = path;
                        start = current;
                    }
                    else if (c == '?')
                    {
                        m_host_start = start;
                        m_host_length = current - start - 1;
                        state = query;
                        start = current;
                    }
                    else if (c == '#')
                    {
                        m_host_start = start;
                        m_host_length = current - start - 1;
                        state = fragment;
                        start = current;
                    }
                    else if (c == ':')
                    {
                        m_host_start = start;
                        m_host_length = current - start - 1;
                        state = port;
                        start = current;
                    }
                    break;
                case port:
                    if (c == '/')
                    {
                        m_port_start = start;
                        m_port_length = current - start - 1;
                        if (m_port_length == 0)
                        {
                            m_host_length++;
                        }
                        state = path;
                        start = current;
                    }
                    else if (c == '?')
                    {
                        m_port_start = start;
                        m_port_length = current - start - 1;
                        state = query;
                        start = current;
                    }
                    else if (c == '#')
                    {
                        m_port_start = start;
                        m_port_length = current - start - 1;
                        state = fragment;
                        start = current;
                    }
                    break;
                case path:
                    if (c == '?')
                    {
                        m_path_start = start;
                        m_path_length = current - start - 1;
                        state = query;
                        start = current;
                    }
                    else if (c == '#')
                    {
                        m_path_start = start;
                        m_path_length = current - start - 1;
                        state = fragment;
                        start = current;
                    }
                    break;
                case query:
                    if (c == '#')
                    {
                        m_query_start = start;
                        m_query_length = current - start - 1;
                        state = fragment;
                        start = current;
                    }
                    break;
                case fragment:
                    break;
                }
            }
            switch (state)
            {
            case user:
            case password:
            case host:
                m_host_start = start;
                m_host_length = current - start;
                break;
            case port:
                m_port_start = start;
                m_port_length = current - start;
                break;
            case path:
                m_path_start = start;
                m_path_length = current - start;
                break;
            case query:
                m_query_start = start;
                m_query_length = current - start;
                break;
            case fragment:
                m_fragment_start = start;
                m_fragment_length = current - start - 1;
                break;
            default: break;
            }
        }
        uri(std::string_view schema,
            std::string_view user,
            std::string_view password,
            std::string_view host,
            std::string_view port,
            std::string_view path,
            std::string_view query,
            std::string_view fragment) : uri()
        {
            using namespace std::string_view_literals;
            m_data.reserve(
                schema.length() +
                "://"sv.length() +
                (user.empty() ? 0 : user.length() + "@"sv.length() + (password.empty() ? 0 : password.length() + ":"sv.length())) +
                host.length() + 
                "/"sv.length() + 
                (port.empty() ? 0 : port.length() + ":"sv.length()) +
                path.length() + 
                (query.empty() ? 0 : query.length() + "?"sv.length()) +
                (fragment.empty() ? 0 : fragment.length() + "#"sv.length())
            );
            size_t cur = 0;

            m_data.append(schema);
            m_schema_start = cur;
            cur += m_schema_length = schema.length();

            m_data.append("://"sv);
            cur += "://"sv.length();

            if (!user.empty())
            {
                m_data.append(user);
                m_user_start = cur;
                cur += m_user_length = user.length();

                if (!password.empty())
                {
                    m_data.append(":"sv);
                    cur += ":"sv.length();

                    m_data.append(password);
                    m_password_start = cur;
                    cur += m_password_length = password.length();
                }

                m_data.append("@"sv);
                cur += "@"sv.length();
            }

            m_data.append(host);
            m_host_start = cur;
            cur += m_host_length = host.length();

            if (!port.empty())
            {
                m_data.append(":"sv);
                cur += ":"sv.length();

                m_data.append(port);
                m_port_start = cur;
                cur += m_port_length = port.length();
            }

            m_data.append("/"sv);
            cur += "/"sv.length();

            m_data.append(path);
            m_path_start = cur;
            cur += m_path_length = path.length();

            if (!query.empty())
            {
                m_data.append("?"sv);
                cur += "?"sv.length();

                m_data.append(query);
                m_query_start = cur;
                cur += m_query_length = query.length();
            }

            if (!fragment.empty())
            {
                m_data.append("#"sv);
                cur += "#"sv.length();

                m_data.append(fragment);
                m_fragment_start = cur;
                cur += m_fragment_length = fragment.length();
            }
        }


        std::string_view full()     const { return m_data; }
        std::string_view schema()   const { return std::string_view(m_data.data() + m_schema_start, m_schema_length); }
        std::string_view user()     const { return std::string_view(m_data.data() + m_user_start, m_user_length); }
        std::string_view password() const { return std::string_view(m_data.data() + m_password_start, m_password_length); }
        std::string_view host()     const { return std::string_view(m_data.data() + m_host_start, m_host_length); }
        std::string_view port()     const { return std::string_view(m_data.data() + m_port_start, m_port_length); }
        std::string_view path()     const { return std::string_view(m_data.data() + m_path_start, m_path_length); }
        std::string_view query()    const { return std::string_view(m_data.data() + m_query_start, m_query_length); }
        std::string_view fragment() const { return std::string_view(m_data.data() + m_fragment_start, m_fragment_length); }

        static std::array<char, 3> escape(char c)
        {
            std::array<char, 3> arr = { '%', '\0', '\0' };
            switch ((c & 0xF0) >> 4)
            {
            case 0:  arr[1] = '0'; break;
            case 1:  arr[1] = '1'; break;
            case 2:  arr[1] = '2'; break;
            case 3:  arr[1] = '3'; break;
            case 4:  arr[1] = '4'; break;
            case 5:  arr[1] = '5'; break;
            case 6:  arr[1] = '6'; break;
            case 7:  arr[1] = '7'; break;
            case 8:  arr[1] = '8'; break;
            case 9:  arr[1] = '9'; break;
            case 10: arr[1] = 'A'; break;
            case 11: arr[1] = 'B'; break;
            case 12: arr[1] = 'C'; break;
            case 13: arr[1] = 'D'; break;
            case 14: arr[1] = 'E'; break;
            case 15: arr[1] = 'F'; break;
            }
            switch (c & 0x0F)
            {
            default:
            case 0:  arr[2] = '0'; break;
            case 1:  arr[2] = '1'; break;
            case 2:  arr[2] = '2'; break;
            case 3:  arr[2] = '3'; break;
            case 4:  arr[2] = '4'; break;
            case 5:  arr[2] = '5'; break;
            case 6:  arr[2] = '6'; break;
            case 7:  arr[2] = '7'; break;
            case 8:  arr[2] = '8'; break;
            case 9:  arr[2] = '9'; break;
            case 10: arr[2] = 'A'; break;
            case 11: arr[2] = 'B'; break;
            case 12: arr[2] = 'C'; break;
            case 13: arr[2] = 'D'; break;
            case 14: arr[2] = 'E'; break;
            case 15: arr[2] = 'F'; break;
            }

            return arr;
        }

    private:
        void encode_helper(std::stringstream& sstream, std::string_view selected_view, const char* allowed) const
        {
            for (auto c : selected_view)
            {
                bool flag = false;
                for (const char* it = allowed; *it != '\0'; it++)
                {
                    if (*it == c)
                    {
                        flag = true;
                        break;
                    }
                }
                if (flag)
                {
                    sstream << c;
                }
                else
                {
                    auto res = escape(c);
                    sstream << std::string_view(res.data(), res.size());
                }
            }
        }
    public:
        std::string encoded() const
        {
            std::stringstream sstream;

            // ToDo: Parse into https://de.wikipedia.org/wiki/URL-Encoding
            encode_helper(sstream, schema(), "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0987654321-._~");
            sstream << "://";
            if (!user().empty())
            {
                encode_helper(sstream, user(), "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0987654321-._~");
                if (!password().empty())
                {
                    sstream << ":";
                    encode_helper(sstream, password(), "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0987654321-._~");
                }
                sstream << "@";
            }
            encode_helper(sstream, host(), "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0987654321-._~");
            if (!port().empty())
            {
                sstream << ":";
                encode_helper(sstream, port(), "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0987654321-._~");
            }
            sstream << "/";
            encode_helper(sstream, path(), "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0987654321-._~/");
            if (!query().empty())
            {
                sstream << "?";
                encode_helper(sstream, query(), "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0987654321-._~&");
            }
            if (!fragment().empty())
            {
                sstream << "#";
                encode_helper(sstream, fragment(), "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0987654321-._~");
            }

            return sstream.str();
        }
    };
}