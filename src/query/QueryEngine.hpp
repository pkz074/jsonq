#pragma once
#include <string>
#include "parser/JsonValue.hpp"


struct Step {
    enum class Kind {Key, Index, Wildcard} kind;
    std::string key;
    int index;

};

using PathExpr = std::vector<Step>;

class QueryEngine {
    public:
        PathExpr parse(const std::string& query);
        JsonValue evaluate(const JsonValue& root, const PathExpr& path);


};
