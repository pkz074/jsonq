#pragma once
#include <string>
#include <stdexcept>

enum class TokenType {

    LBrace,
    RBrace,
    LBracket,
    RBracket,
    Comma,
    Colon,
    String,
    Number,
    True,
    False,
    Null,
    EoF
};

struct Token {

    TokenType type;
    std::string value;
    size_t line;
    size_t column;
};

class Lexer {

    public:
    Lexer(const std::string& input) : input(input), pos(0), line(1), column(1) {}

    Token nextToken();

    private:
    std::string input;
    size_t pos;
    size_t line;
    size_t column;

    char advance();
    std::runtime_error errorAt(size_t errorLine, size_t errorColumn, const std::string& message) const;
    std::runtime_error errorHere(const std::string& message) const;
    unsigned int readUnicodeEscape();
};
