#include "QueryEngine.hpp"
#include <cctype>
#include <stdexcept>

namespace {

std::string parseQuotedKey(const std::string& query, size_t& pos) {
    std::string key;
    pos++; // skip opening quote

    while (pos < query.size() && query[pos] != '"') {
        if (query[pos] == '\\') {
            pos++;
            if (pos >= query.size()) {
                throw std::runtime_error("unterminated quoted key");
            }

            if (query[pos] != '"' && query[pos] != '\\') {
                throw std::runtime_error("invalid escape in quoted key");
            }
        }

        key.push_back(query[pos]);
        pos++;
    }

    if (pos >= query.size() || query[pos] != '"') {
        throw std::runtime_error("unterminated quoted key");
    }

    pos++; // skip closing quote
    return key;
}

FilterExpr::Operator parseFilterOperator(const std::string& query, size_t& pos) {
    if (query.compare(pos, 2, "==") == 0) {
        pos += 2;
        return FilterExpr::Operator::Equal;
    }
    if (query.compare(pos, 2, "!=") == 0) {
        pos += 2;
        return FilterExpr::Operator::NotEqual;
    }
    if (query.compare(pos, 2, ">=") == 0) {
        pos += 2;
        return FilterExpr::Operator::GreaterEqual;
    }
    if (query.compare(pos, 2, "<=") == 0) {
        pos += 2;
        return FilterExpr::Operator::LessEqual;
    }
    if (pos < query.size() && query[pos] == '>') {
        pos++;
        return FilterExpr::Operator::Greater;
    }
    if (pos < query.size() && query[pos] == '<') {
        pos++;
        return FilterExpr::Operator::Less;
    }

    throw std::runtime_error("expected filter operator");
}

JsonValue parseFilterLiteral(const std::string& query, size_t& pos) {
    if (pos >= query.size()) {
        throw std::runtime_error("expected filter value");
    }

    if (query[pos] == '"') {
        return JsonValue{parseQuotedKey(query, pos)};
    }

    if (query.compare(pos, 4, "true") == 0) {
        pos += 4;
        return JsonValue{true};
    }
    if (query.compare(pos, 5, "false") == 0) {
        pos += 5;
        return JsonValue{false};
    }
    if (query.compare(pos, 4, "null") == 0) {
        pos += 4;
        return JsonValue{nullptr};
    }

    size_t start = pos;
    if (query[pos] == '-') {
        pos++;
    }

    bool hasDigit = false;
    while (pos < query.size() && std::isdigit(static_cast<unsigned char>(query[pos]))) {
        hasDigit = true;
        pos++;
    }

    if (pos < query.size() && query[pos] == '.') {
        pos++;
        while (pos < query.size() && std::isdigit(static_cast<unsigned char>(query[pos]))) {
            hasDigit = true;
            pos++;
        }
    }

    if (!hasDigit) {
        throw std::runtime_error("expected filter value");
    }

    return JsonValue{std::stod(query.substr(start, pos - start))};
}

Step parseFilterStep(const std::string& query, size_t& pos) {
    pos++; // skip '?'

    std::string key;
    if (pos < query.size() && query[pos] == '"') {
        key = parseQuotedKey(query, pos);
    }
    else {
        while (pos < query.size() && (std::isalnum(static_cast<unsigned char>(query[pos])) || query[pos] == '_' || query[pos] == '-')) {
            key.push_back(query[pos]);
            pos++;
        }
    }

    if (key.empty()) {
        throw std::runtime_error("expected filter key");
    }

    FilterExpr::Operator op = parseFilterOperator(query, pos);
    JsonValue value = parseFilterLiteral(query, pos);

    if (pos >= query.size() || query[pos] != ']') {
        throw std::runtime_error("expected ']' after filter");
    }

    pos++;
    return Step{Step::Kind::Filter, "", 0, FilterExpr{op, key, value}};
}

Step parseDotStep(const std::string& query, size_t& pos) {
    pos++; // skip '.'

    if (pos < query.size() && query[pos] == '*') {
        pos++;
        return Step{Step::Kind::Wildcard, "", 0};
    }

    std::string key;
    while (pos < query.size() && query[pos] != '.' && query[pos] != '[') {
        key.push_back(query[pos]);
        pos++;
    }

    if (key.empty()) {
        throw std::runtime_error("expected key after '.'");
    }

    return Step{Step::Kind::Key, key, 0};
}

Step parseBracketStep(const std::string& query, size_t& pos) {
    pos++; // skip '['

    if (pos >= query.size()) {
        throw std::runtime_error("unterminated bracket expression");
    }

    if (query[pos] == '*') {
        pos++;
        if (pos >= query.size() || query[pos] != ']') {
            throw std::runtime_error("expected ']' after wildcard");
        }

        pos++;
        return Step{Step::Kind::Wildcard, "", 0};
    }

    if (query[pos] == '?') {
        return parseFilterStep(query, pos);
    }

    if (query[pos] == '"') {
        std::string key = parseQuotedKey(query, pos);

        if (pos >= query.size() || query[pos] != ']') {
            throw std::runtime_error("expected ']' after quoted key");
        }

        pos++;
        return Step{Step::Kind::Key, key, 0};
    }

    std::string index;
    while (pos < query.size() && query[pos] != ']') {
        if (!std::isdigit(static_cast<unsigned char>(query[pos]))) {
            throw std::runtime_error("array index must be a non-negative integer");
        }
        index.push_back(query[pos]);
        pos++;
    }

    if (index.empty()) {
        throw std::runtime_error("expected array index");
    }

    if (pos >= query.size() || query[pos] != ']') {
        throw std::runtime_error("unterminated bracket expression");
    }

    pos++;
    return Step{Step::Kind::Index, "", static_cast<size_t>(std::stoul(index))};
}

const JsonValue* findObjectValue(const JsonValue& object, const std::string& key) {
    for (const auto& pair : object.asObject()) {
        if (pair.first == key) {
            return &pair.second;
        }
    }

    return nullptr;
}

bool valuesEqual(const JsonValue& left, const JsonValue& right) {
    if (left.isNull() || right.isNull()) {
        return left.isNull() && right.isNull();
    }
    if (left.isBool() || right.isBool()) {
        return left.isBool() && right.isBool() && left.asBool() == right.asBool();
    }
    if (left.isNumber() || right.isNumber()) {
        return left.isNumber() && right.isNumber() && left.asNumber() == right.asNumber();
    }
    if (left.isString() || right.isString()) {
        return left.isString() && right.isString() && left.asString() == right.asString();
    }

    return false;
}

bool matchesFilter(const JsonValue& value, const FilterExpr& filter) {
    if (!value.isObject()) {
        return false;
    }

    const JsonValue* field = findObjectValue(value, filter.key);
    if (field == nullptr) {
        return false;
    }

    switch (filter.op) {
        case FilterExpr::Operator::Equal:
            return valuesEqual(*field, filter.value);
        case FilterExpr::Operator::NotEqual:
            return !valuesEqual(*field, filter.value);
        case FilterExpr::Operator::Greater:
            return field->isNumber() && filter.value.isNumber() && field->asNumber() > filter.value.asNumber();
        case FilterExpr::Operator::Less:
            return field->isNumber() && filter.value.isNumber() && field->asNumber() < filter.value.asNumber();
        case FilterExpr::Operator::GreaterEqual:
            return field->isNumber() && filter.value.isNumber() && field->asNumber() >= filter.value.asNumber();
        case FilterExpr::Operator::LessEqual:
            return field->isNumber() && filter.value.isNumber() && field->asNumber() <= filter.value.asNumber();
    }

    return false;
}

}

