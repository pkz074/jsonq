#include "Parser.hpp"
#include <stdexcept>
#include "JsonValue.hpp"

Parser::Parser(Lexer lexer) : lexer(lexer) {
    advance();
}

void Parser::advance() {

    current = lexer.nextToken();
}

void Parser::expect() {


}

JsonValue Parser::parse() {
    return parseValue();
}


JsonValue Parser::parseObject() { throw std::runtime_error("not implemented"); }
JsonValue Parser::parseArray()  { throw std::runtime_error("not implemented"); }

JsonValue Parser::parseValue() {

    switch (current.type) {
        case TokenType::LBrace: return parseObject();
        case TokenType::LBracket: return parseArray();
        case TokenType::String: {std::string val = current.value; advance(); return JsonValue{val};}
        case TokenType::Number: {double val = std::stod(current.value); advance(); return JsonValue{val};}
        case TokenType::True: advance(); return JsonValue{true};
        case TokenType::False: advance(); return JsonValue{false};
        case TokenType::Null: advance(); return JsonValue{nullptr};
        default: throw std::runtime_error("Unexpected token");
    }
}
