#include <catch2/catch_test_macros.hpp>
#include "schema/SchemaValidator.hpp"

TEST_CASE("schema validator accepts matching primitive fields", "[schema]") {
    SchemaValidator validator;
    JsonValue schema{JsonObject{
        {"name", JsonValue{"string"}},
        {"age", JsonValue{"number"}},
        {"active", JsonValue{"bool"}},
        {"nickname", JsonValue{"null"}},
    }};
    JsonValue document{JsonObject{
        {"name", JsonValue{"Alice"}},
        {"age", JsonValue{30.0}},
        {"active", JsonValue{true}},
        {"nickname", JsonValue{nullptr}},
    }};

    SchemaResult result = validator.validate(document, schema);

    REQUIRE(result.valid);
    REQUIRE(result.errors.empty());
}

TEST_CASE("schema validator reports missing required fields", "[schema]") {
    SchemaValidator validator;
    JsonValue schema{JsonObject{
        {"name", JsonValue{"string"}},
        {"age", JsonValue{"number"}},
    }};
    JsonValue document{JsonObject{
        {"name", JsonValue{"Alice"}},
    }};

    SchemaResult result = validator.validate(document, schema);

    REQUIRE_FALSE(result.valid);
    REQUIRE(result.errors.size() == 1);
    REQUIRE(result.errors[0] == "missing required field: age");
}

TEST_CASE("schema validator reports type mismatches", "[schema]") {
    SchemaValidator validator;
    JsonValue schema{JsonObject{
        {"name", JsonValue{"string"}},
        {"age", JsonValue{"number"}},
        {"active", JsonValue{"bool"}},
    }};
    JsonValue document{JsonObject{
        {"name", JsonValue{false}},
        {"age", JsonValue{"30"}},
        {"active", JsonValue{nullptr}},
    }};

    SchemaResult result = validator.validate(document, schema);

    REQUIRE_FALSE(result.valid);
    REQUIRE(result.errors.size() == 3);
    REQUIRE(result.errors[0] == "field name expected string but got bool");
    REQUIRE(result.errors[1] == "field age expected number but got string");
    REQUIRE(result.errors[2] == "field active expected bool but got null");
}

TEST_CASE("schema validator rejects invalid schema documents", "[schema]") {
    SchemaValidator validator;

    SchemaResult nonObjectSchema = validator.validate(JsonValue{JsonObject{}}, JsonValue{"string"});
    SchemaResult invalidTypeName = validator.validate(
        JsonValue{JsonObject{{"name", JsonValue{"Alice"}}}},
        JsonValue{JsonObject{{"name", JsonValue{"text"}}}}
    );
    SchemaResult nonStringRule = validator.validate(
        JsonValue{JsonObject{{"age", JsonValue{30.0}}}},
        JsonValue{JsonObject{{"age", JsonValue{30.0}}}}
    );

    REQUIRE_FALSE(nonObjectSchema.valid);
    REQUIRE(nonObjectSchema.errors[0] == "schema must be an object");

    REQUIRE_FALSE(invalidTypeName.valid);
    REQUIRE(invalidTypeName.errors[0] == "schema field name has unknown type: text");

    REQUIRE_FALSE(nonStringRule.valid);
    REQUIRE(nonStringRule.errors[0] == "schema field age must be a type string or object schema");
}

TEST_CASE("schema validator accepts nested object schemas", "[schema]") {
    SchemaValidator validator;
    JsonValue schema{JsonObject{
        {"name", JsonValue{"string"}},
        {"profile", JsonValue{JsonObject{
            {"city", JsonValue{"string"}},
            {"active", JsonValue{"bool"}},
        }}},
    }};
    JsonValue document{JsonObject{
        {"name", JsonValue{"Alice"}},
        {"profile", JsonValue{JsonObject{
            {"city", JsonValue{"Toronto"}},
            {"active", JsonValue{true}},
        }}},
    }};

    SchemaResult result = validator.validate(document, schema);

    REQUIRE(result.valid);
    REQUIRE(result.errors.empty());
}

TEST_CASE("schema validator reports nested schema errors with paths", "[schema][error]") {
    SchemaValidator validator;
    JsonValue schema{JsonObject{
        {"profile", JsonValue{JsonObject{
            {"city", JsonValue{"string"}},
            {"active", JsonValue{"bool"}},
        }}},
    }};
    JsonValue document{JsonObject{
        {"profile", JsonValue{JsonObject{
            {"city", JsonValue{42.0}},
        }}},
    }};

    SchemaResult result = validator.validate(document, schema);

    REQUIRE_FALSE(result.valid);
    REQUIRE(result.errors.size() == 2);
    REQUIRE(result.errors[0] == "field profile.city expected string but got number");
    REQUIRE(result.errors[1] == "missing required field: profile.active");
}

TEST_CASE("schema validator requires document fields to match nested schema objects", "[schema][error]") {
    SchemaValidator validator;
    JsonValue schema{JsonObject{
        {"profile", JsonValue{JsonObject{
            {"city", JsonValue{"string"}},
        }}},
    }};
    JsonValue document{JsonObject{
        {"profile", JsonValue{"Toronto"}},
    }};

    SchemaResult result = validator.validate(document, schema);

    REQUIRE_FALSE(result.valid);
    REQUIRE(result.errors.size() == 1);
    REQUIRE(result.errors[0] == "field profile expected object but got string");
}

TEST_CASE("schema validator rejects invalid nested schema documents", "[schema][error]") {
    SchemaValidator validator;
    JsonValue schema{JsonObject{
        {"profile", JsonValue{JsonObject{
            {"city", JsonValue{"text"}},
            {"age", JsonValue{30.0}},
        }}},
    }};
    JsonValue document{JsonObject{}};

    SchemaResult result = validator.validate(document, schema);

    REQUIRE_FALSE(result.valid);
    REQUIRE(result.errors.size() == 2);
    REQUIRE(result.errors[0] == "schema field profile.city has unknown type: text");
    REQUIRE(result.errors[1] == "schema field profile.age must be a type string or object schema");
}
