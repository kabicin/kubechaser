#include "utility/Reader.h"
#include <unordered_map>
#include <bitset>
#include <climits>
#include <sys/stat.h>

void Reader::ScanOBJ(const std::string& fileName)
{
    std::cout << "====== Scanning OBJ file =====" << std::endl << fileName << std::endl;
    std::unordered_map<std::string, int> umap;
    std::ifstream file(MESH_DIR + fileName);
    if (file.is_open())
    {
        std::istringstream iss;
        std::string line;
        while (std::getline(file, line))
        {
            if (line.find("#") == 0) continue;
            //std::cout << line << std::endl;
            std::istringstream liness(line);
            std::string token;
            int c = 0;
            while (liness >> token)
            {
                if (c == 0)
                {
                    if (umap.find(token) != umap.end())
                    {
                        umap[token]++;
                    }
                    else
                    {
                        umap[token] = 1;
                    }
                }
                c++;
            }
        }
    }
    file.close();
    std::cout << "====== RESULTS ======" << std::endl;
    std::ofstream outfile;
    outfile.open(MESH_DIR + fileName + "_scan_out.txt");

    for (auto it = umap.begin(); it != umap.end(); ++it)
    {
        outfile << it->first << " " << it->second << std::endl;
        std::cout << it->first << " " << it->second << std::endl;
    }
    outfile.close();
    std::cout << "====== Finished Scan ======" << std::endl;
}

bool Reader::findMaterialKeyAndPopulateValue(const std::string& line, const std::string& key, std::string& value)
{
    std::string keyWithSpace = key + " ";
    if (line.find(keyWithSpace) != std::string::npos)
    {
        value = line.substr(line.find(keyWithSpace) + keyWithSpace.length());
        if (value.find_first_not_of(" ") != std::string::npos)
        {
            value = value.substr(value.find_first_not_of(" "));
        }
        return true;
    }
    return false;                 
}

bool Reader::findMaterialKeyAndPopulateValue(const std::string& line, const std::string& key, float& value)
{
    std::string keyWithSpace = key + " ";
    std::string tempValue;
    if (line.find(keyWithSpace) != std::string::npos)
    {
        tempValue = line.substr(line.find(keyWithSpace) + keyWithSpace.length());
        if (tempValue.find_first_not_of(" ") != std::string::npos)
        {
            value = std::stof(tempValue.substr(tempValue.find_first_not_of(" ")));
        }
        return true;
    }
    return false;                   
}

std::vector<std::string> Reader::split(const std::string& line, const std::string& delimiter)
{
    std::vector<std::string> values;
    size_t last = 0; 
    size_t next = 0; 
    while ((next = line.find(delimiter, last)) != std::string::npos) {
        values.push_back(line.substr(last, next-last));   
        last = next + 1; 
    }
    values.push_back(line.substr(last));
    return values;
}

bool Reader::findMaterialKeyAndPopulateValue(const std::string& line, const std::string& key, int& value)
{
    std::string keyWithSpace = key + " ";
    std::string tempValue;
    if (line.find(keyWithSpace) != std::string::npos)
    {
        tempValue = line.substr(line.find(keyWithSpace) + keyWithSpace.length());
        if (tempValue.find_first_not_of(" ") != std::string::npos)
        {
            value = std::stoi(tempValue.substr(tempValue.find_first_not_of(" ")));
        }
        return true;
    }
    return false;       
}

bool Reader::findMaterialKeyAndPopulateValue(const std::string& line, const std::string& key, glm::vec3& value)
{
    std::string keyWithSpace = key + " ";
    std::string tempValue;
    if (line.find(keyWithSpace) != std::string::npos)
    {
        tempValue = line.substr(line.find(keyWithSpace) + keyWithSpace.length());
        if (tempValue.find_first_not_of(" ") != std::string::npos)
        {
            tempValue = tempValue.substr(tempValue.find_first_not_of(" "));
            std::vector<std::string> values = split(tempValue, " ");
            if (values.size() == 3) {
                value.x = std::stof(values[0]);
                value.y = std::stof(values[1]);
                value.z = std::stof(values[2]);
            }
            
        }
        return true;
    }
    return false;       
}

bool Reader::startsWithTab(const std::string& line)
{
    return line.find("\t") == 0;
}

