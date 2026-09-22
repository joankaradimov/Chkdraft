#include "json.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace mcp
{
    const Json* Json::find(const std::string & name) const
    {
        if ( type == Type::Object )
        {
            for ( const auto & member : members )
            {
                if ( member.first == name )
                    return &member.second;
            }
        }
        return nullptr;
    }

    Json & Json::operator[](const std::string & name)
    {
        if ( type == Type::Null )
            type = Type::Object;
        if ( type != Type::Object )
            throw std::runtime_error("Not an object");

        for ( auto & member : members )
        {
            if ( member.first == name )
                return member.second;
        }
        members.emplace_back(name, Json{});
        return members.back().second;
    }

    Json & Json::push(Json value)
    {
        if ( type == Type::Null )
            type = Type::Array;
        if ( type != Type::Array )
            throw std::runtime_error("Not an array");

        items.push_back(std::move(value));
        return *this;
    }

    static void dumpString(const std::string & value, std::string & out)
    {
        out += '"';
        for ( unsigned char c : value )
        {
            switch ( c )
            {
                case '"': out += "\\\""; break;
                case '\\': out += "\\\\"; break;
                case '\n': out += "\\n"; break;
                case '\r': out += "\\r"; break;
                case '\t': out += "\\t"; break;
                case '\b': out += "\\b"; break;
                case '\f': out += "\\f"; break;
                default:
                    if ( c < 0x20 )
                    {
                        char buffer[8];
                        std::snprintf(buffer, sizeof(buffer), "\\u%04x", unsigned(c));
                        out += buffer;
                    }
                    else
                        out += char(c);
            }
        }
        out += '"';
    }

    static void dumpValue(const Json & json, std::string & out)
    {
        switch ( json.type )
        {
            case Json::Type::Null: out += "null"; break;
            case Json::Type::Bool: out += json.boolean ? "true" : "false"; break;
            case Json::Type::Number:
            {
                char buffer[32];
                // Whole numbers print as integers so that ids and coordinates round-trip as they were given
                if ( std::isfinite(json.number) && json.number == std::floor(json.number) && std::fabs(json.number) < 9007199254740992.0 )
                    std::snprintf(buffer, sizeof(buffer), "%lld", (long long)json.number);
                else if ( std::isfinite(json.number) )
                    std::snprintf(buffer, sizeof(buffer), "%.17g", json.number);
                else
                    std::snprintf(buffer, sizeof(buffer), "null");
                out += buffer;
                break;
            }
            case Json::Type::String: dumpString(json.string, out); break;
            case Json::Type::Array:
                out += '[';
                for ( size_t i=0; i<json.items.size(); ++i )
                {
                    if ( i > 0 )
                        out += ',';
                    dumpValue(json.items[i], out);
                }
                out += ']';
                break;
            case Json::Type::Object:
                out += '{';
                for ( size_t i=0; i<json.members.size(); ++i )
                {
                    if ( i > 0 )
                        out += ',';
                    dumpString(json.members[i].first, out);
                    out += ':';
                    dumpValue(json.members[i].second, out);
                }
                out += '}';
                break;
        }
    }

    std::string Json::dump() const
    {
        std::string out {};
        dumpValue(*this, out);
        return out;
    }

    namespace
    {
        struct Parser
        {
            const std::string & text;
            size_t at = 0;

            [[noreturn]] void fail(const std::string & what) const
            {
                throw std::runtime_error(what + " at offset " + std::to_string(at));
            }

            void skipWhitespace()
            {
                while ( at < text.size() && (text[at] == ' ' || text[at] == '\t' || text[at] == '\n' || text[at] == '\r') )
                    ++at;
            }

            bool consume(char c)
            {
                if ( at < text.size() && text[at] == c )
                {
                    ++at;
                    return true;
                }
                return false;
            }

            void expect(const char* word)
            {
                for ( const char* c = word; *c != '\0'; ++c )
                {
                    if ( at >= text.size() || text[at] != *c )
                        fail(std::string("Expected ") + word);
                    ++at;
                }
            }

            static void appendUtf8(unsigned codePoint, std::string & out)
            {
                if ( codePoint < 0x80 )
                    out += char(codePoint);
                else if ( codePoint < 0x800 )
                {
                    out += char(0xC0 | (codePoint >> 6));
                    out += char(0x80 | (codePoint & 0x3F));
                }
                else if ( codePoint < 0x10000 )
                {
                    out += char(0xE0 | (codePoint >> 12));
                    out += char(0x80 | ((codePoint >> 6) & 0x3F));
                    out += char(0x80 | (codePoint & 0x3F));
                }
                else
                {
                    out += char(0xF0 | (codePoint >> 18));
                    out += char(0x80 | ((codePoint >> 12) & 0x3F));
                    out += char(0x80 | ((codePoint >> 6) & 0x3F));
                    out += char(0x80 | (codePoint & 0x3F));
                }
            }

            unsigned hex4()
            {
                if ( at + 4 > text.size() )
                    fail("Truncated \\u escape");
                unsigned value = 0;
                for ( int i=0; i<4; ++i )
                {
                    char c = text[at++];
                    value <<= 4;
                    if ( c >= '0' && c <= '9' ) value |= unsigned(c - '0');
                    else if ( c >= 'a' && c <= 'f' ) value |= unsigned(c - 'a' + 10);
                    else if ( c >= 'A' && c <= 'F' ) value |= unsigned(c - 'A' + 10);
                    else fail("Bad \\u escape");
                }
                return value;
            }

            std::string parseString()
            {
                if ( !consume('"') )
                    fail("Expected a string");
                std::string out {};
                while ( true )
                {
                    if ( at >= text.size() )
                        fail("Unterminated string");
                    char c = text[at++];
                    if ( c == '"' )
                        return out;
                    else if ( c == '\\' )
                    {
                        if ( at >= text.size() )
                            fail("Unterminated escape");
                        char e = text[at++];
                        switch ( e )
                        {
                            case '"': out += '"'; break;
                            case '\\': out += '\\'; break;
                            case '/': out += '/'; break;
                            case 'b': out += '\b'; break;
                            case 'f': out += '\f'; break;
                            case 'n': out += '\n'; break;
                            case 'r': out += '\r'; break;
                            case 't': out += '\t'; break;
                            case 'u':
                            {
                                unsigned codePoint = hex4();
                                if ( codePoint >= 0xD800 && codePoint <= 0xDBFF ) // A surrogate pair spells one code point
                                {
                                    if ( at + 6 <= text.size() && text[at] == '\\' && text[at+1] == 'u' )
                                    {
                                        at += 2;
                                        unsigned low = hex4();
                                        codePoint = 0x10000 + ((codePoint - 0xD800) << 10) + (low - 0xDC00);
                                    }
                                }
                                appendUtf8(codePoint, out);
                                break;
                            }
                            default: fail("Bad escape");
                        }
                    }
                    else if ( (unsigned char)c < 0x20 )
                        fail("Control character in string");
                    else
                        out += c;
                }
            }

            Json parseNumber()
            {
                size_t start = at;
                if ( consume('-') ) {}
                while ( at < text.size() && ((text[at] >= '0' && text[at] <= '9') || text[at] == '.' || text[at] == 'e' || text[at] == 'E' || text[at] == '+' || text[at] == '-') )
                    ++at;
                std::string digits = text.substr(start, at - start);
                char* end = nullptr;
                double value = std::strtod(digits.c_str(), &end);
                if ( end == digits.c_str() || *end != '\0' )
                    fail("Bad number");
                return Json(value);
            }

            Json parseValue(int depth)
            {
                if ( depth > 256 )
                    fail("Nested too deeply");
                skipWhitespace();
                if ( at >= text.size() )
                    fail("Unexpected end of document");

                char c = text[at];
                if ( c == '{' )
                {
                    ++at;
                    Json object = Json::object();
                    skipWhitespace();
                    if ( consume('}') )
                        return object;
                    while ( true )
                    {
                        skipWhitespace();
                        std::string name = parseString();
                        skipWhitespace();
                        if ( !consume(':') )
                            fail("Expected ':'");
                        Json value = parseValue(depth + 1);
                        object.members.emplace_back(std::move(name), std::move(value));
                        skipWhitespace();
                        if ( consume(',') )
                            continue;
                        if ( consume('}') )
                            return object;
                        fail("Expected ',' or '}'");
                    }
                }
                else if ( c == '[' )
                {
                    ++at;
                    Json array = Json::array();
                    skipWhitespace();
                    if ( consume(']') )
                        return array;
                    while ( true )
                    {
                        array.items.push_back(parseValue(depth + 1));
                        skipWhitespace();
                        if ( consume(',') )
                            continue;
                        if ( consume(']') )
                            return array;
                        fail("Expected ',' or ']'");
                    }
                }
                else if ( c == '"' )
                    return Json(parseString());
                else if ( c == 't' )
                {
                    expect("true");
                    return Json(true);
                }
                else if ( c == 'f' )
                {
                    expect("false");
                    return Json(false);
                }
                else if ( c == 'n' )
                {
                    expect("null");
                    return Json(nullptr);
                }
                else if ( c == '-' || (c >= '0' && c <= '9') )
                    return parseNumber();
                else
                    fail("Unexpected character");
            }
        };
    }

    Json Json::parse(const std::string & text)
    {
        Parser parser { text };
        Json value = parser.parseValue(0);
        parser.skipWhitespace();
        if ( parser.at != text.size() )
            parser.fail("Trailing characters");
        return value;
    }
}
