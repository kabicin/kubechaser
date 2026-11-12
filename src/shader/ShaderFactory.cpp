#include "shader/ShaderFactory.h"
#include <cstdlib>
#include <filesystem>
#include <iostream>

#include "glm.h"

ShaderType ShaderType_Vertex = 1;
ShaderType ShaderType_Fragment = 2;
ShaderType ShaderType_TCS = 4;
ShaderType ShaderType_TES = 8;
ShaderType ShaderType_NONE = 16;

ShaderArgs ShaderArgs_Texture = 1;
ShaderArgs ShaderArgs_Normal = 2;
ShaderArgs ShaderArgs_Tesselate_5 = 4;
ShaderArgs ShaderArgs_Lighting = 8;
ShaderArgs ShaderArgs_Color = 16; 
ShaderArgs ShaderArgs_Instanced = 32;

const UniformLocations ShaderFactory::emptyUniforms;

ShaderFactory::ShaderFactory()
    : shaderProgram(0)
{
    // InitShader creates, caches and selects the default program
    InitShader();
}

ShaderFactory::~ShaderFactory()
{
    // every program ever created by InitShader lives in the cache, including
    // the currently selected one, so deleting the cache frees them all
    for (const ShaderCacheTriplet& cached : cachedShaders)
    {
        glDeleteProgram(cached.second.second);
    }
    cachedShaders.clear();
    uniformCache.clear();
    currentUniforms = nullptr;
    this->shaderProgram = 0;
}

GLuint ShaderFactory::generateShaderProgram()
{
    GLuint program = glCreateProgram();
    this->shaderProgram = program;
    return program;
}

// returns true if shader is loaded by OpenGL else false
bool ShaderFactory::isShaderLoaded(GLuint program)
{
    return this->shaderProgram == program;
}

// returns non-zero value of shader program if shader is cached, else -1
int ShaderFactory::isShaderCached(const ShaderArgs& shaderArgs)
{
    for (ShaderCacheTriplet shader : cachedShaders)
    {   
        // if shader args match
        if (shader.second.first == shaderArgs) {
            return shader.second.second; // return the shader program
        }
    }
    return -1;
}

ShaderType ShaderFactory::convertExtraShaderArgsToType(const ShaderArgs& extraShaderArgs)
{
    ShaderType shaderType = ShaderType_Vertex | ShaderType_Fragment;
    if (extraShaderArgs & ShaderArgs_Tesselate_5)
    {
        shaderType |= ShaderType_TCS | ShaderType_TES;
        // this is used for calls to IsTesselationEnabled()
        this->tesselationEnabled = true;
    }
    else
    {
        this->tesselationEnabled = false;
    }
    return shaderType;
}

unsigned int ShaderFactory::cacheShaderAndReturnIndex(GLuint shaderProgram, const ShaderArgs& shaderArgs)
{
    unsigned int index = 0;
    bool indexInvalid = true;
    while (indexInvalid) {
        bool indexFound = false;
        for (int i = 0; i < cachedShaders.size(); i++) {
            if (cachedShaders[i].first == index) {
                indexFound = true;
            }
        }
        if (!indexFound) indexInvalid = false;
        else index++;
    }
    // from this point on, index is valid
    ShaderCacheTriplet sct = std::make_pair(index, std::make_pair(shaderArgs, shaderProgram));
    cachedShaders.push_back(sct);
    std::cout << "creating cached shader: " << shaderArgs << " , " << index << std::endl;
    return index;
}

