#include "utility/Reader.h"
#include <unordered_map>
#include <bitset>
#include <climits>
#include <sys/stat.h>
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <stdexcept>

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

int Reader::resolveIndex(int index, size_t definedCount)
{
    // OBJ indices are 1-based; negative indices count back from the most recently defined element
    const long long resolved = index < 0 ? static_cast<long long>(definedCount) + index + 1 : index;
    if (index == 0 || resolved < 1 || resolved > static_cast<long long>(definedCount))
    {
        throw std::out_of_range("index " + std::to_string(index) + " out of range");
    }
    return static_cast<int>(resolved);
}

void Reader::ReadMTL(const std::string& fileName, std::vector<Material>& materials)
{
    const std::string mtlFileName = MESH_DIR + fileName;
    std::ifstream materialFile(mtlFileName);
    if (!materialFile.is_open())
    {
        std::cout << "ERROR: Could not open file: " << mtlFileName << std::endl;
        std::cout << "Error: " << strerror(errno) << std::endl;
        return;
    }

    // texture paths in an MTL file are relative to the MTL file, but are loaded relative to MESH_DIR
    const size_t slash = fileName.find_last_of("/\\");
    const std::string mtlDir = slash == std::string::npos ? std::string() : fileName.substr(0, slash + 1);

    const size_t firstNew = materials.size();
    Material* material = nullptr;
    std::string line;
    int lineNumber = 0;
    while (std::getline(materialFile, line))
    {
        lineNumber++;
        // statements may be indented or not; the first token is the keyword
        std::istringstream lineStream(line);
        std::string keyword;
        if (!(lineStream >> keyword) || keyword[0] == '#')
        {
            continue;
        }

        if (keyword == "newmtl")
        {
            materials.emplace_back();
            material = &materials.back();
            lineStream >> material->name;
            continue;
        }
        if (material == nullptr)
        {
            continue;
        }

        try
        {
            std::vector<std::string> args;
            std::string arg;
            while (lineStream >> arg)
            {
                args.push_back(arg);
            }
            auto readColor = [&args](glm::vec3& color) {
                // "Kd r g b", or "Kd r" meaning r r r; spectral/xyz forms are ignored
                if (args.empty() || args[0] == "spectral" || args[0] == "xyz") return;
                const float r = std::stof(args[0]);
                color = args.size() >= 3 ? glm::vec3(r, std::stof(args[1]), std::stof(args[2])) : glm::vec3(r);
            };

            if (keyword == "Ka") readColor(material->Ka);
            else if (keyword == "Kd") readColor(material->Kd);
            else if (keyword == "Ks") readColor(material->Ks);
            else if (keyword == "Ns" && !args.empty()) material->Ns = std::stof(args[0]);
            else if (keyword == "map_Kd" && !args.empty())
            {
                // options such as "-s 1 1 1" precede the file name
                material->map_Kd = mtlDir + args.back();
            }
        }
        catch (const std::exception& error)
        {
            std::cout << "WARNING: skipping malformed line " << lineNumber << " in " << mtlFileName
                << ": " << error.what() << std::endl;
        }
    }

    for (size_t i = firstNew; i < materials.size(); ++i)
    {
        std::cout << "ADDING TO MATERIALS" << std::endl;
        std::cout << materials[i] << std::endl;
    }
}

