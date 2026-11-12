#include "shader/DrawAttributes.h"
#include "utility/Json.h"

#include <cmath>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

namespace {

int failures = 0;

void expect(bool ok, const std::string& what)
{
    if (!ok) {
        std::cerr << "FAIL: " << what << "\n";
        ++failures;
    }
}

std::string str(const std::string& s)
{
    std::ostringstream os;
    Json::WriteString(os, s);
    return os.str();
}

std::string num(float v)
{
    std::ostringstream os;
    Json::WriteNumber(os, v);
    return os.str();
}

bool contains(const std::string& haystack, const std::string& needle)
{
    return haystack.find(needle) != std::string::npos;
}

} // namespace

int main()
{
    expect(str("plain") == "\"plain\"", "plain string");
    expect(str("a\"b\\c") == "\"a\\\"b\\\\c\"", "quote and backslash escaped");
    expect(str("a\nb\tc") == "\"a\\nb\\tc\"", "newline and tab escaped");
    expect(str(std::string("\x01", 1)) == "\"\\u0001\"", "control char escaped");

    expect(num(0.0f) == "0", "zero");
    expect(num(-2.5f) == "-2.5", "negative");
    expect(std::stof(num(0.1f)) == 0.1f, "float round trips");
    expect(num(std::numeric_limits<float>::quiet_NaN()) == "null", "NaN is null");
    expect(num(std::numeric_limits<float>::infinity()) == "null", "inf is null");

    // The caller's stream formatting must not leak into or be changed by WriteNumber.
    {
        std::ostringstream os;
        os.precision(2);
        os << std::fixed;
        Json::WriteNumber(os, 1.0f / 3.0f);
        expect(std::stof(os.str()) == 1.0f / 3.0f, "ignores caller precision");
        expect(os.precision() == 2, "caller precision preserved");
    }

    {
        std::ostringstream os;
        Json::WriteVec3(os, glm::vec3(1.0f, -2.0f, 0.5f));
        expect(os.str() == "[1, -2, 0.5]", "vec3");
    }

    {
        DrawAttributes da;
        da.wireframe = true;
        da.lightSwitch = 5;
        da.solidColor = glm::vec3(0.25f, 0.5f, 1.0f);
        da.materialAttributes.shininess = 64.0f;
        std::ostringstream os;
        da.serializeJSON(os, "  ");
        const std::string json = os.str();
        expect(json.front() == '{' && json.back() == '}', "object braces");
        expect(contains(json, "\n    \"wireframe\": true,"), "wireframe with indent");
        expect(contains(json, "\"tesselate\": false,"), "tesselate");
        expect(contains(json, "\"solidColor\": [0.25, 0.5, 1],"), "solidColor");
        expect(contains(json, "\"shininess\": 64,"), "nested shininess");
        expect(contains(json, "\"lightSwitch\": 5,"), "lightSwitch");
        expect(contains(json, "\"instanced\": false\n  }"), "last field has no comma and closes at indent");
        expect(!contains(json, ",\n  }") && !contains(json, ",\n    }"), "no trailing commas");
    }

    if (failures == 0) std::cout << "json_test passed\n";
    return failures == 0 ? 0 : 1;
}
