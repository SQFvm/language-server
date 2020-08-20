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
        std::string_view m_schema;
        std::string_view m_user;
        std::string_view m_password;
        std::string_view m_host;
        std::string_view m_port;
        std::string_view m_path;
        std::string_view m_query;
        std::string_view m_fragment;
    protected:
        uri() {}
    public:
        uri(const std::string& input) : uri(std::string_view(input)) {}
        uri(std::string_view input)
        {
            std::string output;
            output.reserve(input.size()); // parsed string is always at max as long as input

            enum estate { schema_read, schema_wait_end, user, password, host, port, path, query, fragment };
            estate state = schema_read;
            size_t start = 0;
            size_t current = 0;
            int specialCharacter = 0;
            char specialBuffer[3] = { '\0', '\0','\0' };
            for (auto c : input)
            {
                if (specialCharacter != 0)
                {
                    specialBuffer[specialCharacter++] = c;
                    if (specialCharacter == 2)
                    {
                        specialCharacter = 0;
                        switch (specialBuffer[0])
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
                        switch (specialBuffer[1])
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
                        state = schema_wait_end;
                        m_schema = std::string_view(output).substr(start, current);
                    }
                    break;
                case schema_wait_end:
                    if (c != '/' && c != ':')
                    {
                        state = user;
                        start = current;
                    }
                    break;
                case user:
                    if (c == ':')
                    {
                        auto atLocation = input.find("@");
                        if (atLocation == std::string::npos)
                        { // no user present, pass to host state
                            state = host;
                            goto case_host;
                        }
                        m_user = std::string_view(output).substr(start, current);
                        state = password;
                        start = current + 1;
                    }
                    else if (c == '@')
                    {
                        m_user = std::string_view(output).substr(start, current);
                        state = host;
                        start = current + 1;
                    }
                    break;
                case password:
                    if (c == '@')
                    {
                        m_password = std::string_view(output).substr(start, current);
                        state = host;
                        start = current + 1;
                    }
                    break;
                case host:
                case_host:
                    if (c == '/')
                    {
                        m_host = std::string_view(output).substr(start, current);
                        state = path;
                        start = current;
                    }
                    else if (c == ':')
                    {
                        m_host = std::string_view(output).substr(start, current);
                        state = port;
                        start = current + 1;
                    }
                    break;
                case port:
                    if (c == '/')
                    {
                        m_port = std::string_view(output).substr(start, current);
                        state = path;
                        start = current;
                    }
                    break;
                case path:
                    if (c == '?')
                    {
                        m_path = std::string_view(output).substr(start, current);
                        state = path;
                        start = current;
                    }
                    break;
                case query:
                    if (c == '#')
                    {
                        m_query = std::string_view(output).substr(start, current);
                        state = path;
                        start = current;
                    }
                    break;
                case fragment:
                    break;
                }
            }
            switch (state)
            {
            case host:     m_host = std::string_view(output).substr(start, current); break;
            case port:     m_port = std::string_view(output).substr(start, current); break;
            case path:     m_path = std::string_view(output).substr(start, current); break;
            case query:    m_query = std::string_view(output).substr(start, current); break;
            case fragment: m_fragment = std::string_view(output).substr(start, current); break;
            default: break;
            }
        }


        std::string_view full() const { return m_data; }
        std::string_view schema() const { return m_schema; }
        std::string_view user() const { return m_user; }
        std::string_view password() const { return m_password; }
        std::string_view host() const { return m_host; }
        std::string_view port() const { return m_port; }
        std::string_view path() const { return m_path; }
        std::string_view query() const { return m_query; }
        std::string_view fragment() const { return m_fragment; }

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
        void encode_helper(std::stringstream& sstream, std::string_view selected_view) const
        {
            for (auto c : selected_view)
            {
                if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '.' || c == '_' || c == '~')
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
            encode_helper(sstream, m_schema);
            sstream << "://";
            if (!m_user.empty())
            {
                encode_helper(sstream, m_user);
                if (!m_password.empty())
                {
                    sstream << ":";
                    encode_helper(sstream, m_password);
                }
                sstream << "@";
            }
            encode_helper(sstream, m_host);
            if (!m_port.empty())
            {
                sstream << ":";
                encode_helper(sstream, m_port);
            }
            encode_helper(sstream, m_path);
            if (!m_query.empty())
            {
                sstream << "?";
                encode_helper(sstream, m_query);
            }
            if (!m_fragment.empty())
            {
                sstream << "#";
                encode_helper(sstream, m_fragment);
            }

            return sstream.str();
        }
    };
}