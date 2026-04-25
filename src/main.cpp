#include <iostream>
#include <string>
#include <fstream>
#include "parser/Parser.hpp"
#include "printer/Printer.hpp"

int main(int argc, char *argv[]) {

    if (argc < 2) {
            std::cout << "Usage: " << argv[0] << " <test_string>" << std::endl;
            return EXIT_FAILURE;
        }

    std::ifstream file(argv[1]);

    if (!file){
        std::cerr << "error: could not open file" << argv[1] << std::endl;
        return 1;
    }

    std::string contents((std::istreambuf_iterator<char>(file)),
                          std::istreambuf_iterator<char>());

    Lexer lexer(contents);
    Parser parser(lexer);
    JsonValue result = parser.parse();
    Printer printer;

    std::cout << printer.print(result) << std::endl;

    return 0;
}
