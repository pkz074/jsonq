#include "Parser.hpp"
#include <stdexcept>
#include "JsonValue.hpp"
#include <sstream>
#include <vector>

namespace {

std::string errorAt(const Token& token, const std::string& message) {
    std::ostringstream oss;
    oss << "line " << token.line << ", column " << token.column << ": " << message;
    return oss.str();
}

}

Parser::Parser(Lexer lexer) : lexer(lexer) {
    advance();
}

void Parser::advance() {

    current = lexer.nextToken();
}

void Parser::expect(TokenType type) {

    if (current.type != type)
        throw std::runtime_error(errorAt(current, "unexpected token"));
    advance();


}

JsonValue Parser::parse() {
    JsonValue value = parseValue();

    if (current.type != TokenType::EoF) {
        throw std::runtime_error(errorAt(current, "unexpected token after JSON value"));
    }

    return value;
}


JsonValue Parser::parseObject() {

    JsonObject obj;
    advance(); // we start with "{"

    if (current.type == TokenType::RBrace){
        advance();
        return JsonValue{obj};
    }
    while(true){

        std::string key = current.value;
        expect(TokenType::String);
        expect(TokenType::Colon);
        JsonValue val = parseValue();
        obj.push_back({key, val});

        if (current.type == TokenType::Comma) {
            advance();
        } else if (current.type == TokenType::RBrace){ // the last value doesnt have comma
            advance();
            return JsonValue{obj};
        } else {
            throw std::runtime_error(errorAt(current, "expected ',' or '}'"));
        }

    }
}
JsonValue Parser::parseArray()  {

    JsonArray arr;
    advance();

    if (current.type == TokenType::RBracket){
        advance();
        return JsonValue{arr};
    }
    while(true){

        JsonValue val = parseValue();;
        arr.push_back(val);

        if (current.type == TokenType::Comma){
            advance();
        } else if (current.type == TokenType::RBracket){
            advance();
            return JsonValue{arr};
        } else {
            throw std::runtime_error(errorAt(current, "expected ',' or ']'"));
        }
    }
}

JsonValue Parser::parseValue() {

    switch (current.type) {
        case TokenType::LBrace: return parseObject();
        case TokenType::LBracket: return parseArray();
        case TokenType::String: {std::string val = current.value; advance(); return JsonValue{val};}
        case TokenType::Number: {double val = std::stod(current.value); advance(); return JsonValue{val};}
        case TokenType::True: advance(); return JsonValue{true};
        case TokenType::False: advance(); return JsonValue{false};
        case TokenType::Null: advance(); return JsonValue{nullptr};
        default: throw std::runtime_error(errorAt(current, "unexpected token"));
    }
}
