#include <catch2/catch_test_macros.hpp>
#include "lexer/lexer.hpp"

TEST_CASE("single character tokens") {
    Lexer lexer("{");
    Token t = lexer.nextToken();
    REQUIRE(t.type == TokenType::LBrace);
}
