#pragma once
#include <string>
#include <vector>
#include "parser/JsonValue.hpp"

struct SchemaResult {
    bool valid;
    std::vector<std::string> errors;
};

class SchemaValidator {
public:
    SchemaResult validate(const JsonValue& document, const JsonValue& schema) const;
};
