#include <chrono>
#include <iostream>
#include <string>
#include "lexer/lexer.hpp"
#include "parser/Parser.hpp"
#include "printer/Printer.hpp"
#include "query/QueryEngine.hpp"
#include "schema/SchemaValidator.hpp"

namespace {

std::string makeUsersJson(size_t count) {
    std::string json = "{\"users\":[";

    for (size_t i = 0; i < count; i++) {
        if (i > 0) {
            json += ",";
        }

        json += "{\"name\":\"user";
        json += std::to_string(i);
        json += "\",\"age\":";
        json += std::to_string(20 + (i % 50));
        json += ",\"active\":";
        json += (i % 2 == 0 ? "true" : "false");
        json += "}";
    }

    json += "]}";
    return json;
}

JsonValue parseJson(const std::string& input) {
    Lexer lexer(input);
    Parser parser(lexer);
    return parser.parse();
}

template <typename Func>
double measureMs(size_t iterations, Func func) {
    auto start = std::chrono::steady_clock::now();

    for (size_t i = 0; i < iterations; i++) {
        func();
    }

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    return elapsed.count() / static_cast<double>(iterations);
}

void printBenchmark(const std::string& name, size_t iterations, double ms) {
    std::cout << name << ": " << ms << " ms/op"
              << " (" << iterations << " iterations)" << std::endl;
}

}

int main() {
    const size_t userCount = 1000;
    const size_t parseIterations = 100;
    const size_t queryIterations = 1000;
    const size_t printIterations = 100;
    const size_t schemaIterations = 1000;

    std::string json = makeUsersJson(userCount);
    JsonValue root = parseJson(json);

    QueryEngine queryEngine;
    PathExpr activeUsersQuery = queryEngine.parse("$.users[?active==true].name");

    Printer printer;
    SchemaValidator schemaValidator;
    JsonValue schema{JsonObject{
        {"users", JsonValue{"array"}},
    }};

    std::cout << "jsonq benchmark dataset: " << userCount << " users" << std::endl;

    printBenchmark("parse", parseIterations, measureMs(parseIterations, [&]() {
        JsonValue parsed = parseJson(json);
        (void)parsed;
    }));

    printBenchmark("query filter", queryIterations, measureMs(queryIterations, [&]() {
        JsonValue result = queryEngine.evaluate(root, activeUsersQuery);
        (void)result;
    }));

    printBenchmark("pretty print", printIterations, measureMs(printIterations, [&]() {
        std::string output = printer.print(root);
        (void)output;
    }));

    printBenchmark("schema validate", schemaIterations, measureMs(schemaIterations, [&]() {
        SchemaResult result = schemaValidator.validate(root, schema);
        (void)result;
    }));

    return 0;
}
