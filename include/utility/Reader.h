#ifndef READER_H
#define READER_H
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include "utility/Material.h"

#ifndef ASSET_DIR
#define ASSET_DIR "./assets"
#endif

typedef Eigen::Triplet<float> T_float;
typedef Eigen::Triplet<int> T_int;

class Reader
{
public:
    static std::string MESH_DIR;
	static void ScanOBJ(const std::string& fileName);
    static void ReadOBJ(const std::string& fileName,
        std::string& texture_file,
        Eigen::MatrixXf& V,
        Eigen::MatrixXi& F,
        Eigen::MatrixXf& UV,
        Eigen::MatrixXi& UF,
        Eigen::MatrixXf& NV,
        Eigen::MatrixXi& NF,
        std::vector<Material>& materials);

    static bool findMaterialKeyAndPopulateValue(const std::string& line, const std::string& key, std::string& value);
    static bool findMaterialKeyAndPopulateValue(const std::string& line, const std::string& key, float& value);
    static bool findMaterialKeyAndPopulateValue(const std::string& line, const std::string& key, int& value);
    static bool findMaterialKeyAndPopulateValue(const std::string& line, const std::string& key, glm::vec3& value);

    // Appends the materials in MESH_DIR + fileName; map_Kd paths are returned relative to MESH_DIR
    static void ReadMTL(const std::string& fileName, std::vector<Material>& materials);
    // Index of the material used by the most faces (see Material::faceCount), or -1 if there are none
    static int PrimaryMaterialIndex(const std::vector<Material>& materials);
    static int resolveIndex(int index, size_t definedCount);

    // Final component of a path, accepting both '/' and '\\' as separators.
    // Returns the input unchanged when it contains no separator.
    static std::string FileNameFromPath(const std::string& path);

    // Sorted file names of the regular files in directory with a ".obj"
    // extension (case-insensitive). Returns an empty list if the directory
    // doesn't exist or can't be read.
    static std::vector<std::string> ListOBJFiles(const std::string& directory);

    static std::vector<std::string> split(const std::string& line, const std::string& delimiter);
};
#endif
