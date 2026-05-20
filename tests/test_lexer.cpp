#include <catch2/catch_test_macros.hpp>
#include <string>
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

TEST_CASE("lexer tracks token locations", "[lexer][location]") {
    Lexer lexer("{\n  \"name\": true\n}");

    Token leftBrace = lexer.nextToken();
    REQUIRE(leftBrace.line == 1);
    REQUIRE(leftBrace.column == 1);

    Token name = lexer.nextToken();
    REQUIRE(name.type == TokenType::String);
    REQUIRE(name.line == 2);
    REQUIRE(name.column == 3);

    Token colon = lexer.nextToken();
    REQUIRE(colon.type == TokenType::Colon);
    REQUIRE(colon.line == 2);
    REQUIRE(colon.column == 9);

    Token boolean = lexer.nextToken();
    REQUIRE(boolean.type == TokenType::True);
    REQUIRE(boolean.line == 2);
    REQUIRE(boolean.column == 11);
}

TEST_CASE("lexer reads literals and numbers", "[lexer]") {
    Lexer lexer(R"(true false null -12.5 0 3.14 6e2 -1.5E-3)");

    REQUIRE(lexer.nextToken().type == TokenType::True);
    REQUIRE(lexer.nextToken().type == TokenType::False);
    REQUIRE(lexer.nextToken().type == TokenType::Null);

    Token negativeDecimal = lexer.nextToken();
    REQUIRE(negativeDecimal.type == TokenType::Number);
    REQUIRE(negativeDecimal.value == "-12.5");

    Token zero = lexer.nextToken();
    REQUIRE(zero.type == TokenType::Number);
    REQUIRE(zero.value == "0");

    Token decimal = lexer.nextToken();
    REQUIRE(decimal.type == TokenType::Number);
    REQUIRE(decimal.value == "3.14");

    Token exponent = lexer.nextToken();
    REQUIRE(exponent.type == TokenType::Number);
    REQUIRE(exponent.value == "6e2");

    Token negativeExponent = lexer.nextToken();
    REQUIRE(negativeExponent.type == TokenType::Number);
    REQUIRE(negativeExponent.value == "-1.5E-3");

    REQUIRE(lexer.nextToken().type == TokenType::EoF);
}

TEST_CASE("lexer rejects invalid numbers", "[lexer][error]") {
    REQUIRE_THROWS_AS(Lexer("-").nextToken(), std::runtime_error);
    REQUIRE_THROWS_AS(Lexer("01").nextToken(), std::runtime_error);
    REQUIRE_THROWS_AS(Lexer("1.").nextToken(), std::runtime_error);
    REQUIRE_THROWS_AS(Lexer("1..2").nextToken(), std::runtime_error);
    REQUIRE_THROWS_AS(Lexer("1e").nextToken(), std::runtime_error);
    REQUIRE_THROWS_AS(Lexer("1e+").nextToken(), std::runtime_error);
}

TEST_CASE("lexer errors include location", "[lexer][error]") {
    Lexer lexer("{\n  \"bad\": 01\n}");

    try {
        while (true) {
            Token token = lexer.nextToken();
            if (token.type == TokenType::EoF) {
                break;
            }
        }
        FAIL("expected lexer error");
    } catch (const std::runtime_error& e) {
        std::string message = e.what();
        REQUIRE(message.find("line 2, column 10") != std::string::npos);
        REQUIRE(message.find("invalid number") != std::string::npos);
    }
}

TEST_CASE("lexer reads escaped strings", "[lexer][string]") {
    Lexer lexer(R"("a\"b\\c\n")");

    Token token = lexer.nextToken();

    REQUIRE(token.type == TokenType::String);
    REQUIRE(token.value == "a\"b\\c\n");
}

TEST_CASE("lexer reads unicode escapes as utf-8", "[lexer][string]") {
    Lexer ascii(R"("\u0041")");
    REQUIRE(ascii.nextToken().value == "A");

    Lexer latin1(R"("\u00E9")");
    REQUIRE(latin1.nextToken().value == "\xC3\xA9");

    Lexer surrogatePair(R"("\uD83D\uDE00")");
    REQUIRE(surrogatePair.nextToken().value == "\xF0\x9F\x98\x80");
}

TEST_CASE("lexer rejects invalid unicode escapes", "[lexer][error]") {
    REQUIRE_THROWS_AS(Lexer(R"("\u12")").nextToken(), std::runtime_error);
    REQUIRE_THROWS_AS(Lexer(R"("\u12XZ")").nextToken(), std::runtime_error);
    REQUIRE_THROWS_AS(Lexer(R"("\uD83D")").nextToken(), std::runtime_error);
    REQUIRE_THROWS_AS(Lexer(R"("\uDE00")").nextToken(), std::runtime_error);
    REQUIRE_THROWS_AS(Lexer(R"("\uD83D\u0041")").nextToken(), std::runtime_error);
}

TEST_CASE("lexer rejects unterminated strings", "[lexer][error]") {
    Lexer lexer(R"("unterminated)");

    REQUIRE_THROWS_AS(lexer.nextToken(), std::runtime_error);
}
