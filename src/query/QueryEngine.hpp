#pragma once
#include <string>
#include <vector>
#include "parser/JsonValue.hpp"

struct FilterExpr {
    enum class Operator {Equal, NotEqual, Greater, Less, GreaterEqual, LessEqual} op;
    std::string key;
    JsonValue value;
};

struct Step {
    enum class Kind {Key, Index, Wildcard, Filter} kind;
    std::string key;
    size_t index;
    FilterExpr filter;

};

using PathExpr = std::vector<Step>;

class QueryEngine {
    public:
        PathExpr parse(const std::string& query);
        JsonValue evaluate(const JsonValue& root, const PathExpr& path);


};
