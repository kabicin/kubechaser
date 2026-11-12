#ifndef UNIFORMLOCATIONS_H
#define UNIFORMLOCATIONS_H
#include <glad/glad.h>
#include <string>

// Uniform locations for one linked shader program. Resolved once right after
// the program links so per-draw code never calls glGetUniformLocation.
// A location of -1 means the uniform is absent (or optimized out) in this
// program; glUniform* silently ignores -1, matching the old by-name behaviour.
struct UniformLocations
{
    static constexpr int POINT_LIGHT_COUNT = 4;

    struct Light {
        GLint direction = -1;
        GLint position = -1;
        GLint ambient = -1;
        GLint diffuse = -1;
        GLint specular = -1;
        GLint constant = -1;
        GLint linear = -1;
        GLint quadratic = -1;
        GLint cutOff = -1;
        GLint outerCutOff = -1;

        void Load(GLuint program, const std::string& prefix) {
            direction = glGetUniformLocation(program, (prefix + ".direction").c_str());
            position = glGetUniformLocation(program, (prefix + ".position").c_str());
            ambient = glGetUniformLocation(program, (prefix + ".ambient").c_str());
            diffuse = glGetUniformLocation(program, (prefix + ".diffuse").c_str());
            specular = glGetUniformLocation(program, (prefix + ".specular").c_str());
            constant = glGetUniformLocation(program, (prefix + ".constant").c_str());
            linear = glGetUniformLocation(program, (prefix + ".linear").c_str());
            quadratic = glGetUniformLocation(program, (prefix + ".quadratic").c_str());
            cutOff = glGetUniformLocation(program, (prefix + ".cutOff").c_str());
            outerCutOff = glGetUniformLocation(program, (prefix + ".outerCutOff").c_str());
        }
    };

    // camera transforms
    GLint model = -1;
    GLint view = -1;
    GLint projection = -1;

    // lighting
    GLint blinn = -1;
    GLint usingTexture = -1;
    GLint lightSwitch = -1;
    GLint viewPos = -1;
    GLint lightColor = -1;
    Light dirLight;
    Light pointLights[POINT_LIGHT_COUNT];
    Light spotlight;

    // material
    GLint materialShininess = -1;
    GLint materialDiffuseMap = -1;
    GLint materialDiffuse = -1;
    GLint materialSpecular = -1;

    // solid color
    GLint solidColor = -1;
    GLint useVertexColor = -1;

    void Load(GLuint program) {
        model = glGetUniformLocation(program, "model");
        view = glGetUniformLocation(program, "view");
        projection = glGetUniformLocation(program, "projection");

        blinn = glGetUniformLocation(program, "blinn");
        usingTexture = glGetUniformLocation(program, "usingTexture");
        lightSwitch = glGetUniformLocation(program, "lightSwitch");
        viewPos = glGetUniformLocation(program, "viewPos");
        lightColor = glGetUniformLocation(program, "lightColor");
        dirLight.Load(program, "dirLight");
        for (int i = 0; i < POINT_LIGHT_COUNT; i++) {
            pointLights[i].Load(program, "pointLights[" + std::to_string(i) + "]");
        }
        spotlight.Load(program, "spotlight");

        materialShininess = glGetUniformLocation(program, "material.shininess");
        materialDiffuseMap = glGetUniformLocation(program, "material.diffuseMap");
        materialDiffuse = glGetUniformLocation(program, "material.diffuse");
        materialSpecular = glGetUniformLocation(program, "material.specular");

        solidColor = glGetUniformLocation(program, "solidColor");
        useVertexColor = glGetUniformLocation(program, "useVertexColor");
    }
};
#endif
