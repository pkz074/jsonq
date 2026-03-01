#include "lexer.hpp"

Token Lexer::nextToken() {


    while (pos < input.size() && std::isspace(input[pos])) {
        pos++;
    }


    if (pos >= input.size())
        return Token{TokenType::EoF, "", 0};

    char c = input[pos];

    // could implement a switch as well

    if (c == '{') {pos++; return Token{TokenType::LBrace, "", 0};}
    if (c == '}') {pos++; return Token{TokenType::RBrace, "", 0};}
    if (c == '[') {pos++; return Token{TokenType::LBracket, "", 0};}
    if (c == ']') {pos++; return Token{TokenType::RBracket, "", 0};}
    if (c == ':') {pos++; return Token{TokenType::Colon, "", 0};}
    if (c == ',') {pos++; return Token{TokenType::Comma, "", 0};}

    if (c == '"'){

        std::string buffer;
        pos++;
        while (pos < input.size() && input[pos] != '"') {
            buffer += input[pos];
            pos ++;
        }
        if (pos >= input.size())
            throw std::runtime_error("unterminated string");
        pos++;

        return Token{TokenType::String, buffer, 0};
    }

    if (std::isdigit(c) || c == '-') {

        std::string buffer;
        buffer += c;
        pos++;
        while (pos < input.size() && (std::isdigit(input[pos]) || input[pos] == '.')) {
            buffer += input[pos];
            pos++;
        }

        return Token{TokenType::Number, buffer, 0};
    }

    if (c == 't') {
        if (input.substr(pos, 4) == "true") { pos += 4; return Token{TokenType::True, "", 0}; }
        throw std::runtime_error("unexpected character");
    }

    if (c == 'f') {
        if (input.substr(pos, 5) == "false") { pos += 5; return Token{TokenType::False, "", 0}; }
        throw std::runtime_error("unexpected character");
    }

    if (c == 'n') {
        if (input.substr(pos, 4) == "null") { pos += 4; return Token{TokenType::Null, "", 0}; }
        throw std::runtime_error("unexpected character");
    }

    throw std::runtime_error("unexpected character");

}