// returns the shader location (unsigned int)
unsigned int ShaderFactory::InitShader(const ShaderArgs& extraShaderArgs)
{
    ShaderType shaderType = convertExtraShaderArgsToType(extraShaderArgs);

    GLuint program = generateShaderProgram();

    // unload all shaders
    this->shaderMetadata.clear();

    // write shader to file
    MutableShader shader;
    unsigned int shaderLocation = cacheShaderAndReturnIndex(program, extraShaderArgs);
    for (int i = 0; i < 5; i++)
    {
        // if the shader is not already loaded, add its metadata.
        shader.type = shaderType & (1 << i);
        shader.args = extraShaderArgs;
        if (shader.type)
        {
            // load the MutableShader with data
            populateShader(shader);

            // generate the shader source; optionally dump it for debugging
            std::string objectString = shaderObjectToString(shader);
            std::string fileName = "mutable_shader_" + std::to_string(shaderLocation) + "." + getShaderTypeSuffix(shader.type);
            dumpGeneratedShader(fileName, objectString);
            shader.object.body = "";
            shader.object.variables.clear();

            // check the shader type
            GLuint macro = getShaderMacro(shader.type);
            if (macro == 0) {
                std::cout << fileName << " has an unknown shader type (" << shader.type << "), skipping..." << std::endl;
                continue;
            }

            // load, compile and then attach shader to the program
            GLuint shaderId = glCreateShader(macro);
            GLchar* shaderFile = (GLchar*)objectString.c_str();
            glShaderSource(shaderId, 1, &shaderFile, NULL);
            glCompileShader(shaderId);
            checkError(shaderId, fileName + " failed to compile...");
            glAttachShader(program, shaderId);
            glDeleteShader(shaderId);

            // bundle metadata
            ShaderMetadata metadata;
            metadata.object = shader;
            metadata.shader = shaderId;

            // save metadata into memory
            shaderMetadata.push_back(metadata);
        }
    }
    shader.type = shaderType;
    // link shader program
    glLinkProgram(program);
    bool linked = checkLinkError(program, "shader program " + std::to_string(shaderLocation) + " (args " + std::to_string(extraShaderArgs) + ") failed to link...");

    // resolve uniform locations once so draw calls never look them up by name;
    // an unlinked program keeps all locations at -1 (glUniform* ignores those)
    UniformLocations& locations = uniformCache[program];
    locations = UniformLocations();
    if (linked) locations.Load(program);
    selectUniforms(program);
    return shaderLocation;
}

void ShaderFactory::LoadShader(const ShaderArgs& extraShaderArgs)
{
    // get shader type
    int program = isShaderCached(extraShaderArgs);
    if (program == -1)
    {
        InitShader(extraShaderArgs);
    }
    else if (!isShaderLoaded(program)) {
        // switch shader program if not loaded
        GLuint castedProgram = (GLuint)program;
        glUseProgram(castedProgram);
        this->shaderProgram = castedProgram;
        selectUniforms(castedProgram);
    }
}

void ShaderFactory::selectUniforms(GLuint program)
{
    auto it = uniformCache.find(program);
    currentUniforms = it != uniformCache.end() ? &it->second : nullptr;
}

const UniformLocations& ShaderFactory::GetUniformLocations() const
{
    return currentUniforms ? *currentUniforms : emptyUniforms;
}

void ShaderFactory::checkError(GLuint obj, std::string errorMessage) {
    GLint  success;
    GLchar infoLog[512];
    glGetShaderiv(obj, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(obj, 512, NULL, infoLog);
        std::cout << errorMessage << "\n" << infoLog << std::endl;
    }
}

bool ShaderFactory::checkLinkError(GLuint program, std::string errorMessage) {
    GLint  success;
    GLchar infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cout << errorMessage << "\n" << infoLog << std::endl;
    }    return success == GL_TRUE;
}

bool ShaderFactory::IsTesselationEnabled()
{
    return this->tesselationEnabled;
}

void ShaderFactory::ReloadShader(const ShaderArgs& extraShaderArgs)
{
    if (extraShaderArgs != this->currentShaderArgs) {
        this->currentShaderArgs = extraShaderArgs;
        LoadShader(extraShaderArgs);
    }
}

