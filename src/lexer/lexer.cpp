#include "lexer.hpp"

char Lexer::advance() {
    char c = input[pos];
    pos++;

    if (c == '\n') {
        line++;
        column = 1;
    }
    else {
        column++;
    }

    return c;
}

Token Lexer::nextToken() {

    while (pos < input.size() && std::isspace(input[pos])) {
        advance();
    }


    if (pos >= input.size())
        return Token{TokenType::EoF, "", line, column};

    size_t tokenLine = line;
    size_t tokenColumn = column;
    char c = input[pos];

    if (c == '{') {advance(); return Token{TokenType::LBrace, "", tokenLine, tokenColumn};}
    if (c == '}') {advance(); return Token{TokenType::RBrace, "", tokenLine, tokenColumn};}
    if (c == '[') {advance(); return Token{TokenType::LBracket, "", tokenLine, tokenColumn};}
    if (c == ']') {advance(); return Token{TokenType::RBracket, "", tokenLine, tokenColumn};}
    if (c == ':') {advance(); return Token{TokenType::Colon, "", tokenLine, tokenColumn};}
    if (c == ',') {advance(); return Token{TokenType::Comma, "", tokenLine, tokenColumn};}

    if (c == '"'){

        std::string buffer;
        advance();
        while (pos < input.size() && input[pos] != '"') {
            if (input[pos] == '\\') {
                advance();

                if (pos >= input.size()) {
                    throw std::runtime_error("unterminated string escape");
                }

                switch (input[pos]) {
                    case '"':
                        buffer += '"';
                        break;
                    case '\\':
                        buffer += '\\';
                        break;
                    case '/':
                        buffer += '/';
                        break;
                    case 'b':
                        buffer += '\b';
                        break;
                    case 'f':
                        buffer += '\f';
                        break;
                    case 'n':
                        buffer += '\n';
                        break;
                    case 'r':
                        buffer += '\r';
                        break;
                    case 't':
                        buffer += '\t';
                        break;
                    default:
                        throw std::runtime_error("invalid string escape");
                }
            }
            else {
                buffer += input[pos];
            }

            advance();
        }
        if (pos >= input.size())
            throw std::runtime_error("unterminated string");
        advance();

        return Token{TokenType::String, buffer, tokenLine, tokenColumn};
    }

    if (std::isdigit(c) || c == '-') {

        std::string buffer;

        if (input[pos] == '-') {
            buffer += input[pos];
            advance();

            if (pos >= input.size() || !std::isdigit(static_cast<unsigned char>(input[pos]))) {
                throw std::runtime_error("invalid number");
            }
        }

        if (input[pos] == '0') {
            buffer += input[pos];
            advance();

            if (pos < input.size() && std::isdigit(static_cast<unsigned char>(input[pos]))) {
                throw std::runtime_error("invalid number");
            }
        }
        else {
            while (pos < input.size() && std::isdigit(static_cast<unsigned char>(input[pos]))) {
                buffer += input[pos];
                advance();
            }
        }

        if (pos < input.size() && input[pos] == '.') {
            buffer += input[pos];
            advance();

            if (pos >= input.size() || !std::isdigit(static_cast<unsigned char>(input[pos]))) {
                throw std::runtime_error("invalid number");
            }

            while (pos < input.size() && std::isdigit(static_cast<unsigned char>(input[pos]))) {
                buffer += input[pos];
                advance();
            }
        }

        if (pos < input.size() && (input[pos] == 'e' || input[pos] == 'E')) {
            buffer += input[pos];
            advance();

            if (pos < input.size() && (input[pos] == '+' || input[pos] == '-')) {
                buffer += input[pos];
                advance();
            }

            if (pos >= input.size() || !std::isdigit(static_cast<unsigned char>(input[pos]))) {
                throw std::runtime_error("invalid number");
            }

            while (pos < input.size() && std::isdigit(static_cast<unsigned char>(input[pos]))) {
                buffer += input[pos];
                advance();
            }
        }

        if (pos < input.size() && (input[pos] == '.' || std::isalpha(static_cast<unsigned char>(input[pos])))) {
            throw std::runtime_error("invalid number");
        }

        return Token{TokenType::Number, buffer, tokenLine, tokenColumn};
    }

    if (c == 't') {
        if (input.substr(pos, 4) == "true") {
            advance();
            advance();
            advance();
            advance();
            return Token{TokenType::True, "", tokenLine, tokenColumn};
        }
        throw std::runtime_error("unexpected character");
    }

    if (c == 'f') {
        if (input.substr(pos, 5) == "false") {
            advance();
            advance();
            advance();
            advance();
            advance();
            return Token{TokenType::False, "", tokenLine, tokenColumn};
        }
        throw std::runtime_error("unexpected character");
    }

    if (c == 'n') {
        if (input.substr(pos, 4) == "null") {
            advance();
            advance();
            advance();
            advance();
            return Token{TokenType::Null, "", tokenLine, tokenColumn};
        }
        throw std::runtime_error("unexpected character");
    }

    throw std::runtime_error("unexpected character");

}
