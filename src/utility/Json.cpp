#include "utility/Json.h"
#include <cmath>
#include <cstdio>
#include <limits>
#include <locale>
#include <sstream>

namespace Json
{

void WriteString(std::ostream& os, const std::string& s)
{
    os << '"';
    for (unsigned char c : s) {
        switch (c) {
        case '"': os << "\\\""; break;
        case '\\': os << "\\\\"; break;
        case '\b': os << "\\b"; break;
        case '\f': os << "\\f"; break;
        case '\n': os << "\\n"; break;
        case '\r': os << "\\r"; break;
        case '\t': os << "\\t"; break;
        default:
            if (c < 0x20) {
                char buf[7];
                std::snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned>(c));
                os << buf;
            } else {
                os << static_cast<char>(c);
            }
        }
    }
    os << '"';
}

void WriteNumber(std::ostream& os, float value)
{
    if (!std::isfinite(value)) {
        os << "null";
        return;
    }
    // Format in a private stream so the caller's flags and locale are not
    // disturbed (and a comma decimal separator can never leak in).
    std::ostringstream ss;
    ss.imbue(std::locale::classic());
    ss.precision(std::numeric_limits<float>::max_digits10);
    ss << value;
    os << ss.str();
}

void WriteBool(std::ostream& os, bool value)
{
    os << (value ? "true" : "false");
}

void WriteVec3(std::ostream& os, const glm::vec3& v)
{
    os << '[';
    WriteNumber(os, v.x);
    os << ", ";
    WriteNumber(os, v.y);
    os << ", ";
    WriteNumber(os, v.z);
    os << ']';
}

} // namespace Json
