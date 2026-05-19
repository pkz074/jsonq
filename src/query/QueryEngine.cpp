#include "QueryEngine.hpp"
#include <cctype>
#include <stdexcept>


PathExpr QueryEngine::parse(const std::string& query) {
    if (query.empty() || query[0] != '$') {
        throw std::runtime_error("query must start with '$'");
    }

    PathExpr path;
    size_t pos = 1; // skip '$'

    while (pos < query.size()) {
        if (query[pos] == '.') {
            pos++;
            std::string key;
            while (pos < query.size() && query[pos] != '.' && query[pos] != '[') {
                key.push_back(query[pos]);
                pos++;
            }

            if (key.empty()) {
                throw std::runtime_error("expected key after '.'");
            }

            path.push_back(Step{Step::Kind::Key, key, 0});
        }
        else if (query[pos] == '[') {
            pos++;

            if (pos >= query.size()) {
                throw std::runtime_error("unterminated index expression");
            }

            if (query[pos] == '*') {
                pos++;
                if (pos >= query.size() || query[pos] != ']') {
                    throw std::runtime_error("expected ']' after wildcard");
                }
                pos++;
                path.push_back(Step{Step::Kind::Wildcard, "", 0});
            }
            else {
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
                    throw std::runtime_error("unterminated index expression");
                }

                pos++;
                path.push_back(Step{Step::Kind::Index, "", static_cast<size_t>(std::stoul(index))});
            }
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
                    if (!current->isArray()) {
                        throw std::runtime_error("cannot select wildcard from non-array");
                    }
                    for (const JsonValue& value : current->asArray()) {
                        nextValues.push_back(&value);
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
