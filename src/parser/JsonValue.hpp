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

};
