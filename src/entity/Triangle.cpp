#include "entity/Triangle.h"

Triangle::Triangle()
{
    initialize();
}

Triangle::Triangle(glm::vec3 offset)
{
    initialize();
    this->Translate(offset);
}

void Triangle::initialize()
{
    // init vertices
    Vertex vertices[] = {
        // position         // color          // texture
        Vertex(-0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f),
        Vertex(0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f),
        Vertex(0.0f,  0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 0.5f, 1.0f)
    };

    // create VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // create VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // pos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, Color)));
    glEnableVertexAttribArray(1);

    // texture
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, TexturePos)));
    glEnableVertexAttribArray(2);

    v0 = glm::vec4(vertices[0].Pos[0], vertices[0].Pos[1], vertices[0].Pos[2], 1.0f);
    v1 = glm::vec4(vertices[1].Pos[0], vertices[1].Pos[1], vertices[1].Pos[2], 1.0f);
    v2 = glm::vec4(vertices[2].Pos[0], vertices[2].Pos[1], vertices[2].Pos[2], 1.0f);
}

Triangle::~Triangle()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void Triangle::SetObjectPicked()
{
    usingTexture = !usingTexture;
}

void stencil()
{
    glEnable(GL_STENCIL_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glStencilFunc(GL_EQUAL, 1, 0xFF);
}

void Triangle::Draw(const std::shared_ptr<ShaderFactory>& factory, const std::shared_ptr<Camera>& camera)
{
    factory->Use(da, camera->GetCameraPos());

    camera->Render(factory->GetShaderProgram(), GetCameraAttributes());

    if (TEX != -1)
        glBindTexture(GL_TEXTURE_2D, TEX);
    glBindVertexArray(VAO);
    
    if (da.wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    if (factory->IsTesselationEnabled() && da.tesselate)
    {
        glPatchParameteri(GL_PATCH_VERTICES, 3);
    }
    glDrawArrays(GL_TRIANGLES, 0, 3);

    if (da.wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

bool Triangle::Intersect(const std::shared_ptr<Camera>& camera, const Ray& ray, double& t)
{
    // on each render, update ray casting vectors
    glm::mat4 model = camera->GenerateModel(GetCameraAttributes());

    glm::vec4 u0 = model * v0;
    glm::vec4 u1 = model * v1;
    glm::vec4 u2 = model * v2;

    Eigen::Vector3d normal;
    return Linalg::RayBaryIntersect(ray, 
        Eigen::Vector3d(u0.x, u1.y, u2.z), 
        Eigen::Vector3d(u1.x - u0.x, u1.y - u0.y, u1.z - u0.z), 
        Eigen::Vector3d(u2.x - u0.x, u2.y - u0.y, u2.z - u0.z),
        normal, t);
}
