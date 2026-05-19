#include <catch2/catch_test_macros.hpp>
#include "printer/Printer.hpp"

TEST_CASE("printer formats scalar values", "[printer]") {
    Printer printer;

    REQUIRE(printer.print(JsonValue{nullptr}) == "null");
    REQUIRE(printer.print(JsonValue{true}) == "true");
    REQUIRE(printer.print(JsonValue{false}) == "false");
    REQUIRE(printer.print(JsonValue{30.0}) == "30");
    REQUIRE(printer.print(JsonValue{3.8}) == "3.8");
    REQUIRE(printer.print(JsonValue{"Alice"}) == R"("Alice")");
}

TEST_CASE("printer escapes strings", "[printer]") {
    Printer printer;

    REQUIRE(printer.print(JsonValue{"a\"b"}) == R"("a\"b")");
    REQUIRE(printer.print(JsonValue{"a\\b"}) == R"("a\\b")");
    REQUIRE(printer.print(JsonValue{"a\nb"}) == R"("a\nb")");
}

TEST_CASE("printer formats empty containers compactly", "[printer]") {
    Printer printer;

    REQUIRE(printer.print(JsonValue{JsonArray{}}) == "[]");
    REQUIRE(printer.print(JsonValue{JsonObject{}}) == "{}");
}

TEST_CASE("printer formats arrays and objects", "[printer]") {
    Printer printer;
    JsonObject object{
        {"name", JsonValue{"Alice"}},
        {"age", JsonValue{30.0}},
        {"hobbies", JsonValue{JsonArray{JsonValue{"reading"}, JsonValue{"coding"}}}},
    };

    REQUIRE(printer.print(JsonValue{object}) == R"({
  "name": "Alice",
  "age": 30,
  "hobbies": [
    "reading",
    "coding"
  ]
})");
}