PathExpr QueryEngine::parse(const std::string& query) {
    if (query.empty() || query[0] != '$') {
        throw std::runtime_error("query must start with '$'");
    }

    PathExpr path;
    size_t pos = 1; // skip '$'

    while (pos < query.size()) {
        if (query[pos] == '.') {
            path.push_back(parseDotStep(query, pos));
        }
        else if (query[pos] == '[') {
            path.push_back(parseBracketStep(query, pos));
        }
        else {
            throw std::runtime_error("expected '.' or '[' in query");
        }
    }
    return path;
}


JsonValue QueryEngine::evaluate(const JsonValue& root, const PathExpr& path) {

    std::vector<const JsonValue*> currentValues{&root};
    bool usedWildcard = false;

    for (const auto& step : path) {
        std::vector<const JsonValue*> nextValues;

        switch (step.kind) {
            case Step::Kind::Key:
                for (const JsonValue* current : currentValues) {
                    if (!current->isObject()) {
                        throw std::runtime_error("cannot select key from non-object");
                    }
                    nextValues.push_back(&(*current)[step.key]);
                }
                break;
            case Step::Kind::Index:
                for (const JsonValue* current : currentValues) {
                    if (!current->isArray()) {
                        throw std::runtime_error("cannot select index from non-array");
                    }
                    if (step.index >= current->asArray().size()) {
                        throw std::runtime_error("array index out of range");
                    }
                    nextValues.push_back(&current->asArray()[step.index]);
                }
                break;
            case Step::Kind::Wildcard:
                usedWildcard = true;
                for (const JsonValue* current : currentValues) {
                    if (current->isArray()) {
                        for (const JsonValue& value : current->asArray()) {
                            nextValues.push_back(&value);
                        }
                    }
                    else if (current->isObject()) {
                        for (const auto& pair : current->asObject()) {
                            nextValues.push_back(&pair.second);
                        }
                    }
                    else {
                        throw std::runtime_error("cannot select wildcard from non-container");
                    }
                }
                break;
            case Step::Kind::Filter:
                usedWildcard = true;
                for (const JsonValue* current : currentValues) {
                    if (!current->isArray()) {
                        throw std::runtime_error("cannot filter non-array");
                    }

                    for (const JsonValue& value : current->asArray()) {
                        if (matchesFilter(value, step.filter)) {
                            nextValues.push_back(&value);
                        }
                    }
                }
                break;
        }

        currentValues = nextValues;
    }

    if (usedWildcard) {
        JsonArray results;
        for (const JsonValue* value : currentValues) {
            results.push_back(*value);
        }
        return JsonValue{results};
    }

    return *currentValues.front();
}
