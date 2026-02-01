#include "shader/DrawAttributes.h"
#include <ostream>

struct DrawAttributes da;

constexpr std::streamsize MAX_STREAMSIZE = std::numeric_limits<std::streamsize>::max();

std::ostream& operator<<(std::ostream& os, const DrawAttributes& da)
{
    os << "DrawAttributes{\n" << 
        "\twireframe: " << da.wireframe << "\n" <<
        "\ttesselate: " << da.tesselate << "\n" <<
        "\tnormal: " << da.normal << "\n" <<
        "\tpassthrough: " << da.passthrough << "\n" <<
        "\tcolor: " << da.color << "\n" <<
        "\tuseVertexColor: " << da.useVertexColor << "\n" <<
        "\tsolidColor: [" << da.solidColor.x << "," << da.solidColor.y << "," << da.solidColor.z << "]\n" <<
        "\tlightBlinn: " << da.lightBlinn << "\n" <<
        "\tlightSwitch: " << da.lightSwitch << "\n}";
    return os;
}

void DrawAttributes::serialize(std::ostream& os) const
{
   os << this->wireframe << "\n" << this->tesselate << "\n" << this->normal << "\n" << this->passthrough << "\n"
    << this->color << "\n" << this->useVertexColor << "\n" << this->solidColor.x << "\n" << this->solidColor.y << "\n" 
    << this->solidColor.z << "\n" << this->lightBlinn << "\n" << this->lightSwitch << "\n";
}

DrawAttributes DrawAttributes::deserialize(std::istream& is)
{
    DrawAttributes drawAttribs;
    int intVal;
    
    is >> intVal;
    drawAttribs.wireframe = intVal != 0;
    is.ignore(MAX_STREAMSIZE, '\n');
    
    is >> intVal;
    drawAttribs.tesselate = intVal != 0;
    is.ignore(MAX_STREAMSIZE, '\n');

    is >> intVal;
    drawAttribs.normal = intVal != 0;
    is.ignore(MAX_STREAMSIZE, '\n');

    is >> intVal;
    drawAttribs.passthrough = intVal != 0;
    is.ignore(MAX_STREAMSIZE, '\n');

    is >> intVal;
    drawAttribs.color = intVal != 0;
    is.ignore(MAX_STREAMSIZE, '\n');

    is >> intVal;
    drawAttribs.useVertexColor = intVal != 0;
    is.ignore(MAX_STREAMSIZE, '\n');

    float fVal;
    is >> fVal;
    drawAttribs.solidColor.x = fVal;
    is.ignore(MAX_STREAMSIZE, '\n');

    is >> fVal;
    drawAttribs.solidColor.y = fVal;
    is.ignore(MAX_STREAMSIZE, '\n');

    is >> fVal;
    drawAttribs.solidColor.z = fVal;
    is.ignore(MAX_STREAMSIZE, '\n');

    is >> intVal;
    drawAttribs.lightBlinn = intVal != 0;
    is.ignore(MAX_STREAMSIZE, '\n');

    unsigned int uIntVal;
    is >> uIntVal;
    drawAttribs.lightSwitch = uIntVal;
    is.ignore(MAX_STREAMSIZE, '\n');

    return drawAttribs;
}