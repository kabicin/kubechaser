#include "shader/Shader.h"

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath, const std::string& tcsName, const std::string& tesName)
{
    shaderProgram = glCreateProgram();
    initialize(shaderProgram, vertexPath, fragmentPath);
    if (tcsName != "" && tesName != "")
    {
        addTesselationShader(shaderProgram, tcsName, tesName);
    }
    glLinkProgram(shaderProgram);
}

void Shader::initialize(const GLuint& shaderProgram, const std::string& vertexPath, const std::string& fragmentPath)
{
    GLuint vertexShader, fragmentShader;
    // load vertex shader
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    std::string vertexShaderFileContents = getFileContents(vertexPath);
    GLchar* vertexShaderFile = (GLchar*)vertexShaderFileContents.c_str();
    glShaderSource(vertexShader, 1, &vertexShaderFile, NULL);
    glCompileShader(vertexShader);
    checkError(vertexShader, "Vertex Shader Failed");

    // load fragment shader
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    std::string fragmentShaderFileContents = getFileContents(fragmentPath);
    GLchar* fragmentShaderFile = (GLchar*)fragmentShaderFileContents.c_str();
    glShaderSource(fragmentShader, 1, &fragmentShaderFile, NULL);
    glCompileShader(fragmentShader);
    checkError(fragmentShader, "Fragment Shader Failed");

    // load global shader program
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    checkError(shaderProgram, "Shader Program Failed");

    // shaders are linked into the program, so we can delete shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

Shader::~Shader() {
    glDeleteProgram(shaderProgram);
}

void Shader::addTesselationShader(const GLuint& shaderProgram, const std::string& tcsName, const std::string& tesName)
{
    GLuint tcsShader, tesShader;

    // load tcs shader
    tcsShader = glCreateShader(GL_TESS_CONTROL_SHADER);
    std::string tcsShaderFileContents = getFileContents(tcsName);
    GLchar* tcsShaderFile = (GLchar*)tcsShaderFileContents.c_str();
    glShaderSource(tcsShader, 1, &tcsShaderFile, NULL);
    glCompileShader(tcsShader);
    checkError(tcsShader, "Tesselation Control Shader Failed");

    // load tes shader
    tesShader = glCreateShader(GL_TESS_EVALUATION_SHADER);
    std::string tesShaderFileContents = getFileContents(tesName);
    GLchar* tesShaderFile = (GLchar*)tesShaderFileContents.c_str();
    glShaderSource(tesShader, 1, &tesShaderFile, NULL);
    glCompileShader(tesShader);
    checkError(tesShader, "Tesselation Eval Shader Failed");

    glAttachShader(shaderProgram, tcsShader);
    glAttachShader(shaderProgram, tesShader);

    glDeleteShader(tcsShader);
    glDeleteShader(tesShader);
}

void Shader::checkError(GLuint obj, std::string errorMessage) {
    GLint  success;
    GLchar infoLog[512];
    glGetShaderiv(obj, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(obj, 512, NULL, infoLog);
        std::cout << errorMessage << "\n" << infoLog << std::endl;
    }
}

void Shader::Use() {
    glUseProgram(shaderProgram);
}


std::string Shader::getFileContents(std::string fileName) {
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

void Shader::SetFloat(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(shaderProgram, name.c_str()), value);
}

void Shader::SetInt(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(shaderProgram, name.c_str()), value);
}

void Shader::SetVec3(const std::string& name, float x, float y, float z) const
{
    glUniform3f(glGetUniformLocation(shaderProgram, name.c_str()), x, y, z);
}

