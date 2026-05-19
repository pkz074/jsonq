#include "printer/Printer.hpp"
#include <limits>
#include <sstream>
#include <iomanip>

namespace {

std::string escapeString(const std::string& value) {
    std::string escaped;

    for (char c : value) {
        switch (c) {
            case '"':
                escaped += "\\\"";
                break;
            case '\\':
                escaped += "\\\\";
                break;
            case '\n':
                escaped += "\\n";
                break;
            case '\r':
                escaped += "\\r";
                break;
            case '\t':
                escaped += "\\t";
                break;
            default:
                escaped += c;
                break;
        }
    }

    return escaped;
}

std::string formatNumber(double value) {
    std::ostringstream oss;
    oss << std::setprecision(std::numeric_limits<double>::digits10) << value;
    return oss.str();
}

}

std::string Printer::identStr(int ident) {

    return std::string(ident * 2, ' ');
}

std::string Printer::print(const JsonValue& val) {
    return printValue(val, 0);
}

std::string Printer::printValue(const JsonValue& val, int indent) {

    return std::visit(overloads{
        [](const std::string& v){return "\"" + escapeString(v) + "\"";},
        [](double v){return formatNumber(v);},
        [](bool v){return v ? std::string("true") : std::string("false");},
        [](std::nullptr_t){return std::string("null");},
        [&](const JsonArray& v){
            if (v.empty()) {
                return std::string("[]");
            }

            std::string result = "[\n";
            for(size_t i = 0; i < v.size(); i++){
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
            if (v.empty()) {
                return std::string("{}");
            }

            std::string result = "{\n";
            for (size_t i = 0; i < v.size(); i++){
                result += identStr(indent + 1);
                result += "\"" + escapeString(v[i].first) + "\"" + ": " + printValue(v[i].second, indent + 1);
                if (i < v.size() - 1)
                    result += ",";
                result += "\n";
            }
            result += identStr(indent) + "}";
            return result;
        }
    }, val.data);

}