void Reader::ReadOBJ(const std::string& fileName,
    std::string& texture_file,
    Eigen::MatrixXf& V,
    Eigen::MatrixXi& F,
    Eigen::MatrixXf& UV,
    Eigen::MatrixXi& UF,
    Eigen::MatrixXf& NV,
    Eigen::MatrixXi& NF,
    std::vector<Material>& materials)
{

    std::string line;
    static const std::string arr[] = { std::string("v "), std::string("vt "), std::string("vn "), std::string("f ") };
    std::vector<std::string> delimiters(arr, arr + sizeof(arr) / sizeof(arr[0]));
    std::vector<int> counts(delimiters.size() + 2, 0);
    std::vector<T_int> vecF, vecUF, vecNF;
    std::vector<Eigen::RowVector3f> vecV, vecUV, vecNV;

    // get file name
    std::string objFileName = MESH_DIR + fileName;
    std::ifstream file(objFileName, std::ios::in);
    if (file.is_open())
    {
        std::istringstream iss;
        std::string str;
        int max_count = 0;
        while (std::getline(file, line))
        {
            const size_t recordStart = line.find_first_not_of(" \t\r");
            if (recordStart == std::string::npos || line[recordStart] == '#') {
                continue;
            }

            const std::string record = line.substr(recordStart);
            const std::string materialLibraryPrefix = "mtllib ";
            if (record.compare(0, materialLibraryPrefix.length(), materialLibraryPrefix) == 0)
            {
                // get mtl file name
                std::string fileName = record.substr(materialLibraryPrefix.length());
                fileName = fileName.substr(0, fileName.find(".mtl") + strlen(".mtl"));
                std::string mtlFileName = MESH_DIR + fileName;

                std::ifstream material_file(mtlFileName);
                if (material_file.is_open())
                {
                    std::istringstream material_iss;
                    std::string material_str;
                    std::string material_line;

                    // material file 
                    Material material;
                    bool materialLoading = false;

                    while (std::getline(material_file, material_line))
                    {
                        if (materialLoading) {
                            findMaterialKeyAndPopulateValue(material_line, "\tKa", material.Ka);
                            findMaterialKeyAndPopulateValue(material_line, "\tKd", material.Kd);
                            findMaterialKeyAndPopulateValue(material_line, "\tKs", material.Ks);
                            findMaterialKeyAndPopulateValue(material_line, "\tNs", material.Ns);

                            // if a line starting without \t is observed, we are done loading..
                            if (!startsWithTab(material_line)) {
                                materialLoading = false;
                                materials.push_back(material);
                                std::cout << "ADDING TO MATERIALS" << std::endl;
                                std::cout << material << std::endl;
                            }
                        }

                        // Get material values
                        if (findMaterialKeyAndPopulateValue(material_line, "newmtl", material.name)) {
                            materialLoading = true;
                        }
                        
                        // find map_Kd
                        findMaterialKeyAndPopulateValue(texture_file, "map_Kd", material_line);
                    }

                    if (materialLoading) {
                        materialLoading = false;
                        materials.push_back(material);
                        std::cout << "ADDING TO MATERIALS" << std::endl;
                        std::cout << material << std::endl;
                    }
                } else {
                    std::cout << "ERROR: Could not open file: " << MESH_DIR + record.substr(materialLibraryPrefix.length()) << std::endl;
                    std::cout << "Error: " << strerror(errno) << std::endl;
                }
            }

            for (int i = 0; i < delimiters.size(); ++i)
            {
                if (record.compare(0, delimiters[i].length(), delimiters[i]) == 0)
                {
                    iss.str(std::string());
                    iss.clear();
                    iss.str(record.substr(delimiters[i].length()));
                    int count = 0;
                    Eigen::RowVector3f rowf3 = Eigen::RowVector3f::Zero();
                    while (iss >> str)
                    {
                        if (i < 3)
                        {
                            if (count < 3)
                            {
                                rowf3(count) = std::stof(str);
                            }
                        }
                        else
                        {
                            size_t pos = 0;
                            int icount = 0;
                            str += "/";

                            while ((pos = str.find("/")) != std::string::npos)
                            {
                                int val = std::stoi(str.substr(0, pos));
                                if (icount == 0)
                                {
                                    vecF.emplace_back(counts[3] + 1, count, val);
                                }
                                else if (icount == 1)
                                {
                                    vecUF.emplace_back(counts[4] + 1, count, val);
                                }
                                else
                                {
                                    vecNF.emplace_back(counts[5] + 1, count, val);
                                }
                                str.erase(0, pos + 1);
                                icount++;
                            }
                        }
                        count++;
                        if (count > max_count)
                        {
                            max_count = count;
                        }
                    }
                    if (i < 3)
                    {
                        if (i == 0)
                        {
                            vecV.push_back(rowf3);
                        }
                        else if (i == 1)
                        {
                            vecUV.push_back(rowf3);
                        }
                        else if (i == 2)
                        {
                            vecNV.push_back(rowf3);
                        }
                    }
                    else {
                        counts[3]++;
                        counts[4]++;
                        counts[5]++;
                    }
                }
            }

        }
        V.resize(vecV.size(), 3);
        for (size_t r = 0; r < vecV.size(); ++r) V.row(r) = vecV[r];
        UV.resize(vecUV.size(), 2);
        for (size_t r = 0; r < vecUV.size(); ++r) UV.row(r) = vecUV[r].head<2>();
        NV.resize(vecNV.size(), 3);
        for (size_t r = 0; r < vecNV.size(); ++r) NV.row(r) = vecNV[r];
        int rows = vecF.size();
        int cols = max_count;
        Eigen::SparseMatrix<int> sparseF, sparseUF, sparseNF;
        sparseF.resize(rows, cols);
        sparseUF.resize(rows, cols);
        sparseNF.resize(rows, cols);
        sparseF.setFromTriplets(vecF.begin(), vecF.end());
        sparseUF.setFromTriplets(vecUF.begin(), vecUF.end());
        sparseNF.setFromTriplets(vecNF.begin(), vecNF.end());
        F = Eigen::MatrixXi(sparseF);
        UF = Eigen::MatrixXi(sparseUF);
        NF = Eigen::MatrixXi(sparseNF);
    } else {
        std::cout << "ERROR: Could not open file: " << objFileName << std::endl;
        std::cout << "Error: " << strerror(errno) << std::endl;
    }
}

std::string Reader::MESH_DIR = std::string(ASSET_DIR) + "/models/";
