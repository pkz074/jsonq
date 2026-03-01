#pragma once
#include "JsonValue.hpp"
#include "../lexer/lexer.hpp"

class Parser {

    public:
    Parser(Lexer lexer);
    JsonValue parse();

    private:
    Lexer lexer;
    Token current;

    void advance(); // fetches the next token
    JsonValue parseValue();
    JsonValue parseObject();
    JsonValue parseArray();
    void expect(TokenType type);


};
