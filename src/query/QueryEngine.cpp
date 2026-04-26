#include "QueryEngine.hpp"
#include <stdexcept>


PathExpr QueryEngine::parse(const std::string& query) {
    PathExpr path;
    size_t pos = 1; // skip '$'

    while (pos < query.size()) {
        if (query[pos] == '.') {
            pos++;
            std::string key;
            while (pos < query.size() && query[pos] != '.' && query[pos] != '[') {
                // collect characters into key
                key.push_back(query[pos]);
                pos++;
            }
            // create a Step with Kind::Key and push to path
            path.push_back(Step{Step::Kind::Key, key});
        }
        else if (query[pos] == '[') {
            //Fix that and then implement the [n] and [*] cases. For [*] it's simple — just push a Step with Kind::Wildcard. For [n] you need to collect digits between [ and ] and convert to an int with std::stoi
            pos++;
            if (query[pos] == '*'){
                path.push_back(Step{Step::Kind::Wildcard, 0});
                pos++;
                pos++;
            }
            else {
                std::string index;
                while (pos < query.size() && query[pos] != ']') {

                    index.push_back(query[pos]);
                    pos++;
                }
                pos++;
                path.push_back(Step{Step::Kind::Index, "", std::stoi(index)});
            }
        }
    }
    return path;
}


JsonValue QueryEngine::evaluate(const JsonValue& root, const PathExpr& path) {

    JsonValue current = root;
    for (const auto& step : path) {
        // if step is key -> current = current[step.key]
        // if step is index -> current = current.asArray()[step.index]
        // if step is wildcard -> current = current[*]

        switch (step.kind) {
            case Step::Kind::Key:
                current = current[step.key];
                break;
            case Step::Kind::Index:
                current = current.asArray()[step.index];
                break;
            case Step::Kind::Wildcard:
                throw std::runtime_error("todo");
                break;
        }

    }
    return current;
}
