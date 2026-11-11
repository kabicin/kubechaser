#include "utility/Reader.h"

#include <cmath>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

bool approximatelyEqual(float lhs, float rhs)
{
    return std::abs(lhs - rhs) < 1e-6f;
}

} // namespace

int main()
{
    const std::vector<std::string> values = Reader::split("alpha,beta,gamma", ",");
    if (values != std::vector<std::string>{"alpha", "beta", "gamma"}) {
        std::cerr << "Reader::split returned unexpected values.\n";
        return 1;
    }

    std::string name;
    if (!Reader::findMaterialKeyAndPopulateValue("newmtl kube", "newmtl", name)
        || name != "kube") {
        std::cerr << "Reader did not parse a string material property.\n";
        return 1;
    }

    float shininess = 0.0f;
    if (!Reader::findMaterialKeyAndPopulateValue("Ns 32.5", "Ns", shininess)
        || !approximatelyEqual(shininess, 32.5f)) {
        std::cerr << "Reader did not parse a float material property.\n";
        return 1;
    }

    glm::vec3 diffuse(0.0f);
    if (!Reader::findMaterialKeyAndPopulateValue("Kd 0.1 0.2 0.3", "Kd", diffuse)
        || !approximatelyEqual(diffuse.x, 0.1f)
        || !approximatelyEqual(diffuse.y, 0.2f)
        || !approximatelyEqual(diffuse.z, 0.3f)) {
        std::cerr << "Reader did not parse a vector material property.\n";
        return 1;
    }

    Reader::MESH_DIR = std::string(KUBECHASER_TEST_FIXTURE_DIR) + "/";
    std::string textureFile;
    Eigen::MatrixXf vertices;
    Eigen::MatrixXi faces;
    Eigen::MatrixXf textureCoordinates;
    Eigen::MatrixXi textureFaces;
    Eigen::MatrixXf normals;
    Eigen::MatrixXi normalFaces;
    std::vector<Material> materials;

    try {
        Reader::ReadOBJ(
            "commented_triangle.obj",
            textureFile,
            vertices,
            faces,
            textureCoordinates,
            textureFaces,
            normals,
            normalFaces,
            materials);
    } catch (const std::exception& error) {
        std::cerr << "Reader treated an OBJ comment as geometry: " << error.what() << '\n';
        return 1;
    }

    if (vertices.rows() != 3
        || textureCoordinates.rows() != 3
        || normals.rows() != 1) {
        std::cerr << "Reader returned unexpected geometry for the commented OBJ fixture.\n";
        return 1;
    }

    // larger than the old fixed 42,000-row buffers, with extra per-vertex color values
    const int largeVertexCount = 50000;
    const std::filesystem::path largeDir = std::filesystem::temp_directory_path() / "kubechaser_reader_test";
    std::filesystem::create_directories(largeDir);
    {
        std::ofstream largeObj(largeDir / "large.obj");
        for (int i = 0; i < largeVertexCount; ++i) {
            largeObj << "v " << i << " 0 0 0.5 0.5 0.5\n";
            largeObj << "vt 0 0\n";
        }
        for (int i = 0; i + 2 < largeVertexCount; i += 3) {
            largeObj << "f " << i + 1 << "/" << i + 1 << " " << i + 2 << "/" << i + 2 << " " << i + 3 << "/" << i + 3 << "\n";
        }
    }

    Reader::MESH_DIR = largeDir.string() + "/";
    materials.clear();
    Reader::ReadOBJ(
        "large.obj",
        textureFile,
        vertices,
        faces,
        textureCoordinates,
        textureFaces,
        normals,
        normalFaces,
        materials);
    std::filesystem::remove_all(largeDir);

    if (vertices.rows() != largeVertexCount
        || textureCoordinates.rows() != largeVertexCount
        || !approximatelyEqual(vertices(largeVertexCount - 1, 0), static_cast<float>(largeVertexCount - 1))
        || !approximatelyEqual(vertices(largeVertexCount - 1, 2), 0.0f)) {
        std::cerr << "Reader returned unexpected geometry for a large OBJ.\n";
        return 1;
    }

    return 0;
}
