#include "entity/Mesh.h"
#include <algorithm>
#include <memory>

namespace
{
// Everything a Mesh needs from its OBJ file that does not depend on the GL
// context or on per-instance state. Built once per file and shared, so
// creating another Mesh for the same OBJ only has to upload the vertices.
struct ParsedMesh
{
    std::vector<NVertex> vertices;
    glm::vec3 lo = glm::vec3(0.0f);
    glm::vec3 hi = glm::vec3(0.0f);
    float ndcRatio = 0.0f; // 0 when the mesh has no non-zero coordinate
    bool hasMaterial = false;
    Material material;
    std::string textureFile;
};

using ParsedMeshCache = std::unordered_map<std::string, std::shared_ptr<const ParsedMesh>>;

ParsedMeshCache& parsedMeshCache()
{
    static ParsedMeshCache cache;
    return cache;
}

void addToMap(std::unordered_map<int, std::unordered_set<int>>& map, int key, int val)
{
    map[key].insert(val);
}

std::vector<NVertex> buildVertices(
    const Eigen::MatrixXf& V,
    const Eigen::MatrixXi& F,
    const Eigen::MatrixXf& UV,
    const Eigen::MatrixXi& UF)
{
    int numTriangles = 0;
    for (int i = 0; i < F.rows(); ++i)
    {
        int j = 0;
        while (j < F.cols() - 2 && F(i, j + 2) != 0)
        {
            numTriangles++;
            j++;
        }
    }

    Eigen::MatrixXi face_fan;
    face_fan.resize(numTriangles, 6);
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

    std::vector<NVertex> vertices(face_fan.rows() * 3);
    for (int i = 0; i < face_fan.rows(); i++)
    {
        for (int j = 0; j < 3; j++)
        {
            int vertex = face_fan.row(i)[j];
            Eigen::RowVector3f pos = V.row(vertex);
            // faces without texture coordinates (e.g. v//vn) have uv index -1
            int uvIndex = face_fan.row(i)[j+3];
            Eigen::RowVector2f uvpos = (uvIndex >= 0 && uvIndex < UV.rows())
                ? Eigen::RowVector2f(UV.row(uvIndex))
                : Eigen::RowVector2f(0.0f, 0.0f);
            
            Eigen::RowVector3f normal(0, 0, 0);
            auto adjacent = adjacentFaces.find(vertex);
            if (adjacent != adjacentFaces.end())
            {
                for (const auto& k : adjacent->second)
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
    return vertices;
}

std::shared_ptr<const ParsedMesh> parseOBJ(const std::string& fileNameOBJ)
{
    Eigen::MatrixXf V;
    Eigen::MatrixXi F;
    Eigen::MatrixXf UV;
    Eigen::MatrixXi UF;
    Eigen::MatrixXf NV;
    Eigen::MatrixXi NF;
    std::vector<Material> materials;
    auto parsed = std::make_shared<ParsedMesh>();
    Reader::ReadOBJ(fileNameOBJ, parsed->textureFile, V, F, UV, UF, NV, NF, materials);
    std::cout << "READ OBJ file " << fileNameOBJ << " with materials: " << materials.size()
        << " and texture: " << parsed->textureFile << std::endl;

    const int primaryMaterial = Reader::PrimaryMaterialIndex(materials);
    if (primaryMaterial >= 0) {
        parsed->hasMaterial = true;
        parsed->material = materials[primaryMaterial];
        const auto usedMaterials = std::count_if(materials.begin(), materials.end(),
            [](const Material& m) { return m.faceCount > 0; });
        if (usedMaterials > 1) {
            std::cout << "WARNING: " << fileNameOBJ << " uses " << usedMaterials
                << " materials; only " << parsed->material.name << " is applied" << std::endl;
        }
    }

    parsed->vertices = buildVertices(V, F, UV, UF);

    // Axis-aligned bounding box in model space: the per-axis min and max over
    // every vertex. ndcRatio is the largest absolute vertex coordinate on any
    // axis, used to scale screen-space drag offsets into model units.
    if (V.rows() > 0 && V.cols() >= 3)
    {
        const Eigen::RowVector3f lo = V.leftCols<3>().colwise().minCoeff();
        const Eigen::RowVector3f hi = V.leftCols<3>().colwise().maxCoeff();
        parsed->lo = glm::vec3(lo(0), lo(1), lo(2));
        parsed->hi = glm::vec3(hi(0), hi(1), hi(2));
        parsed->ndcRatio = V.leftCols<3>().cwiseAbs().maxCoeff();
    }
    return parsed;
}

// Returns the parsed contents of fileNameOBJ, reading the file only the first
// time it is requested. Files that yield no geometry (missing or unreadable)
// are not cached so a later request retries them.
std::shared_ptr<const ParsedMesh> loadOBJ(const std::string& fileNameOBJ)
{
    ParsedMeshCache& cache = parsedMeshCache();
    auto it = cache.find(fileNameOBJ);
    if (it != cache.end()) return it->second;

    std::shared_ptr<const ParsedMesh> parsed = parseOBJ(fileNameOBJ);
    if (!parsed->vertices.empty()) cache.emplace(fileNameOBJ, parsed);
    return parsed;
}
}

Mesh::Mesh() {}

Mesh::~Mesh()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

Mesh::Mesh(const std::string& fileNameOBJ, const std::string& /*fileNameMTL*/, const std::string& textureName)
    : sourceFile(fileNameOBJ)
{
    std::shared_ptr<const ParsedMesh> parsed = loadOBJ(fileNameOBJ);
    // Aliasing constructor: keeps the parsed mesh alive (even if the cache is
    // cleared) while exposing only its vertices.
    vertices = std::shared_ptr<const std::vector<NVertex>>(parsed, &parsed->vertices);
    boundsLo = parsed->lo;
    boundsHi = parsed->hi;

    if (parsed->hasMaterial) {
        da.materialAttributes.LoadFromMaterial(parsed->material);
    } else {
        da.materialAttributes.diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
    }
    if (parsed->ndcRatio > 0.0f) this->ndcRatio = parsed->ndcRatio;

    upload(parsed->vertices);
    setTexture(parsed->textureFile.empty() ? textureName : parsed->textureFile);
    setBoundingBox(parsed->lo, parsed->hi);
}

const std::vector<NVertex>& Mesh::GetVertices() const
{
    static const std::vector<NVertex> empty;
    return vertices ? *vertices : empty;
}

void Mesh::ClearCache()
{
    parsedMeshCache().clear();
}

void Mesh::setBoundingBox(const glm::vec3& lo, const glm::vec3& hi)
{
    // Its 8 corners are transformed into world space in Intersect.
    v1 = glm::vec4(hi.x, hi.y, lo.z, 1.0f);
    v2 = glm::vec4(lo.x, hi.y, lo.z, 1.0f);
    v3 = glm::vec4(lo.x, hi.y, hi.z, 1.0f);
    v4 = glm::vec4(hi.x, hi.y, hi.z, 1.0f);
    v5 = glm::vec4(hi.x, lo.y, lo.z, 1.0f);
    v6 = glm::vec4(lo.x, lo.y, lo.z, 1.0f);
    v7 = glm::vec4(lo.x, lo.y, hi.z, 1.0f);
    v8 = glm::vec4(hi.x, lo.y, hi.z, 1.0f);
}

void Mesh::upload(const std::vector<NVertex>& vertices)
{
    // create VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    num_vertices = static_cast<int>(vertices.size());

    // create VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(NVertex) * num_vertices, vertices.data(), GL_STATIC_DRAW);

    // pos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(NVertex), (void*)0);
    glEnableVertexAttribArray(0);

    // normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(NVertex), reinterpret_cast<void*>(offsetof(NVertex, Normal)));
    glEnableVertexAttribArray(1);

    // uv
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(NVertex), reinterpret_cast<void*>(offsetof(NVertex, TexturePos)));
    glEnableVertexAttribArray(2);
}

void Mesh::setTexture(const std::string& textureFile)
{
    if (textureFile != "")
    {
        std::cout << "Setting texture file: "  << textureFile << std::endl;
        this->da.texture = AddTexture(Reader::MESH_DIR + textureFile);
    }
    else
    {
        this->da.texture = false;
    }
}

void Mesh::Draw(const std::shared_ptr<ShaderFactory>& factory, const std::shared_ptr<Camera>& camera)
{
    factory->Use(da, camera->GetCameraPos());
    camera->Render(factory->GetUniformLocations(), GetCameraAttributes());

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