GLuint ShaderFactory::getShaderMacro(const ShaderType& type)
{
    if (type & ShaderType_Vertex) return GL_VERTEX_SHADER;
    else if (type & ShaderType_Fragment) return GL_FRAGMENT_SHADER;
    else if (type & ShaderType_TCS) return GL_TESS_CONTROL_SHADER;
    else if (type & ShaderType_TES) return GL_TESS_EVALUATION_SHADER;
    return 0;
}

GLuint ShaderFactory::GetShaderProgram()
{
    return this->shaderProgram;
}

void ShaderFactory::populateShader(MutableShader& shader)
{
    // clear the shader object before loading contents
    shader.object.Clear();
    /**
     * Vertex Shader - DEFAULTS
     */
    if (shader.type & ShaderType_Vertex) {
        // add mvp and position
        shader.object.AddVariable(VarType::VertexAttributeInput, "vec3", "pos_vs_in", 0);
        if (shader.args & ShaderArgs_Instanced)
        {
            shader.object.AddVariable(VarType::VertexAttributeInput, "vec3", "instance_offset", 4);
        }
        shader.object.AddVariable(VarType::Uniform, "mat4", "projection");
        shader.object.AddVariable(VarType::Uniform, "mat4", "view");
        shader.object.AddVariable(VarType::Uniform, "mat4", "model");
        if (shader.args & ShaderArgs_Instanced)
        {
            shader.object.AddBody("gl_Position = projection * view * model * vec4(pos_vs_in + instance_offset, 1.0f);");
        }
        else
        {
            shader.object.AddBody("gl_Position = projection * view * model * vec4(pos_vs_in, 1.0f);");
        }
        // extras
        if (shader.args & ShaderArgs_Tesselate_5)
        {
            if (shader.args & ShaderArgs_Normal)
            {
                shader.object.AddVariable(VarType::VertexAttributeInput, "vec3", "normal_vs_in", 1);
                shader.object.AddVariable(VarType::Output, "vec3", "normal_cs_in");
                shader.object.AddBody("normal_cs_in = mat3(transpose(inverse(model))) * normal_vs_in;");
            }
        }
        else if (shader.args & ShaderArgs_Normal)
        {
            shader.object.AddVariable(VarType::VertexAttributeInput, "vec3", "normal_vs_in", 1);
            shader.object.AddVariable(VarType::Output, "vec3", "normal_fs_in");
            shader.object.AddBody("normal_fs_in = mat3(transpose(inverse(model))) * normal_vs_in;");
        } 
        
        // lighting shader needs fragment shader to have position variable
        if (shader.args & ShaderArgs_Lighting)
        {
            if (shader.args & ShaderArgs_Tesselate_5)
            {
                shader.object.AddVariable(VarType::Output, "vec3", "pos_cs_in");
                if (shader.args & ShaderArgs_Instanced)
                {
                    shader.object.AddBody("pos_cs_in = vec3(model * vec4(pos_vs_in + instance_offset, 1.0));");
                }
                else
                {
                    shader.object.AddBody("pos_cs_in = vec3(model * vec4(pos_vs_in, 1.0));");
                }
            } else {
                shader.object.AddVariable(VarType::Output, "vec3", "pos_fs_in");
                if (shader.args & ShaderArgs_Instanced)
                {
                    shader.object.AddBody("pos_fs_in = vec3(model * vec4(pos_vs_in + instance_offset, 1.0));");
                }
                else
                {
                    shader.object.AddBody("pos_fs_in = vec3(model * vec4(pos_vs_in, 1.0));");
                }
            }
        }

        if (shader.args & ShaderArgs_Texture)
        {
            shader.object.AddVariable(VarType::VertexAttributeInput, "vec2", "uv_vs_in", 2);
            if (shader.args & ShaderArgs_Tesselate_5)
            {
                shader.object.AddVariable(VarType::Output, "vec2", "uv_cs_in");
                shader.object.AddBody("uv_cs_in = uv_vs_in;");
            }
            else
            {
                shader.object.AddVariable(VarType::Output, "vec2", "uv_fs_in");
                shader.object.AddBody("uv_fs_in = uv_vs_in;");
            }
        }

        if (shader.args & ShaderArgs_Color) 
        {
            shader.object.AddVariable(VarType::VertexAttributeInput, "vec4", "color_vs_in", 3);
            if (shader.args & ShaderArgs_Tesselate_5)
            {
                shader.object.AddVariable(VarType::Output, "vec4", "color_cs_in");
                shader.object.AddBody("color_cs_in = color_vs_in;");
            }
            else
            {
                shader.object.AddVariable(VarType::Output, "vec4", "color_fs_in");
                shader.object.AddBody("color_fs_in = color_vs_in;");
            }
        }
    }
    /**
     * Fragment Shader - DEFAULTS
     */
    else if (shader.type & ShaderType_Fragment) {
        shader.object.AddVariable(VarType::Output, "vec4", "FinalColor");

        // add variables
        if (shader.args & ShaderArgs_Texture)
        {
            shader.object.AddVariable(VarType::Input, "vec2", "uv_fs_in");
            shader.object.AddVariable(VarType::Uniform, "sampler2D", "Texture");
        }
        if (shader.args & ShaderArgs_Normal)
        {
            shader.object.AddVariable(VarType::Input, "vec3", "normal_fs_in");
        }
        if (shader.args & ShaderArgs_Color)
        {
            shader.object.AddVariable(VarType::Input, "vec4", "color_fs_in");
            shader.object.AddVariable(VarType::Uniform, "vec3", "solidColor");
            shader.object.AddVariable(VarType::Uniform, "int", "useVertexColor");
        }
        if (shader.args & ShaderArgs_Lighting)
        {
            // lighting shader needs fragment to have position variable
            shader.object.AddVariable(VarType::Input, "vec3", "pos_fs_in");

            // import variables and glsl helpers
            shader.object.AddPreBody(readFileContents("lighting_variables.glsl"));
            shader.object.AddPreBody(readFileContents("bitwise_operators.glsl"));
            shader.object.AddPreBody(readFileContents("lighting_calc_dir_light.glsl"));
            shader.object.AddPreBody(readFileContents("lighting_calc_point_light.glsl"));
            shader.object.AddPreBody(readFileContents("lighting_calc_spot_light.glsl"));
            // import main function
            shader.object.AddBody(readFileContents("lighting_main.glsl"));
        } else {
            // add final color
            std::string colorVector = "";
            if (shader.args & ShaderArgs_Texture)
            {
                colorVector = "texture(Texture, uv_fs_in);";
            }
            else if (shader.args & ShaderArgs_Normal)
            {
                colorVector = "vec4(normal_fs_in, 1.0f);";
            }
            else if (shader.args & ShaderArgs_Lighting)
            {
                colorVector = "vec4(result, 1.0f);";
            }
            else if (shader.args & ShaderArgs_Color)
            {
                colorVector = "(useVertexColor != 0 ? color_fs_in : vec4(solidColor, 1.0f));";
            }
            else
            {
                colorVector = "vec4(0.5f, 0.5f, 0.5f, 1.0f);";
            }

            std::string finalColor = "FinalColor = " + colorVector;
            shader.object.AddBody(finalColor);
        }
    }
    /**
     * Tesselation Control Shader - DEFAULTS
     */
    else if (shader.type & ShaderType_TCS)
    {
        if (shader.args & ShaderArgs_Tesselate_5)
        {
            shader.object.AddCustomVariable("layout (vertices = 3) out;");
            shader.object.AddBody("\
if (gl_InvocationID == 0)\n\
{\n\
    gl_TessLevelOuter[0] = 5;\n\
    gl_TessLevelOuter[1] = 5;\n\
    gl_TessLevelOuter[2] = 5;\n\
    gl_TessLevelInner[0] = 5;\n\
}\n\
gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;\n");
            if (shader.args & ShaderArgs_Texture)
            {
                shader.object.AddCustomVariable("in vec2 uv_cs_in[];");
                shader.object.AddCustomVariable("out vec2 uv_es_in[];");
                shader.object.AddBody("uv_es_in[gl_InvocationID] = uv_cs_in[gl_InvocationID];\n");
            }
            if (shader.args & ShaderArgs_Normal)
            {  
                shader.object.AddCustomVariable("in vec3 normal_cs_in[];");
                shader.object.AddCustomVariable("out vec3 normal_es_in[];");
                shader.object.AddBody("normal_es_in[gl_InvocationID] = normal_cs_in[gl_InvocationID];");
            }
            if (shader.args & ShaderArgs_Lighting)
            {
                shader.object.AddCustomVariable("in vec3 pos_cs_in[];");
                shader.object.AddCustomVariable("out vec3 pos_es_in[];");
                shader.object.AddBody("pos_es_in[gl_InvocationID] = pos_cs_in[gl_InvocationID];");
            }
            if (shader.args & ShaderArgs_Color)
            {
                shader.object.AddCustomVariable("in vec4 color_cs_in[];");
                shader.object.AddCustomVariable("out vec4 color_es_in[];");
                shader.object.AddBody("color_es_in[gl_InvocationID] = color_cs_in[gl_InvocationID];");
            }
        }
        
    }
    /**
     * Tesselation Evaluation Shader - DEFAULTS
     */
    else if (shader.type & ShaderType_TES)
    {
        if (shader.args & ShaderArgs_Tesselate_5)
        {
            shader.object.AddCustomVariable("layout(triangles, equal_spacing, ccw) in;");
            if (shader.args & ShaderArgs_Tesselate_5)
            {
                shader.object.AddBody("float u = gl_TessCoord.x;\n\
float v = gl_TessCoord.y;\n\
float w = gl_TessCoord.z;\n\
gl_Position = u * gl_in[0].gl_Position + v * gl_in[1].gl_Position + w * gl_in[2].gl_Position;\n");
                if (shader.args & ShaderArgs_Texture)
                {
                    shader.object.AddCustomVariable("in vec2 uv_es_in[];");
                    shader.object.AddCustomVariable("out vec2 uv_fs_in;");
                    shader.object.AddBody("uv_fs_in = u * uv_es_in[0] + v * uv_es_in[1] + w * uv_es_in[2];\n");
                }
                if (shader.args & ShaderArgs_Normal)
                {
                    shader.object.AddCustomVariable("in vec3 normal_es_in[];");
                    shader.object.AddCustomVariable("out vec3 normal_fs_in;");
                    shader.object.AddBody("normal_fs_in = u * normal_es_in[0] + v * normal_es_in[1] + w * normal_es_in[2];\n");
                }
            }
            // lighting needs fragment shader to have vertex position
            if (shader.args & ShaderArgs_Lighting)
            {
                shader.object.AddCustomVariable("in vec3 pos_es_in[];");
                shader.object.AddVariable(VarType::Output, "vec3", "pos_fs_in");
                shader.object.AddBody("pos_fs_in = u * pos_es_in[0] + v * pos_es_in[1] + w * pos_es_in[2];");
            }
            if (shader.args & ShaderArgs_Color)
            {
                shader.object.AddCustomVariable("in vec4 color_es_in[];");
                shader.object.AddVariable(VarType::Output, "vec4", "color_fs_in");
                shader.object.AddBody("color_fs_in = u * color_es_in[0] + v * color_es_in[1] + w * color_es_in[2];");
            }
        }
    }
}

