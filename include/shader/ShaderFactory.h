#ifndef SHADERFACTORY_H
#define SHADERFACTORY_H
#include <glad/glad.h>
#include <string>
#include <fstream>
#include <iostream>
#include "shader/Shader.h"
#include <vector>
#include <sstream>
#include "shader/DrawAttributes.h"
#include "glm.h"

enum class VarType { VertexAttributeInput, Input, Output, Uniform, Custom, Primitive };
typedef unsigned int ShaderType;
typedef unsigned int ShaderArgs;
extern ShaderType ShaderType_Vertex;
extern ShaderType ShaderType_Fragment;
extern ShaderType ShaderType_TCS;
extern ShaderType ShaderType_TES;
extern ShaderType ShaderType_NONE;

extern ShaderArgs ShaderArgs_Texture;
extern ShaderArgs ShaderArgs_Normal;
extern ShaderArgs ShaderArgs_Tesselate_5;
extern ShaderArgs ShaderArgs_Lighting;
extern ShaderArgs ShaderArgs_Color;

struct MutableVar {
    int attribIndex;
    VarType type; 
    std::string glType;
    std::string name;
    MutableVar(VarType type, std::string glType, std::string name, int attribIndex = -1)
        : type(type), glType(glType), name(name), attribIndex(attribIndex) {}
};

struct MutableStruct {
    std::vector<MutableVar> variables;
    std::string name;
    std::string body;

    MutableStruct(
        const std::string& name, 
        const std::vector<MutableVar>& variables = std::vector<MutableVar>(),
        const std::string& body = std::string())
        : name(name), variables(variables), body(body) {}

    void AddVariable(const VarType& type, const std::string& glType, const std::string& name) {
        MutableVar obj(type, glType, name);
        variables.push_back(obj);
    }

    void AddBody(const std::string& text) {
        std::stringstream ss(text);
        std::string val;
        while (std::getline(ss, val)) body += "\t" + val + "\n";
    }
};

struct MutableObject {
    std::vector<MutableVar> variables;
    std::vector<MutableStruct> structs;
    std::string prebody, body;

    void AddVariable(const VarType& type, const std::string& glType, const std::string& name, int attribIndex = -1) {
        MutableVar obj(type, glType, name, attribIndex);
        variables.push_back(obj);
    }

    void AddCustomVariable(const std::string& content) {
        MutableVar obj(VarType::Custom, "", content);
        variables.push_back(obj);
    }

    void AddBody(const std::string& text) {
        std::stringstream ss(text);
        std::string val;
        while (std::getline(ss, val)) body += "\t" + val + "\n";
    }

    void AddPreBody(const std::string& text) {
        std::stringstream ss(text);
        std::string val;
        while (std::getline(ss, val)) prebody += val + "\n";
    }

    void AddStruct(const MutableStruct& obj) {
        structs.push_back(obj);
    }

    void Clear() {
        variables.clear();
        structs.clear();
        prebody = "";
        body = "";
    }
};

struct MutableShader {
    MutableObject object;
    ShaderType type;
    ShaderArgs args;
};

struct ShaderMetadata {
    GLuint shader;           // the shader id from glCreateShader()
    MutableShader object;    // shader object data
};

typedef std::pair<unsigned int, std::pair<ShaderArgs, GLuint>> ShaderCacheTriplet;

class ShaderFactory
{
private:
    GLuint shaderProgram; // a single shader program handle
    std::vector<ShaderMetadata> shaderMetadata;

    std::string readFileContents(std::string fileName);
    bool writeFileContents(std::string fileName, std::string content);
    std::string shaderObjectToString(const MutableShader& shader);
    std::string getShaderTypeSuffix(const ShaderType& type);
    void populateShader(MutableShader& shader);
    GLuint getShaderMacro(const ShaderType& type);
    void checkError(GLuint obj, std::string errorMessage);
    bool tesselationEnabled = false;
    ShaderType currentShaderArgs = 0;
    GLuint generateShaderProgram();
    bool isShaderLoaded(GLuint program);
    int isShaderCached(const ShaderArgs& shaderArgs);
    ShaderType convertExtraShaderArgsToType(const ShaderArgs& extraShaderArgs);
    void writeMutableVariables(const ShaderType& shaderType, const std::vector<MutableVar> mutableVariables, std::string& out);

    // cached shaders
    std::vector<ShaderCacheTriplet> cachedShaders;
    unsigned int cacheShaderAndReturnIndex(GLuint shaderProgram, const ShaderArgs& shaderArgs);

public:
    const std::string SHADER_DIR = std::string(SRC_DIR) + "/shader/";
    const std::string OGL_VERSION = "#version 410 core";
    
    ShaderFactory();
    ~ShaderFactory();
    GLuint GetShaderProgram();

    unsigned int InitShader(const ShaderArgs& extraShaderArgs = 0);
    unsigned int InitShaderFromCache(ShaderArgs extraShaderArgs, unsigned int shaderLocation);
    void LoadShader(const ShaderArgs& extraShaderArgs = 0);
    void ReloadShader(const ShaderArgs& extraShaderArgs = 0);

    void Use(const DrawAttributes& da, const glm::vec3& cameraPos = glm::vec3(0,0,0));
    void SetShaderUniforms(const DrawAttributes& da, const glm::vec3& cameraPos = glm::vec3(0,0,0));
    bool IsTesselationEnabled();

    // shader uniform helpers
    void SetInt(const std::string& name, int value) const;
    void SetFloat(const std::string& name, float value) const;
    void SetVec3(const std::string& name, float x, float y, float z) const;
    void SetVec3(const std::string& name, glm::vec3 vec) const;
};
#endif
