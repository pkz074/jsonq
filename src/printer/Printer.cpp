#include "Printer.hpp"
#include <vector>


std::string Printer::identStr(int ident) {

    return std::string(ident * 2, ' ');
}

std::string Printer::print(const JsonValue& val) {
    return printValue(val, 0);
}

std::string Printer::printValue(const JsonValue& val, int ident) {

    return std::visit(overloads{
        [](const std::string& v){return "\"" + v + "\"";},
        [](double v){return std::to_string(v);},
        [](bool v){return v ? std::string("true") : std::string("false");},
        [](std::nullptr_t){return std::string("null");},
        [](const JsonArray& v){return std::string("[]");},
        [](const JsonObject& v){return std::string("{}");},
    }, val.data);

}
