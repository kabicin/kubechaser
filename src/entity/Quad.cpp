#include "entity/Quad.h"

Quad::Quad()
{
    initialize();
}

Quad::Quad(glm::vec3 offset)
{
    initialize();
    this->Translate(offset);
}

void Quad::initialize()
{
    // init vertices
    Vertex vertices[] = {
        // position               // color          // texture
        Vertex(-0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f),
        Vertex(-0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f),
        Vertex(0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f),
        Vertex(0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f)
    };

    int indices[] = {
        0, 1, 2,
        2, 1, 3
    };

    // create VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // create VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // create EBO
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

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
    v3 = glm::vec4(vertices[3].Pos[0], vertices[3].Pos[1], vertices[3].Pos[2], 1.0f);
}

Quad::~Quad()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Quad::Draw(const std::shared_ptr<ShaderFactory>& factory, const std::shared_ptr<Camera>& camera)
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
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    if (da.wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Quad::SetObjectPicked()
{
    usingTexture = !usingTexture;
}

bool Quad::Intersect(const std::shared_ptr<Camera>& camera, const Ray& ray, double& t)
{
    glm::mat4 model = camera->GenerateModel(GetCameraAttributes());

    // move from local to world coordinates
    glm::vec4 u0, u1, u2, u3;
    u0 = model * v0;
    u1 = model * v1;
    u2 = model * v2;
    u3 = model * v3;
    
    // get directions
    Eigen::Vector3d p1(u0.x, u0.y, u0.z);
    Eigen::Vector3d d1(u2.x - u0.x, u2.y - u0.y, u2.z - u0.z);
    Eigen::Vector3d d2(u1.x - u0.x, u1.y - u0.y, u1.z - u0.z);
   
    Eigen::Vector3d p2(u3.x, u3.y, u3.z);
    Eigen::Vector3d d3(u1.x - u3.x, u1.y - u3.y, u1.z - u3.z);
    Eigen::Vector3d d4(u2.x - u3.x, u2.y - u3.y, u2.z - u3.z);
   
    Eigen::Vector3d n = d1.cross(d2);
    n = n / n.norm();

    // if plane is parallel to the ray return false
    if (n.dot(ray.direction) == 0)
        return false;

    // split into two triangle intersection problems
    Eigen::Matrix3d m1, m2;
    m1.col(0) = d1;
    m1.col(1) = d2;
    m1.col(2) = -ray.direction;
    m2.col(0) = d3;
    m2.col(1) = d4;
    m2.col(2) = -ray.direction;
    
    Eigen::Vector3d v1 = m1.colPivHouseholderQr().solve(ray.eye - p1);
    Eigen::Vector3d v2 = m2.colPivHouseholderQr().solve(ray.eye - p2);

    double a1 = v1(0), b1 = v1(1), t1 = v1(2);
    double a2 = v2(0), b2 = v2(1), t2 = v2(2);

    if (t1 > 0 && a1 >= 0 && b1 >= 0 && a1 + b1 <= 1 ||
        t2 > 0 && a2 >= 0 && b2 >= 0 && a2 + b2 <= 1)
    {
        t = std::min(t1, t2);
        std::cout << "Quad Intersection: t=" << t << " n: (" << n(0) << "," << n(1) << "," << n(2) << ")" << std::endl;
        return true;
    }
    return false;
}