int Reader::PrimaryMaterialIndex(const std::vector<Material>& materials)
{
    // the renderer applies one material per mesh: use the one covering the most faces,
    // falling back to the first material when the OBJ has no usemtl statements
    if (materials.empty())
    {
        return -1;
    }
    int best = 0;
    for (int i = 1; i < static_cast<int>(materials.size()); ++i)
    {
        if (materials[i].faceCount > materials[best].faceCount)
        {
            best = i;
        }
    }
    return best;
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
    std::string currentMaterial;
    std::unordered_map<std::string, int> materialFaceCounts;
    texture_file.clear();

    // get file name
    std::string objFileName = MESH_DIR + fileName;
    std::ifstream file(objFileName, std::ios::in);
    if (file.is_open())
    {
        std::istringstream iss;
        std::string str;
        int max_count = 0;
        int lineNumber = 0;
        while (std::getline(file, line))
        {
            lineNumber++;
            const size_t recordStart = line.find_first_not_of(" \t\r");
            if (recordStart == std::string::npos || line[recordStart] == '#') {
                continue;
            }

            const std::string record = line.substr(recordStart);
            std::istringstream recordStream(record);
            std::string keyword;
            recordStream >> keyword;
            if (keyword == "mtllib")
            {
                std::string libraryName;
                while (recordStream >> libraryName)
                {
                    ReadMTL(libraryName, materials);
                }
                continue;
            }
            if (keyword == "usemtl")
            {
                recordStream >> currentMaterial;
                continue;
            }

            try
            {
                for (int i = 0; i < delimiters.size(); ++i)
                {
                    if (record.compare(0, delimiters[i].length(), delimiters[i]) != 0)
                    {
                        continue;
                    }
                    iss.str(std::string());
                    iss.clear();
                    iss.str(record.substr(delimiters[i].length()));
                    int count = 0;
                    Eigen::RowVector3f rowf3 = Eigen::RowVector3f::Zero();
                    std::vector<T_int> lineF, lineUF, lineNF;
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
                            // v, v/vt, v//vn or v/vt/vn; an empty field means that index is absent
                            std::vector<std::string> fields = split(str, "/");
                            if (fields.empty() || fields.size() > 3 || fields[0].empty())
                            {
                                throw std::invalid_argument("bad face vertex '" + str + "'");
                            }
                            const size_t definedCounts[3] = { vecV.size(), vecUV.size(), vecNV.size() };
                            std::vector<T_int>* targets[3] = { &lineF, &lineUF, &lineNF };
                            for (size_t k = 0; k < fields.size(); ++k)
                            {
                                if (fields[k].empty())
                                {
                                    continue;
                                }
                                int val = resolveIndex(std::stoi(fields[k]), definedCounts[k]);
                                targets[k]->emplace_back(counts[3] + 1, count, val);
                            }
                        }
                        count++;
                    }

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
                    else
                    {
                        vecF.insert(vecF.end(), lineF.begin(), lineF.end());
                        vecUF.insert(vecUF.end(), lineUF.begin(), lineUF.end());
                        vecNF.insert(vecNF.end(), lineNF.begin(), lineNF.end());
                        max_count = std::max(max_count, count);
                        counts[3]++;
                        counts[4]++;
                        counts[5]++;
                        if (!currentMaterial.empty())
                        {
                            materialFaceCounts[currentMaterial]++;
                        }
                    }
                }
            }
            catch (const std::exception& error)
            {
                std::cout << "WARNING: skipping malformed line " << lineNumber << " in " << objFileName
                          << ": \"" << record << "\" (" << error.what() << ")" << std::endl;
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

        for (Material& material : materials)
        {
            auto used = materialFaceCounts.find(material.name);
            material.faceCount = used == materialFaceCounts.end() ? 0 : used->second;
            if (used != materialFaceCounts.end())
            {
                materialFaceCounts.erase(used);  // duplicate names: the first definition wins
            }
        }
        const int primary = PrimaryMaterialIndex(materials);
        if (primary >= 0)
        {
            texture_file = materials[primary].map_Kd;
        }
    } else {
        std::cout << "ERROR: Could not open file: " << objFileName << std::endl;
        std::cout << "Error: " << strerror(errno) << std::endl;
    }
}

std::string Reader::FileNameFromPath(const std::string& path)
{
    const size_t separator = path.find_last_of("/\\");
    if (separator == std::string::npos)
    {
        return path;
    }
    return path.substr(separator + 1);
}

std::vector<std::string> Reader::ListOBJFiles(const std::string& directory)
{
    namespace fs = std::filesystem;
    std::vector<std::string> files;
    std::error_code ec;
    fs::directory_iterator it(directory, ec);
    if (ec)
    {
        return files;
    }
    for (const fs::directory_entry& entry : it)
    {
        std::error_code entryError;
        if (!entry.is_regular_file(entryError))
        {
            continue;
        }
        std::string extension = entry.path().extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (extension == ".obj")
        {
            files.push_back(entry.path().filename().string());
        }
    }
    std::sort(files.begin(), files.end());
    return files;
}

std::string Reader::MESH_DIR = std::string(ASSET_DIR) + "/models/";
