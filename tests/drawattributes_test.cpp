#include "shader/DrawAttributes.h"

#include <cmath>
#include <iostream>
#include <sstream>

namespace {

bool near(const glm::vec3& a, const glm::vec3& b)
{
    return std::abs(a.x - b.x) < 1e-5f && std::abs(a.y - b.y) < 1e-5f && std::abs(a.z - b.z) < 1e-5f;
}

int fail(const char* what)
{
    std::cerr << "DrawAttributes round trip lost: " << what << "\n";
    return 1;
}

} // namespace

int main()
{
    DrawAttributes original;
    original.wireframe = true;
    original.tesselate = true;
    original.normal = true;
    original.texture = true;
    original.passthrough = true;
    original.color = true;
    original.useVertexColor = false;
    original.solidColor = glm::vec3(0.1f, 0.2f, 0.3f);
    original.materialAttributes.shininess = 64.5f;
    original.materialAttributes.diffuseMap = 3;
    original.materialAttributes.diffuse = glm::vec3(0.4f, 0.5f, 0.6f);
    original.materialAttributes.specular = glm::vec3(0.7f, 0.8f, 0.9f);
    original.lightSwitch = 5;
    original.lightBlinn = true;
    original.instanced = true;

    std::stringstream stream;
    original.serialize(stream);
    stream << "viewer.lighting 1\n"; // unknown keys must be ignored
    const DrawAttributes loaded = DrawAttributes::deserialize(stream);

    if (loaded.wireframe != original.wireframe) return fail("wireframe");
    if (loaded.tesselate != original.tesselate) return fail("tesselate");
    if (loaded.normal != original.normal) return fail("normal");
    if (loaded.texture != original.texture) return fail("texture");
    if (loaded.passthrough != original.passthrough) return fail("passthrough");
    if (loaded.color != original.color) return fail("color");
    if (loaded.useVertexColor != original.useVertexColor) return fail("useVertexColor");
    if (!near(loaded.solidColor, original.solidColor)) return fail("solidColor");
    if (std::abs(loaded.materialAttributes.shininess - 64.5f) > 1e-5f) return fail("material.shininess");
    if (loaded.materialAttributes.diffuseMap != 3) return fail("material.diffuseMap");
    if (!near(loaded.materialAttributes.diffuse, original.materialAttributes.diffuse)) return fail("material.diffuse");
    if (!near(loaded.materialAttributes.specular, original.materialAttributes.specular)) return fail("material.specular");
    if (loaded.lightSwitch != original.lightSwitch) return fail("lightSwitch");
    if (loaded.lightBlinn != original.lightBlinn) return fail("lightBlinn");
    if (loaded.instanced != original.instanced) return fail("instanced");

    // Missing keys keep defaults.
    std::istringstream partial("wireframe 1\n");
    const DrawAttributes partialLoaded = DrawAttributes::deserialize(partial);
    const DrawAttributes defaults;
    if (!partialLoaded.wireframe || partialLoaded.lightSwitch != defaults.lightSwitch
        || partialLoaded.useVertexColor != defaults.useVertexColor) {
        std::cerr << "DrawAttributes partial load did not keep defaults.\n";
        return 1;
    }

    std::cout << "DrawAttributes tests passed.\n";
    return 0;
}
