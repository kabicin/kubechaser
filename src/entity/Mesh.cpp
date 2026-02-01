#include "entity/Mesh.h"

Mesh::Mesh() {}

Mesh::Mesh(const std::string& fileNameOBJ, const std::string& fileNameMTL, const std::string& textureName)
{
    Eigen::MatrixXf V;
    Eigen::MatrixXi F;
    Eigen::MatrixXf UV;
    Eigen::MatrixXi UF;
    Eigen::MatrixXf NV;
    Eigen::MatrixXi NF;
    initialize(V, F, UV, UF, NV, NF, fileNameOBJ, fileNameMTL, textureName);
}

void Mesh::initialize(
    Eigen::MatrixXf& V, 
    Eigen::MatrixXi& F,
    Eigen::MatrixXf& UV, 
    Eigen::MatrixXi& UF, 
    Eigen::MatrixXf& NV, 
    Eigen::MatrixXi& NF, 
    const std::string& fileNameOBJ, 
    const std::string& fileNameMTL, 
    const std::string& textureName)
{
    std::string textureFile = std::string();
    std::vector<Material> materials;
    Reader::ReadOBJ(fileNameOBJ, textureFile, V, F, UV, UF, NV, NF, materials);
    std::cout << "READ OBJ file with materials: " << materials.size() << " and texture: " << textureFile << " and name: " << textureName << std::endl;
    if (materials.size() > 0) {
        da.materialAttributes.LoadFromMaterial(materials[0]);
        std::cout << "Loading " << materials[0].name << " into mesh " << fileNameOBJ << std::endl;
    } else {
        da.materialAttributes.diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
    }
    generate(V, F, UV, UF, NV, NF, textureFile == "" ? textureName : textureFile);
    generateBoundingBox(V);
}

void Mesh::generateBoundingBox(const Eigen::MatrixXf& V)
{
    Eigen::RowVector3f eq1(0,0,0);
    Eigen::RowVector3f eq2(0,0,0);
    Eigen::RowVector3f eq3(0,0,0);
    Eigen::RowVector3f eq4(0,0,0);
    Eigen::RowVector3f eq5(0,0,0);
    Eigen::RowVector3f eq6(0,0,0);
    Eigen::RowVector3f eq7(0,0,0);
    Eigen::RowVector3f eq8(0,0,0);
    for (int i = 0; i < V.rows(); i++)
    {
        Eigen::RowVector3f row = V.row(i);
        if (row(0) >= 0 && row(1) >= 0 && row(2) < 0 && row.norm() > eq1.norm()) eq1 = row;
        if (row(0) < 0 && row(1) >= 0 && row(2) < 0 && row.norm() > eq2.norm()) eq2 = row;
        if (row(0) < 0 && row(1) >= 0 && row(2) >= 0 && row.norm() > eq3.norm()) eq3 = row;
        if (row(0) >= 0 && row(1) >= 0 && row(2) >= 0 && row.norm() > eq4.norm()) eq4 = row;
        if (row(0) >= 0 && row(1) < 0 && row(2) < 0 && row.norm() > eq5.norm()) eq5 = row;
        if (row(0) < 0 && row(1) < 0 && row(2) < 0 && row.norm() > eq6.norm()) eq6 = row;
        if (row(0) < 0 && row(1) < 0 && row(2) >= 0 && row.norm() > eq7.norm()) eq7 = row;
        if (row(0) >= 0 && row(1) < 0 && row(2) >= 0 && row.norm() > eq8.norm()) eq8 = row;
    }
    v1 = glm::vec4(eq1(0), eq1(1), eq1(2), 1.0f);
    v2 = glm::vec4(eq2(0), eq2(1), eq2(2), 1.0f);
    v3 = glm::vec4(eq3(0), eq3(1), eq3(2), 1.0f);
    v4 = glm::vec4(eq4(0), eq4(1), eq4(2), 1.0f);
    v5 = glm::vec4(eq5(0), eq5(1), eq5(2), 1.0f);
    v6 = glm::vec4(eq6(0), eq6(1), eq6(2), 1.0f);
    v7 = glm::vec4(eq7(0), eq7(1), eq7(2), 1.0f);
    v8 = glm::vec4(eq8(0), eq8(1), eq8(2), 1.0f);
}

void Mesh::addToMap(std::unordered_map<int, std::unordered_set<int>>& map, const int& key, const int& val)
{
    if (map.find(key) == map.end()) map[key] = std::unordered_set<int>();
    map[key].insert(val);
}

