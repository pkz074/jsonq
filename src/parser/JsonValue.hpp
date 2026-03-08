#pragma once
#include <variant>
#include <string>
#include <vector>


struct JsonValue;

using JsonArray = std::vector<JsonValue>;
using JsonObject = std::vector<std::pair<std::string, JsonValue>>;

struct JsonValue {

    using Value = std::variant<
        std::nullptr_t, //null
        bool, // t/f
        double, // numbers
        std::string, // string
        JsonArray, // array
        JsonObject // object
    >;
    Value data;

    bool isString() const {
        return std::holds_alternative<std::string>(data);
    }

    std::string asString() const {
        return std::get<std::string>(data);
    }

    bool isNull() const {
        return std::holds_alternative<std::nullptr_t>(data);
    }

    bool isBool() const {
        return std::holds_alternative<bool>(data);
    }
    bool asBool() const {
        return std::get<bool>(data);
    }

    bool isNumber()const {
        return std::holds_alternative<double>(data);
    }
    double asNumber() const {
        return std::get<double>(data);
    }
    bool isArray() const {
        return std::holds_alternative<JsonArray>(data);
    }
    const JsonArray& asArray() const {
        return std::get<JsonArray>(data);
    }
    bool isObject() const {
        return std::holds_alternative<JsonObject>(data);
    }
    const JsonObject& asObject() const {
        return std::get<JsonObject>(data);
    }
    const JsonValue& operator[] (const std::string& key) const {
        for (const auto& pair : asObject()) {
            if (pair.first == key){
                return pair.second;
            }
        }
        throw std::runtime_error("key not found" + key);
    }

};
