#include "shader/DrawAttributes.h"
#include "utility/Json.h"
#include <istream>
#include <ostream>
#include <sstream>
#include <string>

struct DrawAttributes da;

namespace {

void writeVec3(std::ostream& os, const char* key, const glm::vec3& v)
{
    os << key << ' ' << v.x << ' ' << v.y << ' ' << v.z << '\n';
}

// Only assign on success so a malformed value keeps the default.
template <typename T>
bool readValue(std::istream& is, T& out)
{
    T value;
    if (!(is >> value)) return false;
    out = value;
    return true;
}

bool readBool(std::istream& is, bool& out)
{
    int value;
    if (!(is >> value)) return false;
    out = value != 0;
    return true;
}

bool readVec3(std::istream& is, glm::vec3& out)
{
    glm::vec3 v;
    if (!(is >> v.x >> v.y >> v.z)) return false;
    out = v;
    return true;
}

} // namespace

std::ostream& operator<<(std::ostream& os, const DrawAttributes& da)
{
    const MaterialAttributes& ma = da.materialAttributes;
    os << "DrawAttributes{\n" <<
        "\twireframe: " << da.wireframe << "\n" <<
        "\ttesselate: " << da.tesselate << "\n" <<
        "\tnormal: " << da.normal << "\n" <<
        "\ttexture: " << da.texture << "\n" <<
        "\tpassthrough: " << da.passthrough << "\n" <<
        "\tcolor: " << da.color << "\n" <<
        "\tuseVertexColor: " << da.useVertexColor << "\n" <<
        "\tsolidColor: [" << da.solidColor.x << "," << da.solidColor.y << "," << da.solidColor.z << "]\n" <<
        "\tmaterial.shininess: " << ma.shininess << "\n" <<
        "\tmaterial.diffuseMap: " << ma.diffuseMap << "\n" <<
        "\tmaterial.diffuse: [" << ma.diffuse.x << "," << ma.diffuse.y << "," << ma.diffuse.z << "]\n" <<
        "\tmaterial.specular: [" << ma.specular.x << "," << ma.specular.y << "," << ma.specular.z << "]\n" <<
        "\tlightBlinn: " << da.lightBlinn << "\n" <<
        "\tlightSwitch: " << da.lightSwitch << "\n" <<
        "\tinstanced: " << da.instanced << "\n}";
    return os;
}

// One "key value..." pair per line. Unknown keys are ignored by deserialize,
// so callers may append their own lines to the same stream.
void DrawAttributes::serialize(std::ostream& os) const
{
    os << "wireframe " << wireframe << '\n'
       << "tesselate " << tesselate << '\n'
       << "normal " << normal << '\n'
       << "texture " << texture << '\n'
       << "passthrough " << passthrough << '\n'
       << "color " << color << '\n'
       << "useVertexColor " << useVertexColor << '\n';
    writeVec3(os, "solidColor", solidColor);
    os << "material.shininess " << materialAttributes.shininess << '\n'
       << "material.diffuseMap " << materialAttributes.diffuseMap << '\n';
    writeVec3(os, "material.diffuse", materialAttributes.diffuse);
    writeVec3(os, "material.specular", materialAttributes.specular);
    os << "lightSwitch " << lightSwitch << '\n'
       << "lightBlinn " << lightBlinn << '\n'
       << "instanced " << instanced << '\n';
}

// Fields missing from the stream keep their default values.
DrawAttributes DrawAttributes::deserialize(std::istream& is)
{
    DrawAttributes da;
    std::string line;
    while (std::getline(is, line)) {
        std::istringstream ls(line);
        std::string key;
        if (!(ls >> key)) continue;

        if (key == "wireframe") readBool(ls, da.wireframe);
        else if (key == "tesselate") readBool(ls, da.tesselate);
        else if (key == "normal") readBool(ls, da.normal);
        else if (key == "texture") readBool(ls, da.texture);
        else if (key == "passthrough") readBool(ls, da.passthrough);
        else if (key == "color") readBool(ls, da.color);
        else if (key == "useVertexColor") readBool(ls, da.useVertexColor);
        else if (key == "solidColor") readVec3(ls, da.solidColor);
        else if (key == "material.shininess") readValue(ls, da.materialAttributes.shininess);
        else if (key == "material.diffuseMap") readValue(ls, da.materialAttributes.diffuseMap);
        else if (key == "material.diffuse") readVec3(ls, da.materialAttributes.diffuse);
        else if (key == "material.specular") readVec3(ls, da.materialAttributes.specular);
        else if (key == "lightSwitch") readValue(ls, da.lightSwitch);
        else if (key == "lightBlinn") readBool(ls, da.lightBlinn);
        else if (key == "instanced") readBool(ls, da.instanced);
    }
    return da;
}

void DrawAttributes::serializeJSON(std::ostream& os, const std::string& indent) const
{
    const std::string in1 = indent + "  ";
    const std::string in2 = in1 + "  ";
    auto key = [&os](const std::string& in, const char* name) {
        os << in;
        Json::WriteString(os, name);
        os << ": ";
    };

    os << "{\n";
    key(in1, "wireframe"); Json::WriteBool(os, wireframe); os << ",\n";
    key(in1, "tesselate"); Json::WriteBool(os, tesselate); os << ",\n";
    key(in1, "normal"); Json::WriteBool(os, normal); os << ",\n";
    key(in1, "texture"); Json::WriteBool(os, texture); os << ",\n";
    key(in1, "passthrough"); Json::WriteBool(os, passthrough); os << ",\n";
    key(in1, "color"); Json::WriteBool(os, color); os << ",\n";
    key(in1, "useVertexColor"); Json::WriteBool(os, useVertexColor); os << ",\n";
    key(in1, "solidColor"); Json::WriteVec3(os, solidColor); os << ",\n";
    key(in1, "material"); os << "{\n";
    key(in2, "shininess"); Json::WriteNumber(os, materialAttributes.shininess); os << ",\n";
    key(in2, "diffuseMap"); os << materialAttributes.diffuseMap << ",\n";
    key(in2, "diffuse"); Json::WriteVec3(os, materialAttributes.diffuse); os << ",\n";
    key(in2, "specular"); Json::WriteVec3(os, materialAttributes.specular); os << "\n";
    os << in1 << "},\n";
    key(in1, "lightSwitch"); os << lightSwitch << ",\n";
    key(in1, "lightBlinn"); Json::WriteBool(os, lightBlinn); os << ",\n";
    key(in1, "instanced"); Json::WriteBool(os, instanced); os << "\n";
    os << indent << "}";
}
