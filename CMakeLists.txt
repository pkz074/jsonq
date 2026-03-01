cmake_minimum_required(VERSION 3.20)
project(jsonq)
set(CMAKE_CXX_STANDARD 17)


add_executable(jsonq src/main.cpp src/lexer/lexer.cpp)
target_include_directories(jsonq PRIVATE src)


include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(Catch2)


add_executable(test_lexer tests/test_lexer.cpp src/lexer/lexer.cpp)
target_include_directories(test_lexer PRIVATE src)
target_link_libraries(test_lexer PRIVATE Catch2::Catch2WithMain)
