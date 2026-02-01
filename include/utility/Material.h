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
        const int& Ns                  // specular highlights (0-1000)
    ) : 
        name(name), 
        Ka(Ka), 
        Kd(Kd), 
        Ks(Ks), 
        Ns(Ns) 
    {};
    std::string name;
    glm::vec3 Ka;
    glm::vec3 Kd;
    glm::vec3 Ks;
    int Ns;

    friend std::ostream& operator<<(std::ostream& o, const Material& m);
};
#endif