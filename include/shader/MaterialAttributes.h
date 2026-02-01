#ifndef MATERIALATTRIBUTES_H
#define MATERIALATTRIBUTES_H
#include "glm.h"
#include "utility/Material.h"

struct MaterialAttributes
{
	MaterialAttributes()
	{
        shininess = 32.0f;
        diffuseMap = 0;
        diffuse = glm::vec3(0, 0, 0);
        specular = glm::vec3(0.5f, 0.5f, 0.5f);
	}

    void LoadFromMaterial(const Material& material)
    {
        shininess = material.Ns;
        diffuse = material.Kd;
        specular = material.Ks;
    }

    float shininess;
    int diffuseMap;
    glm::vec3 diffuse;
    glm::vec3 specular;
};
#endif