#ifndef READER_H
#define READER_H
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
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

    static bool startsWithTab(const std::string& line);

    static std::vector<std::string> split(const std::string& line, const std::string& delimiter);
};
#endif
