#include "lexer.hpp"
#include <cctype>
#include <sstream>
#include <stdexcept>

namespace {

int hexValue(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return 10 + (c - 'a');
    }
    if (c >= 'A' && c <= 'F') {
        return 10 + (c - 'A');
    }
    return -1;
}

bool isHighSurrogate(unsigned int codePoint) {
    return codePoint >= 0xD800 && codePoint <= 0xDBFF;
}

bool isLowSurrogate(unsigned int codePoint) {
    return codePoint >= 0xDC00 && codePoint <= 0xDFFF;
}

std::string encodeUtf8(unsigned int codePoint) {
    std::string result;

    if (codePoint <= 0x7F) {
        result += static_cast<char>(codePoint);
    }
    else if (codePoint <= 0x7FF) {
        result += static_cast<char>(0xC0 | (codePoint >> 6));
        result += static_cast<char>(0x80 | (codePoint & 0x3F));
    }
    else if (codePoint <= 0xFFFF) {
        result += static_cast<char>(0xE0 | (codePoint >> 12));
        result += static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
        result += static_cast<char>(0x80 | (codePoint & 0x3F));
    }
    else {
        result += static_cast<char>(0xF0 | (codePoint >> 18));
        result += static_cast<char>(0x80 | ((codePoint >> 12) & 0x3F));
        result += static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
        result += static_cast<char>(0x80 | (codePoint & 0x3F));
    }

    return result;
}

}

std::runtime_error Lexer::errorAt(size_t errorLine, size_t errorColumn, const std::string& message) const {
    std::ostringstream oss;
    oss << "line " << errorLine << ", column " << errorColumn << ": " << message;
    return std::runtime_error(oss.str());
}

std::runtime_error Lexer::errorHere(const std::string& message) const {
    return errorAt(line, column, message);
}

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

unsigned int Lexer::readUnicodeEscape() {
    unsigned int codePoint = 0;

    for (int i = 0; i < 4; i++) {
        if (pos >= input.size()) {
            throw errorHere("unterminated unicode escape");
        }

        int digit = hexValue(input[pos]);
        if (digit < 0) {
            throw errorHere("invalid unicode escape");
        }

        codePoint = (codePoint << 4) | static_cast<unsigned int>(digit);
        advance();
    }

    return codePoint;
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
                    throw errorHere("unterminated string escape");
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
                    case 'u': {
                        advance();
                        unsigned int codePoint = readUnicodeEscape();

                        if (isHighSurrogate(codePoint)) {
                            if (pos + 1 >= input.size() || input[pos] != '\\' || input[pos + 1] != 'u') {
                                throw errorHere("expected low surrogate after high surrogate");
                            }

                            advance();
                            advance();
                            unsigned int lowSurrogate = readUnicodeEscape();
                            if (!isLowSurrogate(lowSurrogate)) {
                                throw errorHere("expected low surrogate after high surrogate");
                            }

                            codePoint = 0x10000 + (((codePoint - 0xD800) << 10) | (lowSurrogate - 0xDC00));
                        }
                        else if (isLowSurrogate(codePoint)) {
                            throw errorHere("unexpected low surrogate");
                        }

                        buffer += encodeUtf8(codePoint);
                        continue;
                    }
                    default:
                        throw errorHere("invalid string escape");
                }
            }
            else {
                buffer += input[pos];
            }

            advance();
        }
        if (pos >= input.size())
            throw errorAt(tokenLine, tokenColumn, "unterminated string");
        advance();

        return Token{TokenType::String, buffer, tokenLine, tokenColumn};
    }

    if (std::isdigit(c) || c == '-') {

        std::string buffer;

        if (input[pos] == '-') {
            buffer += input[pos];
            advance();

            if (pos >= input.size() || !std::isdigit(static_cast<unsigned char>(input[pos]))) {
                throw errorAt(tokenLine, tokenColumn, "invalid number");
            }
        }

        if (input[pos] == '0') {
            buffer += input[pos];
            advance();

            if (pos < input.size() && std::isdigit(static_cast<unsigned char>(input[pos]))) {
                throw errorAt(tokenLine, tokenColumn, "invalid number");
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
                throw errorAt(tokenLine, tokenColumn, "invalid number");
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
                throw errorAt(tokenLine, tokenColumn, "invalid number");
            }

            while (pos < input.size() && std::isdigit(static_cast<unsigned char>(input[pos]))) {
                buffer += input[pos];
                advance();
            }
        }

        if (pos < input.size() && (input[pos] == '.' || std::isalpha(static_cast<unsigned char>(input[pos])))) {
            throw errorAt(tokenLine, tokenColumn, "invalid number");
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
        throw errorAt(tokenLine, tokenColumn, "unexpected character");
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
        throw errorAt(tokenLine, tokenColumn, "unexpected character");
    }

    if (c == 'n') {
        if (input.substr(pos, 4) == "null") {
            advance();
            advance();
            advance();
            advance();
            return Token{TokenType::Null, "", tokenLine, tokenColumn};
        }
        throw errorAt(tokenLine, tokenColumn, "unexpected character");
    }

    throw errorAt(tokenLine, tokenColumn, "unexpected character");

}
