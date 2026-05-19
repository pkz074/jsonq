#include <catch2/catch_test_macros.hpp>
#include "lexer/lexer.hpp"

TEST_CASE("single character tokens") {
    Lexer lexer("{");
    Token t = lexer.nextToken();
    REQUIRE(t.type == TokenType::LBrace);
}

TEST_CASE("lexer reads punctuation tokens", "[lexer]") {
    Lexer lexer("{}[],:");

    REQUIRE(lexer.nextToken().type == TokenType::LBrace);
    REQUIRE(lexer.nextToken().type == TokenType::RBrace);
    REQUIRE(lexer.nextToken().type == TokenType::LBracket);
    REQUIRE(lexer.nextToken().type == TokenType::RBracket);
    REQUIRE(lexer.nextToken().type == TokenType::Comma);
    REQUIRE(lexer.nextToken().type == TokenType::Colon);
    REQUIRE(lexer.nextToken().type == TokenType::EoF);
}

TEST_CASE("lexer reads literals and numbers", "[lexer]") {
    Lexer lexer(R"(true false null -12.5)");

    REQUIRE(lexer.nextToken().type == TokenType::True);
    REQUIRE(lexer.nextToken().type == TokenType::False);
    REQUIRE(lexer.nextToken().type == TokenType::Null);

    Token number = lexer.nextToken();
    REQUIRE(number.type == TokenType::Number);
    REQUIRE(number.value == "-12.5");

    REQUIRE(lexer.nextToken().type == TokenType::EoF);
}

TEST_CASE("lexer reads escaped strings", "[lexer][string]") {
    Lexer lexer(R"("a\"b\\c\n")");

    Token token = lexer.nextToken();

    REQUIRE(token.type == TokenType::String);
    REQUIRE(token.value == "a\"b\\c\n");
}

TEST_CASE("lexer rejects unterminated strings", "[lexer][error]") {
    Lexer lexer(R"("unterminated)");

    REQUIRE_THROWS_AS(lexer.nextToken(), std::runtime_error);
}
