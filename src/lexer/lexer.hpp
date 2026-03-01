
#include <string>

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
};

class Lexer {

    public:
    Lexer(const std::string& input) : input(input), pos(0) {}

    Token nextToken();

    private:
    std::string input;
    size_t pos;
};
