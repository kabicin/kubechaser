#ifndef JSON_H
#define JSON_H
#include "glm.h"
#include <ostream>
#include <string>

// Minimal helpers for writing JSON by hand. There is no JSON library in the
// project and the only need so far is emitting files, not parsing them.
namespace Json
{
// Writes s as a quoted JSON string, escaping quotes, backslashes and control
// characters.
void WriteString(std::ostream& os, const std::string& s);

// Writes a float with enough digits to round-trip. JSON has no
// representation for NaN or infinity, so those are written as null.
void WriteNumber(std::ostream& os, float value);

void WriteBool(std::ostream& os, bool value);

// Writes [x, y, z].
void WriteVec3(std::ostream& os, const glm::vec3& v);
}
#endif
