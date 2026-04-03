#include "parser/Parser.hpp"
#include "printer/Printer.hpp"
#include <iostream>

int main() {
    std::string input = R"({
        "name": "Alice",
        "age": 30,
        "hobbies": ["reading", "coding"]
    })";

    Lexer lexer(input);
    Parser parser(lexer);
    JsonValue result = parser.parse();

    Printer printer;
    std::cout << printer.print(result) << std::endl;
}
