#ifndef MESH_H
#define MESH_H
#include "Entity.h"
#include "utility/Reader.h"
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

class Mesh : public Entity
{
private:
    int num_vertices, num_indices;
    glm::vec4 v1, v2, v3, v4, v5, v6, v7, v8;

    void initialize(
        Eigen::MatrixXf& V, 
        Eigen::MatrixXi& F,
        Eigen::MatrixXf& UV, 
        Eigen::MatrixXi& UF, 
        Eigen::MatrixXf& NV, 
        Eigen::MatrixXi& NF, 
        const std::string& fileNameOBJ,
        const std::string& fileNameMTL = "",
        const std::string& textureName = "");
    void generate(
        const Eigen::MatrixXf& V, 
        const Eigen::MatrixXi& F,
        const Eigen::MatrixXf& UV, 
        const Eigen::MatrixXi& UF, 
        const Eigen::MatrixXf& NV, 
        const Eigen::MatrixXi& NF,
        const std::string& textureName);
    void generateBoundingBox(const Eigen::MatrixXf& V);
    bool rayIntersectBox(const Ray& ray, double& t, float minx, float maxx, float miny, float maxy, float minz, float maxz);  
    void solveMinMax(glm::vec4& u0, glm::vec4& u1, glm::vec4& u2, glm::vec4& u3, glm::vec4& u4,
        glm::vec4& u5, glm::vec4& u6, glm::vec4& u7, float& minx, float& maxx, float& miny, 
        float& maxy, float& minz, float& maxz);
    void addToMap(std::unordered_map<int, std::unordered_set<int>>& map, const int& key, const int& val);

public:
	Mesh();
    Mesh(const std::string& fileNameOBJ, const std::string& fileNameMTL = "", const std::string& textureName = "");
	void Draw(const std::shared_ptr<ShaderFactory>& factory, const std::shared_ptr<Camera>& camera) override;
	void SetObjectPicked() override;
	bool Intersect(const std::shared_ptr<Camera>& camera, const Ray& ray, double& t) override;   
};
#endif
