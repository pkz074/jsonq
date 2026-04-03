#include "Printer.hpp"
#include <vector>
#include <sstream>
#include <iomanip>


std::string Printer::identStr(int ident) {

    return std::string(ident * 2, ' ');
}

std::string Printer::print(const JsonValue& val) {
    return printValue(val, 0);
}

std::string Printer::printValue(const JsonValue& val, int indent) {

    return std::visit(overloads{
        [](const std::string& v){return "\"" + v + "\"";},
        [](double v){
                    std::ostringstream oss;
                    oss << std::fixed << std::setprecision(6) << v; // adjust precision if needed
                    return oss.str();
                },
        [](bool v){return v ? std::string("true") : std::string("false");},
        [](std::nullptr_t){return std::string("null");},
        [&](const JsonArray& v){
            std::string result = "[\n";
            for(int i = 0; i<v.size(); i++){
                result += identStr(indent + 1);
                result += printValue(v[i], indent + 1);
                if (i < v.size() - 1)
                    result += ",";
                result += "\n";
            }
            result += identStr(indent) + "]";
            return result;
        },
        [&](const JsonObject& v){
            std::string result = "{\n";
            for (int i = 0; i <v.size(); i++){
                result += identStr(indent + 1);
                result += "\"" + v[i].first + "\"" + ": " + printValue(v[i].second, indent+1);
                if (i < v.size() - 1)
                    result += ",";
                result += "\n";
            }
            result += identStr(indent) + "}";
            return result;
        }
    }, val.data);

}
