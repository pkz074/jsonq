#include <iostream>
#include <string>
#include <fstream>
#include <exception>
#include <cstdlib>
#include "parser/Parser.hpp"
#include "printer/Printer.hpp"
#include "query/QueryEngine.hpp"
#include "schema/SchemaValidator.hpp"

#ifndef JSONQ_VERSION
#define JSONQ_VERSION "dev"
#endif

namespace {

std::string readFile(const std::string& path) {
    std::ifstream file(path);

    if (!file) {
        throw std::runtime_error("could not open file " + path);
    }

    return std::string((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
}

JsonValue parseFile(const std::string& path) {
    std::string contents = readFile(path);
    Lexer lexer(contents);
    Parser parser(lexer);
    return parser.parse();
}

void printUsage(const char* programName) {
    std::cout << "Usage:\n"
              << "  " << programName << " <json_file> [query]\n"
              << "  " << programName << " --schema <schema_file> <json_file>\n"
              << "  " << programName << " --help\n"
              << "  " << programName << " --version\n\n"
              << "Query examples:\n"
              << "  $.name\n"
              << "  $.hobbies[1]\n"
              << "  $.users[?age>=30].name\n";
}

void printVersion() {
    std::cout << "jsonq " << JSONQ_VERSION << std::endl;
}

}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printUsage(argv[0]);
        return EXIT_FAILURE;
    }

    try {
        std::string command = argv[1];

        if (command == "--help" || command == "-h") {
            printUsage(argv[0]);
            return EXIT_SUCCESS;
        }

        if (command == "--version") {
            printVersion();
            return EXIT_SUCCESS;
        }

        if (command == "--schema") {
            if (argc != 4) {
                printUsage(argv[0]);
                return EXIT_FAILURE;
            }

            JsonValue schema = parseFile(argv[2]);
            JsonValue document = parseFile(argv[3]);
            SchemaValidator validator;
            SchemaResult result = validator.validate(document, schema);

            if (result.valid) {
                std::cout << "valid" << std::endl;
                return 0;
            }

            for (const std::string& error : result.errors) {
                std::cerr << "error: " << error << std::endl;
            }
            return 1;
        }

        if (!command.empty() && command[0] == '-') {
            throw std::runtime_error("unknown option: " + command);
        }

        if (argc > 3) {
            printUsage(argv[0]);
            return EXIT_FAILURE;
        }

        JsonValue result = parseFile(argv[1]);

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