std::string ShaderFactory::getShaderTypeSuffix(const ShaderType& type)
{
    if (type & ShaderType_Vertex) return "vs";
    else if (type & ShaderType_Fragment) return "fs";
    else if (type & ShaderType_TCS) return "tcs";
    else if (type & ShaderType_TES) return "tes";
    return "";
}

void ShaderFactory::writeMutableVariables(const ShaderType& shaderType, const std::vector<MutableVar> mutableVariables, std::string& out)
{
    for (MutableVar var : mutableVariables)
    {
        switch (var.type)
        {
            case VarType::VertexAttributeInput:
                out += "layout (location = " + std::to_string(var.attribIndex);
                out += ") in " + var.glType;
                out += " " + var.name + ";";
                break;
            case VarType::Input:
                out += "in " + var.glType + " " + var.name + ";";
                break;
            case VarType::Output:
                out += "out " + var.glType;
                out += " " + var.name + ";";
                break;
            case VarType::Uniform:
                out += "uniform " + var.glType;
                out += " " + var.name + ";";
                break;
            case VarType::Custom:
                out += var.name;
                break;
            case VarType::Primitive:
                out += var.glType + " " + var.name + ";";
                break;
        }
        out += "\n";
    }
}

std::string ShaderFactory::shaderObjectToString(const MutableShader& shader) 
{
    std::string out = OGL_VERSION + "\n\n";
    const MutableObject& mutableObject = shader.object;
    // 1. write structs
    for (MutableStruct mutableStruct : mutableObject.structs)
    {
        out += "struct " + mutableStruct.name  + " {\n";
        // variables from struct
        writeMutableVariables(ShaderType_NONE, mutableStruct.variables, out);
        out += mutableStruct.body + "\n";
        out += "};\n";
    }
    // 2. write variables to out string
    writeMutableVariables(shader.type, mutableObject.variables, out);

    // 3. add pre-body variables/structs (if any)
    out += shader.object.prebody + "\n";

    // 4. write body within main 
    out += "\nvoid main() { \n";
    out += shader.object.body;
    out += "} \n";
    return out;
}

