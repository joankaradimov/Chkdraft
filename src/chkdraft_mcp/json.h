#ifndef CHKDRAFT_MCP_JSON_H
#define CHKDRAFT_MCP_JSON_H
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace mcp
{
    // A JSON document held as data. MappingCore serializes reflected structs through RareCpp, but the protocol carries
    // free-form objects whose shape depends on the method and the tool, so those are read and written as values here.
    // Object members keep the order they were given in.
    struct Json
    {
        enum class Type { Null, Bool, Number, String, Array, Object };

        Type type = Type::Null;
        bool boolean = false;
        double number = 0;
        std::string string {};
        std::vector<Json> items {}; // The elements of an array
        std::vector<std::pair<std::string, Json>> members {}; // The members of an object

        Json() = default;
        Json(std::nullptr_t) {}
        Json(bool value) : type(Type::Bool), boolean(value) {}
        Json(int value) : type(Type::Number), number(double(value)) {}
        Json(unsigned int value) : type(Type::Number), number(double(value)) {}
        Json(long value) : type(Type::Number), number(double(value)) {}
        Json(unsigned long value) : type(Type::Number), number(double(value)) {}
        Json(long long value) : type(Type::Number), number(double(value)) {}
        Json(unsigned long long value) : type(Type::Number), number(double(value)) {}
        Json(double value) : type(Type::Number), number(value) {}
        Json(const char* value) : type(Type::String), string(value) {}
        Json(const std::string & value) : type(Type::String), string(value) {}
        Json(std::string && value) : type(Type::String), string(std::move(value)) {}

        static Json array() { Json json; json.type = Type::Array; return json; }
        static Json object() { Json json; json.type = Type::Object; return json; }

        bool isNull() const { return type == Type::Null; }
        bool isBool() const { return type == Type::Bool; }
        bool isNumber() const { return type == Type::Number; }
        bool isString() const { return type == Type::String; }
        bool isArray() const { return type == Type::Array; }
        bool isObject() const { return type == Type::Object; }

        // The member of an object by name, or null when there is none
        const Json* find(const std::string & name) const;

        // The member of an object by name, added as null when there is none; turns a null value into an object
        Json & operator[](const std::string & name);

        // Appends to an array; turns a null value into an array
        Json & push(Json value);

        // The text of the document, on one line
        std::string dump() const;

        // Reads a document, throwing std::runtime_error with the position of the first problem
        static Json parse(const std::string & text);
    };
}

#endif
