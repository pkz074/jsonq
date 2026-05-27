#include "schema/SchemaValidator.hpp"

namespace {

const JsonValue* findField(const JsonValue& object, const std::string& key) {
    for (const auto& pair : object.asObject()) {
        if (pair.first == key) {
            return &pair.second;
        }
    }

    return nullptr;
}

std::string typeName(const JsonValue& value) {
    if (value.isString()) {
        return "string";
    }
    if (value.isNumber()) {
        return "number";
    }
    if (value.isBool()) {
        return "bool";
    }
    if (value.isNull()) {
        return "null";
    }
    if (value.isArray()) {
        return "array";
    }
    return "object";
}

bool isKnownType(const std::string& type) {
    return type == "string" ||
           type == "number" ||
           type == "bool" ||
           type == "null" ||
           type == "array" ||
           type == "object";
}

bool matchesType(const JsonValue& value, const std::string& type) {
    if (type == "string") {
        return value.isString();
    }
    if (type == "number") {
        return value.isNumber();
    }
    if (type == "bool") {
        return value.isBool();
    }
    if (type == "null") {
        return value.isNull();
    }
    if (type == "array") {
        return value.isArray();
    }
    if (type == "object") {
        return value.isObject();
    }

    return false;
}

std::string fieldPath(const std::string& prefix, const std::string& fieldName) {
    if (prefix.empty()) {
        return fieldName;
    }

    return prefix + "." + fieldName;
}

void validateSchemaObject(const JsonValue& schema, const std::string& pathPrefix, SchemaResult& result) {
    for (const auto& rule : schema.asObject()) {
        const std::string& fieldName = rule.first;
        const std::string path = fieldPath(pathPrefix, fieldName);
        const JsonValue& expectedTypeValue = rule.second;

        if (expectedTypeValue.isObject()) {
            validateSchemaObject(expectedTypeValue, path, result);
            continue;
        }

        if (!expectedTypeValue.isString()) {
            result.valid = false;
            result.errors.push_back("schema field " + path + " must be a type string or object schema");
            continue;
        }

        std::string expectedType = expectedTypeValue.asString();
        if (!isKnownType(expectedType)) {
            result.valid = false;
            result.errors.push_back("schema field " + path + " has unknown type: " + expectedType);
        }
    }
}

void validateObject(const JsonValue& document, const JsonValue& schema, const std::string& pathPrefix, SchemaResult& result) {
    for (const auto& rule : schema.asObject()) {
        const std::string& fieldName = rule.first;
        const std::string path = fieldPath(pathPrefix, fieldName);
        const JsonValue& expectedTypeValue = rule.second;

        const JsonValue* actualValue = findField(document, fieldName);
        if (actualValue == nullptr) {
            result.valid = false;
            result.errors.push_back("missing required field: " + path);
            continue;
        }

        if (expectedTypeValue.isObject()) {
            if (!actualValue->isObject()) {
                result.valid = false;
                result.errors.push_back("field " + path + " expected object but got " + typeName(*actualValue));
                continue;
            }

            validateObject(*actualValue, expectedTypeValue, path, result);
            continue;
        }

        std::string expectedType = expectedTypeValue.asString();
        if (!matchesType(*actualValue, expectedType)) {
            result.valid = false;
            result.errors.push_back("field " + path + " expected " + expectedType + " but got " + typeName(*actualValue));
        }
    }
}

}

SchemaResult SchemaValidator::validate(const JsonValue& document, const JsonValue& schema) const {
    SchemaResult result{true, {}};

    if (!schema.isObject()) {
        result.valid = false;
        result.errors.push_back("schema must be an object");
        return result;
    }

    if (!document.isObject()) {
        result.valid = false;
        result.errors.push_back("document must be an object");
        return result;
    }

    validateSchemaObject(schema, "", result);
    if (!result.valid) {
        return result;
    }

    validateObject(document, schema, "", result);

    return result;
}
