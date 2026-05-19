#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "parser/Parser.hpp"
#include "query/QueryEngine.hpp"

static JsonValue parseJson(const std::string& input) {
    Lexer lexer(input);
    Parser parser(lexer);
    return parser.parse();
}

static JsonValue queryJson(const std::string& json, const std::string& query) {
    QueryEngine engine;
    JsonValue root = parseJson(json);
    PathExpr path = engine.parse(query);
    return engine.evaluate(root, path);
}

TEST_CASE("query root returns whole document", "[query]") {
    JsonValue result = queryJson(R"({"name": "Alice"})", "$");

    REQUIRE(result.isObject());
    REQUIRE(result["name"].asString() == "Alice");
}

TEST_CASE("query object fields", "[query]") {
    std::string json = R"({"name": "Alice", "age": 30})";

    REQUIRE(queryJson(json, "$.name").asString() == "Alice");
    REQUIRE(queryJson(json, "$.age").asNumber() == Catch::Approx(30));
}

TEST_CASE("query nested object fields", "[query]") {
    std::string json = R"({"user": {"name": "Alice", "active": true}})";

    REQUIRE(queryJson(json, "$.user.name").asString() == "Alice");
    REQUIRE(queryJson(json, "$.user.active").asBool());
}

TEST_CASE("query array index", "[query]") {
    std::string json = R"({"hobbies": ["reading", "coding"]})";

    REQUIRE(queryJson(json, "$.hobbies[0]").asString() == "reading");
    REQUIRE(queryJson(json, "$.hobbies[1]").asString() == "coding");
}

TEST_CASE("query array wildcard", "[query][wildcard]") {
    std::string json = R"({"hobbies": ["reading", "coding"]})";

    JsonValue result = queryJson(json, "$.hobbies[*]");

    REQUIRE(result.isArray());
    REQUIRE(result.asArray().size() == 2);
    REQUIRE(result.asArray()[0].asString() == "reading");
    REQUIRE(result.asArray()[1].asString() == "coding");
}

TEST_CASE("query path after array wildcard", "[query][wildcard]") {
    std::string json = R"({
        "users": [
            {"name": "Alice", "age": 30},
            {"name": "Bob", "age": 25}
        ]
    })";

    JsonValue result = queryJson(json, "$.users[*].name");

    REQUIRE(result.isArray());
    REQUIRE(result.asArray().size() == 2);
    REQUIRE(result.asArray()[0].asString() == "Alice");
    REQUIRE(result.asArray()[1].asString() == "Bob");
}

TEST_CASE("query invalid syntax throws", "[query][error]") {
    QueryEngine engine;

    REQUIRE_THROWS_AS(engine.parse("name"), std::runtime_error);
    REQUIRE_THROWS_AS(engine.parse("$."), std::runtime_error);
    REQUIRE_THROWS_AS(engine.parse("$.hobbies["), std::runtime_error);
    REQUIRE_THROWS_AS(engine.parse("$.hobbies[x]"), std::runtime_error);
}

TEST_CASE("query missing values throw", "[query][error]") {
    QueryEngine engine;
    JsonValue root = parseJson(R"({"hobbies": ["reading"]})");

    REQUIRE_THROWS_AS(engine.evaluate(root, engine.parse("$.missing")), std::runtime_error);
    REQUIRE_THROWS_AS(engine.evaluate(root, engine.parse("$.hobbies[3]")), std::runtime_error);
    REQUIRE_THROWS_AS(engine.evaluate(root, engine.parse("$.hobbies[0][*]")), std::runtime_error);
}
