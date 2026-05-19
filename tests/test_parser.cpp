#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "parser/Parser.hpp"

TEST_CASE("empty object", "[parser][object]") {
    std::string input = "{}";
    Lexer lexer(input);
    Parser parser(lexer);

    JsonValue result = parser.parse();

    REQUIRE(result.isObject());
    REQUIRE(result.asObject().empty());
}

TEST_CASE("object with one string field", "[parser][object]") {
    std::string input = R"({"name": "John"})";
    Lexer lexer(input);
    Parser parser(lexer);

    JsonValue result = parser.parse();

    REQUIRE(result.isObject());

    auto obj = result.asObject();
    REQUIRE(obj.size() == 1);
    REQUIRE(result["name"].isString());
    REQUIRE(result["name"].asString() == "John");
}
TEST_CASE("object with multiple fields", "[parser][object]") {
    std::string input = R"({
        "name": "John",
        "age": 25,
        "student": true,
        "gpa": 3.8,
        "nickname": null
    })";
    Lexer lexer(input);
    Parser parser(lexer);
    JsonValue result = parser.parse();

    REQUIRE(result.isObject());
    auto obj = result.asObject();

    REQUIRE(result["name"].asString() == "John");
    REQUIRE(result["age"].asNumber() == 25);
    REQUIRE(result["student"].asBool() == true);
    REQUIRE(result["gpa"].asNumber() == Catch::Approx(3.8));
    REQUIRE(result["nickname"].isNull());
}
TEST_CASE("missing colon", "[parser][error]") {
    std::string input = R"({"name" "John"})";
    Lexer lexer(input);
    Parser parser(lexer);

    REQUIRE_THROWS_AS(parser.parse(), std::runtime_error);
}
TEST_CASE("invalid JSON", "[parser][error]") {
    REQUIRE_THROWS_AS(
        []() {
            std::string input = R"(invalid)";
            Lexer lexer(input);
            Parser parser(lexer);
            parser.parse();
        }(),
        std::runtime_error
    );
}

TEST_CASE("trailing tokens after JSON value are rejected", "[parser][error]") {
    std::string input = R"({"name": "John"} {"extra": true})";
    Lexer lexer(input);
    Parser parser(lexer);

    REQUIRE_THROWS_AS(parser.parse(), std::runtime_error);
}

TEST_CASE("escaped strings", "[parser][string]") {
    std::string input = R"({"quote": "a\"b", "path": "a\\b", "line": "a\nb"})";
    Lexer lexer(input);
    Parser parser(lexer);

    JsonValue result = parser.parse();

    REQUIRE(result["quote"].asString() == "a\"b");
    REQUIRE(result["path"].asString() == "a\\b");
    REQUIRE(result["line"].asString() == "a\nb");
}

TEST_CASE("nested object", "[parser][object][nested]") {
    std::string input = R"({
        "user": {
            "name": "Alice",
            "age": 30
        }
    })";
    Lexer lexer(input);
    Parser parser(lexer);
    JsonValue result = parser.parse();

    REQUIRE(result["user"]["name"].asString() == "Alice");
    REQUIRE(result["user"]["age"].asNumber() == 30);
}