void Mesh::generate(
    const Eigen::MatrixXf& V, 
    const Eigen::MatrixXi& F,
    const Eigen::MatrixXf& UV, 
    const Eigen::MatrixXi& UF, 
    const Eigen::MatrixXf& NV, 
    const Eigen::MatrixXi& NF,
    const std::string& textureFile)
{
    // create VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    Eigen::MatrixXi face_fan;
    face_fan.resize(20000, 6);
    std::unordered_map<int, std::unordered_set<int>> adjacentFaces;
    int index = 0;
    for (int i = 0; i < F.rows(); ++i)
    {
        Eigen::RowVectorXi row = F.row(i);
        Eigen::RowVectorXi uvrow = UF.row(i);
        int j = 0;
        while (j < F.cols() - 2 && row(j + 2) != 0)
        {
            face_fan.row(index)[0] = row(0) - 1;
            face_fan.row(index)[1] = row(j + 1) - 1;
            face_fan.row(index)[2] = row(j + 2) - 1;
            addToMap(adjacentFaces, row(0) - 1, index);
            addToMap(adjacentFaces, row(j + 1) - 1, index);
            addToMap(adjacentFaces, row(j + 2) - 1, index);
            face_fan.row(index)[3] = uvrow(0) - 1;
            face_fan.row(index)[4] = uvrow(j + 1) - 1;
            face_fan.row(index)[5] = uvrow(j + 2) - 1;
            index++;
            j++;
        }
    }
    face_fan.conservativeResize(index, 6);
    float max_x = 0, max_y = 0, max_z = 0;
    float min_x = 1000, min_y = 1000, min_z = 1000;

    for (int i = 0; i < V.rows(); i++)
    {
        for (int j = 0; j < 3; j++)
        {
            Eigen::RowVector3f pos = V.row(i);
            if (pos(0) > max_x)
            {
                max_x = pos(0);
            }
            if (pos(1) > max_y)
            {
                max_y = pos(1);
            }
            if (pos(2) > max_z)
            {
                max_z = pos(2);
            }
            if (pos(0) < min_x)
            {
                min_x = pos(0);
            }
            if (pos(1) < min_y)
            {
                min_y = pos(1);
            }
            if (pos(2) < min_z)
            {
                min_z = pos(2);
            }
        }    
    }
    float cmin_x = std::min(std::abs(max_x), std::abs(min_x));
    float cmin_y = std::min(std::abs(max_y), std::abs(min_y));
    float cmin_z = std::min(std::abs(max_z), std::abs(min_z));
    float cmax_x = std::max(std::abs(max_x), std::abs(min_x));
    float cmax_y = std::max(std::abs(max_y), std::abs(min_y));
    float cmax_z = std::max(std::abs(max_z), std::abs(min_z));
    this->ndcRatio = std::max(cmax_x, std::max(cmax_y, cmax_z));
    float diff_x = cmax_x - cmin_x;
    float diff_y = cmax_y - cmin_y;
    float diff_z = cmax_z - cmin_z;
    float mid_x =  diff_x / 2.0;
    float mid_y = diff_y / 2.0;
    float mid_z = diff_z / 2.0;

    NVertex vertices[10000];
    num_vertices = face_fan.rows() * 3;
    int vertexIndex = 0;
    for (int i = 0; i < face_fan.rows(); i++)
    {
        for (int j = 0; j < 3; j++)
        {
            int vertex = face_fan.row(i)[j];
            Eigen::RowVector3f pos = V.row(vertex);
            Eigen::RowVector2f uvpos = UV.row(face_fan.row(i)[j+3]);
            
            Eigen::RowVector3f normal(0, 0, 0);
            if (adjacentFaces.find(vertex) != adjacentFaces.end())
            {
                for (const auto& k : adjacentFaces[vertex])
                {
                    Eigen::RowVectorXi trifan = face_fan.row(k);
                    Eigen::RowVector3f v0, v1, v2;
                    if (trifan(0) == vertex)
                    {
                        v0 = V.row(trifan(0));
                        v1 = V.row(trifan(1));
                        v2 = V.row(trifan(2));
                    }
                    else if (trifan(1) == vertex)
                    {
                        v0 = V.row(trifan(1));
                        v1 = V.row(trifan(2));
                        v2 = V.row(trifan(0));
                    }
                    else
                    {
                        v0 = V.row(trifan(2));
                        v1 = V.row(trifan(0));
                        v2 = V.row(trifan(1));
                    }
                    normal += (v1 - v0).cross(v2 - v0);
                }
            }
            if (normal.norm() > 0.0f)
            {
                normal.normalize();
            }
            vertices[3 * i + j] = NVertex(pos(0), pos(1), pos(2), normal(0), normal(1), normal(2), uvpos(0), uvpos(1));
        }
    }
    
    // create VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(NVertex) * num_vertices, vertices, GL_STATIC_DRAW);

    // pos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(NVertex), (void*)0);
    glEnableVertexAttribArray(0);

    // normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(NVertex), reinterpret_cast<void*>(offsetof(NVertex, Normal)));
    glEnableVertexAttribArray(1);

    // uv
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(NVertex), reinterpret_cast<void*>(offsetof(NVertex, TexturePos)));
    glEnableVertexAttribArray(2);

    // texture
    if (textureFile != "")
    {
        std::cout << "Setting texture file: "  << textureFile << std::endl;
        AddTexture(Reader::MESH_DIR + textureFile);
        this->da.texture = true;
    }
    else
    {
        this->da.texture = false;
    }
}

