#ifndef SHADER_H
#define SHADER_H
#include <glad/glad.h>
#include <string>
#include <fstream>
#include <iostream>

class Shader
{
private:
    void initialize(const GLuint& shaderProgram, const std::string& vertexPath, const std::string& fragmentPath);
    void addTesselationShader(const GLuint& shaderProgram, const std::string& tcsName, const std::string& tesName);
    std::string getFileContents(std::string fileName);
    void checkError(GLuint obj, std::string errorMessage);

public:
    const std::string SHADER_DIR = std::string(SRC_DIR) + "/shader/";
    GLuint shaderProgram;
    Shader(const std::string& vertexPath, const std::string& fragmentPath, const std::string& tcsShader = std::string(), const std::string& tesShader = std::string());
    ~Shader();
    void Use();
    void SetInt(const std::string& name, int value) const;
    void SetFloat(const std::string& name, float value) const;
    void SetVec3(const std::string& name, float x, float y, float z) const;
};
#endif
