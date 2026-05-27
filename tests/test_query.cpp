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

TEST_CASE("query parser returns expected step kinds", "[query][parser]") {
    QueryEngine engine;
    PathExpr path = engine.parse(R"($.users[*]["display name"])");

    REQUIRE(path.size() == 3);
    REQUIRE(path[0].kind == Step::Kind::Key);
    REQUIRE(path[0].key == "users");
    REQUIRE(path[1].kind == Step::Kind::Wildcard);
    REQUIRE(path[2].kind == Step::Kind::Key);
    REQUIRE(path[2].key == "display name");
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

TEST_CASE("query quoted object fields", "[query]") {
    std::string json = R"({
        "display name": "Alice",
        "profile.name": "alice.dev",
        "quote\"key": true,
        "slash\\key": 42
    })";

    REQUIRE(queryJson(json, R"($["display name"])").asString() == "Alice");
    REQUIRE(queryJson(json, R"($["profile.name"])").asString() == "alice.dev");
    REQUIRE(queryJson(json, R"($["quote\"key"])").asBool());
    REQUIRE(queryJson(json, R"($["slash\\key"])").asNumber() == Catch::Approx(42));
}

TEST_CASE("query quoted field after wildcard", "[query][wildcard]") {
    std::string json = R"({
        "users": [
            {"display name": "Alice"},
            {"display name": "Bob"}
        ]
    })";

    JsonValue result = queryJson(json, R"($.users[*]["display name"])");

    REQUIRE(result.isArray());
    REQUIRE(result.asArray().size() == 2);
    REQUIRE(result.asArray()[0].asString() == "Alice");
    REQUIRE(result.asArray()[1].asString() == "Bob");
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

TEST_CASE("query object wildcard", "[query][wildcard]") {
    std::string json = R"({"name": "Alice", "city": "Toronto"})";

    JsonValue result = queryJson(json, "$.*");

    REQUIRE(result.isArray());
    REQUIRE(result.asArray().size() == 2);
    REQUIRE(result.asArray()[0].asString() == "Alice");
    REQUIRE(result.asArray()[1].asString() == "Toronto");
}

TEST_CASE("query path after object wildcard", "[query][wildcard]") {
    std::string json = R"({
        "primary": {"name": "Alice"},
        "secondary": {"name": "Bob"}
    })";

    JsonValue result = queryJson(json, "$.*.name");

    REQUIRE(result.isArray());
    REQUIRE(result.asArray().size() == 2);
    REQUIRE(result.asArray()[0].asString() == "Alice");
    REQUIRE(result.asArray()[1].asString() == "Bob");
}

TEST_CASE("query filters arrays by number field", "[query][filter]") {
    std::string json = R"({
        "users": [
            {"name": "Alice", "age": 30},
            {"name": "Bob", "age": 25},
            {"name": "Carol", "age": 35}
        ]
    })";

    JsonValue result = queryJson(json, "$.users[?age>25].name");

    REQUIRE(result.isArray());
    REQUIRE(result.asArray().size() == 2);
    REQUIRE(result.asArray()[0].asString() == "Alice");
    REQUIRE(result.asArray()[1].asString() == "Carol");
}

TEST_CASE("query filters arrays by string field", "[query][filter]") {
    std::string json = R"({
        "users": [
            {"name": "Alice", "role": "admin"},
            {"name": "Bob", "role": "user"}
        ]
    })";

    JsonValue result = queryJson(json, R"($.users[?role=="admin"].name)");

    REQUIRE(result.isArray());
    REQUIRE(result.asArray().size() == 1);
    REQUIRE(result.asArray()[0].asString() == "Alice");
}

TEST_CASE("query filters arrays by boolean field", "[query][filter]") {
    std::string json = R"({
        "users": [
            {"name": "Alice", "active": true},
            {"name": "Bob", "active": false}
        ]
    })";

    JsonValue result = queryJson(json, "$.users[?active==true].name");

    REQUIRE(result.isArray());
    REQUIRE(result.asArray().size() == 1);
    REQUIRE(result.asArray()[0].asString() == "Alice");
}

TEST_CASE("query filters arrays by quoted field", "[query][filter]") {
    std::string json = R"({
        "users": [
            {"display name": "Alice", "score.total": 10},
            {"display name": "Bob", "score.total": 4}
        ]
    })";

    JsonValue byName = queryJson(json, R"($.users[?"display name"=="Alice"]["score.total"])");
    JsonValue byScore = queryJson(json, R"($.users[?"score.total">=10]["display name"])");

    REQUIRE(byName.isArray());
    REQUIRE(byName.asArray().size() == 1);
    REQUIRE(byName.asArray()[0].asNumber() == Catch::Approx(10));

    REQUIRE(byScore.isArray());
    REQUIRE(byScore.asArray().size() == 1);
    REQUIRE(byScore.asArray()[0].asString() == "Alice");
}

TEST_CASE("query filters arrays with not equal less equal and null", "[query][filter]") {
    std::string json = R"({
        "users": [
            {"name": "Alice", "age": 30, "deleted": null},
            {"name": "Bob", "age": 25, "deleted": true},
            {"name": "Carol", "age": 35, "deleted": null}
        ]
    })";

    JsonValue notBob = queryJson(json, R"($.users[?name!="Bob"].name)");
    JsonValue young = queryJson(json, "$.users[?age<=30].name");
    JsonValue active = queryJson(json, "$.users[?deleted==null].name");

    REQUIRE(notBob.asArray().size() == 2);
    REQUIRE(notBob.asArray()[0].asString() == "Alice");
    REQUIRE(notBob.asArray()[1].asString() == "Carol");

    REQUIRE(young.asArray().size() == 2);
    REQUIRE(young.asArray()[0].asString() == "Alice");
    REQUIRE(young.asArray()[1].asString() == "Bob");

    REQUIRE(active.asArray().size() == 2);
    REQUIRE(active.asArray()[0].asString() == "Alice");
    REQUIRE(active.asArray()[1].asString() == "Carol");
}

TEST_CASE("query invalid syntax throws", "[query][error]") {
    QueryEngine engine;

    REQUIRE_THROWS_AS(engine.parse("name"), std::runtime_error);
    REQUIRE_THROWS_AS(engine.parse("$."), std::runtime_error);
    REQUIRE_THROWS_AS(engine.parse("$.hobbies["), std::runtime_error);
    REQUIRE_THROWS_AS(engine.parse("$.hobbies[x]"), std::runtime_error);
    REQUIRE_THROWS_AS(engine.parse(R"($["unterminated])"), std::runtime_error);
    REQUIRE_THROWS_AS(engine.parse(R"($["missing close")"), std::runtime_error);
    REQUIRE_THROWS_AS(engine.parse("$.users[?age]"), std::runtime_error);
    REQUIRE_THROWS_AS(engine.parse("$.users[?>25]"), std::runtime_error);
}

TEST_CASE("query missing values throw", "[query][error]") {
    QueryEngine engine;
    JsonValue root = parseJson(R"({"hobbies": ["reading"]})");

    REQUIRE_THROWS_AS(engine.evaluate(root, engine.parse("$.missing")), std::runtime_error);
    REQUIRE_THROWS_AS(engine.evaluate(root, engine.parse("$.hobbies[3]")), std::runtime_error);
    REQUIRE_THROWS_AS(engine.evaluate(root, engine.parse("$.hobbies[0].*")), std::runtime_error);
    REQUIRE_THROWS_AS(engine.evaluate(root, engine.parse("$.hobbies[0][?age>25]")), std::runtime_error);
}
