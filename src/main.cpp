#include <iostream>
#include <string>
#include <fstream>
#include <exception>
#include "parser/Parser.hpp"
#include "printer/Printer.hpp"
#include "query/QueryEngine.hpp"

int main(int argc, char *argv[]) {

    if (argc < 2) {
            std::cout << "Usage: " << argv[0] << " <json_file> [query]" << std::endl;
            return EXIT_FAILURE;
        }

    std::ifstream file(argv[1]);

    if (!file){
        std::cerr << "error: could not open file" << argv[1] << std::endl;
        return 1;
    }

    std::string contents((std::istreambuf_iterator<char>(file)),
                          std::istreambuf_iterator<char>());

    try {
        Lexer lexer(contents);
        Parser parser(lexer);
        JsonValue result = parser.parse();

        if (argc >= 3) {
            QueryEngine queryEngine;
            PathExpr path = queryEngine.parse(argv[2]);
            result = queryEngine.evaluate(result, path);
        }

        Printer printer;

        std::cout << printer.print(result) << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