// Generated shaders are compiled straight from memory. Writing them to disk is
// only a debugging aid, so it is opt-in: set KUBECHASER_SHADER_DUMP_DIR to a
// writable directory to get a copy of every generated shader. Nothing is ever
// written into the source tree, so read-only checkouts and installed builds work.
bool ShaderFactory::dumpGeneratedShader(const std::string& fileName, const std::string& content) {
    const char* dumpDir = std::getenv(SHADER_DUMP_DIR_ENV);
    if (dumpDir == nullptr || dumpDir[0] == '\0') {
        return false;
    }

    std::error_code ec;
    std::filesystem::path dir(dumpDir);
    std::filesystem::create_directories(dir, ec);
    if (ec) {
        std::cout << "could not create shader dump directory " << dir.string() << ": " << ec.message() << std::endl;
        return false;
    }

    std::filesystem::path path = dir / fileName;
    std::ofstream file(path);
    if (!file) {
        std::cout << "could not write generated shader to " << path.string() << std::endl;
        return false;
    }
    file << content;
    file.close();
    if (!file) {
        std::cout << "could not write generated shader to " << path.string() << std::endl;
        return false;
    }
    return true;
}

std::string ShaderFactory::readFileContents(std::string fileName) {
    std::string fileStr;
    std::string line;
    std::ifstream file(SHADER_DIR + fileName);
    if (file.is_open()) {
        while (std::getline(file, line)) {
            fileStr += line + '\n';
        }
        file.close();
    }
    return fileStr;
}