void Mesh::Draw(const std::shared_ptr<ShaderFactory>& factory, const std::shared_ptr<Camera>& camera)
{
    factory->Use(da, camera->GetCameraPos());
    camera->Render(factory->GetShaderProgram(), GetCameraAttributes());

    glBindTexture(GL_TEXTURE_2D, TEX);
    glBindVertexArray(VAO);

    if (da.wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    if (da.tesselate)
    {
        glPatchParameteri(GL_PATCH_VERTICES, 3);
        glDrawArrays(GL_PATCHES, 0, num_vertices);
    }
    else
    {
        glDrawArrays(GL_TRIANGLES, 0, num_vertices);
    }

    if (da.wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Mesh::SetObjectPicked()
{
    // do nothing
}

bool Mesh::Intersect(const std::shared_ptr<Camera>& camera, const Ray& ray, double& t)
{
    glm::mat4 model = camera->GenerateModel(GetCameraAttributes());
    glm::vec4 u1, u2, u3, u4, u5, u6, u7, u8;
    u1 = model * v1;
    u2 = model * v2;
    u3 = model * v3;
    u4 = model * v4;
    u5 = model * v5;
    u6 = model * v6;
    u7 = model * v7;
    u8 = model * v8;

    float minx, miny, minz, maxx, maxy, maxz;
    solveMinMax(u1, u2, u3, u4, u5, u6, u7, u8, minx, maxx, miny, maxy, minz, maxz);

    // bounding box intersection
    bool intersect = rayIntersectBox(ray, t, minx, maxx, miny, maxy, minz, maxz);
    if (intersect) std::cout << "Mesh Intersection t=" << t << std::endl;
    return intersect;
}

void Mesh::solveMinMax(glm::vec4& u0, glm::vec4& u1, glm::vec4& u2, glm::vec4& u3, glm::vec4& u4,
    glm::vec4& u5, glm::vec4& u6, glm::vec4& u7, 
    float& minx, float& maxx, float& miny, 
    float& maxy, float& minz, float& maxz)
{
    minx = std::min(u0.x, std::min(u1.x, std::min(u2.x, std::min(u3.x, std::min(u4.x, std::min(u5.x, std::min(u6.x, u7.x)))))));
    maxx = std::max(u0.x, std::max(u1.x, std::max(u2.x, std::max(u3.x, std::max(u4.x, std::max(u5.x, std::max(u6.x, u7.x)))))));
    miny = std::min(u0.y, std::min(u1.y, std::min(u2.y, std::min(u3.y, std::min(u4.y, std::min(u5.y, std::min(u6.y, u7.y)))))));
    maxy = std::max(u0.y, std::max(u1.y, std::max(u2.y, std::max(u3.y, std::max(u4.y, std::max(u5.y, std::max(u6.y, u7.y)))))));
    minz = std::min(u0.z, std::min(u1.z, std::min(u2.z, std::min(u3.z, std::min(u4.z, std::min(u5.z, std::min(u6.z, u7.z)))))));
    maxz = std::max(u0.z, std::max(u1.z, std::max(u2.z, std::max(u3.z, std::max(u4.z, std::max(u5.z, std::max(u6.z, u7.z)))))));
}

bool Mesh::rayIntersectBox(const Ray& ray, double& t, float minx, float maxx, float miny, float maxy, float minz, float maxz)
{
    double x1 = (minx - ray.eye(0)) / ray.direction(0);  
    double x2 = (maxx - ray.eye(0)) / ray.direction(0);  
    double y1 = (miny - ray.eye(1)) / ray.direction(1);  
    double y2 = (maxy - ray.eye(1)) / ray.direction(1);  
    double z1 = (minz - ray.eye(2)) / ray.direction(2);  
    double z2 = (maxz - ray.eye(2)) / ray.direction(2);  
    double txmin = std::min(x1, x2);  
    double txmax = std::max(x1, x2);  
    double tymin = std::min(y1, y2);  
    double tymax = std::max(y1, y2);  
    double tzmin = std::min(z1, z2);  
    double tzmax = std::max(z1, z2);  
    double maxofmin = std::max(std::max(txmin, tymin), tzmin);  
    double minofmax = std::min(std::min(txmax, tymax), tzmax);  
    double min_t = 0;
    double max_t = 1000;
    t = std::min(maxofmin, minofmax);
    return maxofmin < minofmax && std::max(min_t, maxofmin) <= std::min(max_t, minofmax);  
}
