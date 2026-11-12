#ifndef MESH_H
#define MESH_H
#include "Entity.h"
#include "utility/Reader.h"
#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Mesh : public Entity
{
private:
    int num_vertices = 0, num_indices = 0;
    glm::vec4 v1, v2, v3, v4, v5, v6, v7, v8;
    std::string sourceFile;
    // CPU copy of the uploaded triangles, shared with the parsed-OBJ cache.
    std::shared_ptr<const std::vector<NVertex>> vertices;
    glm::vec3 boundsLo = glm::vec3(0.0f), boundsHi = glm::vec3(0.0f);

    void upload(const std::vector<NVertex>& vertices);
    void setTexture(const std::string& textureFile);
    void setBoundingBox(const glm::vec3& lo, const glm::vec3& hi);
    bool rayIntersectBox(const Ray& ray, double& t, float minx, float maxx, float miny, float maxy, float minz, float maxz);  
    void solveMinMax(glm::vec4& u0, glm::vec4& u1, glm::vec4& u2, glm::vec4& u3, glm::vec4& u4,
        glm::vec4& u5, glm::vec4& u6, glm::vec4& u7, float& minx, float& maxx, float& miny, 
        float& maxy, float& minz, float& maxz);

public:
	Mesh();
    Mesh(const std::string& fileNameOBJ, const std::string& fileNameMTL = "", const std::string& textureName = "");
    ~Mesh() override;
	void Draw(const std::shared_ptr<ShaderFactory>& factory, const std::shared_ptr<Camera>& camera) override;
	bool Intersect(const std::shared_ptr<Camera>& camera, const Ray& ray, double& t) override;   

    const std::string& GetSourceFile() const { return sourceFile; }
    // Triangle list (3 vertices per triangle) in model space; empty for a
    // default-constructed or unreadable mesh.
    const std::vector<NVertex>& GetVertices() const;
    glm::vec3 GetBoundsMin() const { return boundsLo; }
    glm::vec3 GetBoundsMax() const { return boundsHi; }

    // Drops every cached parsed OBJ. Meshes created afterwards re-read their file.
    static void ClearCache();
};
#endif
