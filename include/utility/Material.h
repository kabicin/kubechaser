#ifndef MATERIAL_H
#define MATERIAL_H
#include "glm.h"
#include <string>
#include <iostream>

class Material
{
public:

    Material() {};
	Material(
        const std::string& name,
        const glm::vec3& Ka,            // ambient color
        const glm::vec3& Kd,            // diffuse color
        const glm::vec3& Ks,            // specular color
        float Ns                       // specular highlights (0-1000)
    ) : 
        name(name), 
        Ka(Ka), 
        Kd(Kd), 
        Ks(Ks), 
        Ns(Ns) 
    {};
    std::string name;
    glm::vec3 Ka = glm::vec3(0.0f);
    glm::vec3 Kd = glm::vec3(0.8f);
    glm::vec3 Ks = glm::vec3(0.0f);
    float Ns = 0.0f;
    std::string map_Kd;             // diffuse texture, relative to the MTL file
    int faceCount = 0;              // faces in the OBJ that select this material via usemtl

    friend std::ostream& operator<<(std::ostream& o, const Material& m);
};
#endif