void ShaderFactory::SetShaderUniforms(const DrawAttributes& da, const glm::vec3& cameraPos) {
    const UniformLocations& u = GetUniformLocations();
    if (da.lightBlinn || da.lightSwitch != 0) {
        glUniform1i(u.blinn, da.lightBlinn ? 1 : 0);
        glUniform1i(u.usingTexture, da.texture ? 1 : 0);
        glUniform1i(u.lightSwitch, da.lightSwitch);
        glUniform3f(u.viewPos, cameraPos.x, cameraPos.y, cameraPos.z);
        glUniform3f(u.lightColor, 1.0f, 1.0f, 1.0f);

        glUniform3f(u.dirLight.direction, -0.2f, -1.0f, -0.3f);
        glUniform3f(u.dirLight.ambient, 0.3f, 0.3f, 0.3f);
        glUniform3f(u.dirLight.diffuse, 0.4f, 0.4f, 0.4f);
        glUniform3f(u.dirLight.specular, 0.2f, 0.5f, 0.5f);

        for (const UniformLocations::Light& light : u.pointLights) {
            glUniform3f(light.position, -0.2f, -1.0f, -0.3f);
            glUniform3f(light.ambient, 0.05f, 0.05f, 0.05f);
            glUniform3f(light.diffuse, 0.4f, 0.4f, 0.4f);
            glUniform3f(light.specular, 0.5f, 0.5f, 0.5f);
            glUniform1f(light.constant, 1.0f);
            glUniform1f(light.linear, 0.09f);
            glUniform1f(light.quadratic, 0.032f);
        }

        glUniform3f(u.spotlight.position, 0.5f, 0.0f, 3.0f);
        glUniform3f(u.spotlight.direction, 0.0f, 0.0f, -1.0f);

        glUniform3f(u.spotlight.ambient, 0.0f, 0.0f, 0.0f);
        glUniform3f(u.spotlight.diffuse, 0.6f, 0.6f, 0.6f);
        glUniform3f(u.spotlight.specular, 1.0f, 1.0f, 1.0f);

        glUniform1f(u.spotlight.constant, 1.0f);
        glUniform1f(u.spotlight.linear, 0.09f);
        glUniform1f(u.spotlight.quadratic, 0.032f);

        static const float spotCutOff = glm::cos(glm::radians(12.5f));
        static const float spotOuterCutOff = glm::cos(glm::radians(15.0f));
        glUniform1f(u.spotlight.cutOff, spotCutOff);
        glUniform1f(u.spotlight.outerCutOff, spotOuterCutOff);
    }
    const MaterialAttributes& material = da.materialAttributes;
    glUniform1f(u.materialShininess, material.shininess);
    glUniform1i(u.materialDiffuseMap, material.diffuseMap);
    glUniform3f(u.materialDiffuse, material.diffuse.x, material.diffuse.y, material.diffuse.z);
    glUniform3f(u.materialSpecular, material.specular.x, material.specular.y, material.specular.z);
    if (da.color) {
        glUniform3f(u.solidColor, da.solidColor.x, da.solidColor.y, da.solidColor.z);
        glUniform1i(u.useVertexColor, da.useVertexColor ? 1 : 0);
    }
}

