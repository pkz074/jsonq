cmake_minimum_required(VERSION 3.20)
project(jsonq VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_executable(jsonq src/main.cpp src/lexer/lexer.cpp src/parser/Parser.cpp src/printer/Printer.cpp src/query/QueryEngine.cpp src/schema/SchemaValidator.cpp)
target_include_directories(jsonq PRIVATE src)
target_compile_definitions(jsonq PRIVATE JSONQ_VERSION="${PROJECT_VERSION}")
install(TARGETS jsonq RUNTIME DESTINATION bin)



include(FetchContent)
enable_testing()

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(Catch2)


add_executable(test_lexer tests/test_lexer.cpp src/lexer/lexer.cpp src/parser/Parser.cpp)
target_include_directories(test_lexer PRIVATE src)
target_link_libraries(test_lexer PRIVATE Catch2::Catch2WithMain)
add_test(NAME lexer COMMAND test_lexer)

add_executable(test_parser tests/test_parser.cpp src/lexer/lexer.cpp src/parser/Parser.cpp)
target_include_directories(test_parser PRIVATE src)
target_link_libraries(test_parser PRIVATE Catch2::Catch2WithMain)
add_test(NAME parser COMMAND test_parser)

add_executable(test_query tests/test_query.cpp src/query/QueryEngine.cpp src/lexer/lexer.cpp src/parser/Parser.cpp)
target_include_directories(test_query PRIVATE src)
target_link_libraries(test_query PRIVATE Catch2::Catch2WithMain)
add_test(NAME query COMMAND test_query)

add_executable(test_printer tests/test_printer.cpp src/printer/Printer.cpp)
target_include_directories(test_printer PRIVATE src)
target_link_libraries(test_printer PRIVATE Catch2::Catch2WithMain)
add_test(NAME printer COMMAND test_printer)

add_executable(test_schema tests/test_schema.cpp src/schema/SchemaValidator.cpp)
target_include_directories(test_schema PRIVATE src)
target_link_libraries(test_schema PRIVATE Catch2::Catch2WithMain)
add_test(NAME schema COMMAND test_schema)

add_executable(bench_jsonq benchmarks/bench_jsonq.cpp src/lexer/lexer.cpp src/parser/Parser.cpp src/printer/Printer.cpp src/query/QueryEngine.cpp src/schema/SchemaValidator.cpp)
target_include_directories(bench_jsonq PRIVATE src)
