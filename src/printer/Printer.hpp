#pragma once
#include <string>
#include "parser/JsonValue.hpp"
template<class... Ts>
struct overloads : Ts... { using Ts::operator()...; };

template<class... Ts>
overloads(Ts...) -> overloads<Ts...>;

class Printer {
    public:
    std::string print(const JsonValue& val);


    private:
    std::string printValue(const JsonValue& val, int indent);
    std::string identStr(int ident);

};