void ShaderFactory::SetFloat(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(shaderProgram, name.c_str()), value);
}

void ShaderFactory::SetInt(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(shaderProgram, name.c_str()), value);
}

void ShaderFactory::SetVec3(const std::string& name, float x, float y, float z) const
{
    glUniform3f(glGetUniformLocation(shaderProgram, name.c_str()), x, y, z);
}

void ShaderFactory::SetVec3(const std::string& name, glm::vec3 vec) const
{
    glUniform3f(glGetUniformLocation(shaderProgram, name.c_str()), vec.x, vec.y, vec.z);
}

void ShaderFactory::Use(const DrawAttributes& da, const glm::vec3& cameraPos) {
    ShaderArgs args = 0;
    // add draw attribute configs here
    if (da.tesselate) args |= ShaderArgs_Tesselate_5;
    if (da.normal) args |= ShaderArgs_Normal;
    if (da.texture) args |= ShaderArgs_Texture;
    if (da.lightBlinn || da.lightSwitch) {
        args |= ShaderArgs_Lighting;
        // lighting shader REQUIRES normals for blinn-phong calculation
        args |= ShaderArgs_Normal;
        // lighting shader REQUIRES texture support (even if it doesn't use textures)
        args |= ShaderArgs_Texture;
    }
    if (da.color) args |= ShaderArgs_Color;
    if (da.instanced) args |= ShaderArgs_Instanced;

    // end draw attribute configs
    ReloadShader(args);
    glUseProgram(shaderProgram);
    SetShaderUniforms(da, cameraPos);
}
