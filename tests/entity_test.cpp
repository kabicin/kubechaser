#include "support/TestSupport.h"

#include "entity/Entity.h"
#include "entity/Grid.h"
#include "entity/Mesh.h"
#include "entity/Quad.h"
#include "entity/Triangle.h"

#include <filesystem>
#include <fstream>
#include <memory>

using test::approx;
using test::expect;
using test::GlStub;

namespace {

// Minimal concrete Entity for exercising the base class.
class FakeEntity : public Entity
{
public:
    void Draw(const std::shared_ptr<ShaderFactory>&, const std::shared_ptr<Camera>&) override {}
    void SetObjectPicked() override {}
    bool Intersect(const std::shared_ptr<Camera>&, const Ray&, double&) override { return false; }
};

std::filesystem::path writeTestImage()
{
    // 2x2 binary PPM; stb_image reads PNM out of the box.
    const auto path = std::filesystem::temp_directory_path() / "kubechaser_entity_test.ppm";
    std::ofstream out(path, std::ios::binary);
    out << "P6\n2 2\n255\n";
    const unsigned char pixels[12] = {255, 0, 0, 0, 255, 0, 0, 0, 255, 255, 255, 255};
    out.write(reinterpret_cast<const char*>(pixels), sizeof(pixels));
    return path;
}

Ray rayDownZ(float x, float y)
{
    return Ray(glm::vec3(x, y, 5.0f), glm::vec3(0.0f, 0.0f, -1.0f));
}

void testIds()
{
    auto a = std::make_unique<FakeEntity>();
    auto b = std::make_unique<FakeEntity>();
    auto c = std::make_unique<FakeEntity>();
    expect(a->GetId() != b->GetId() && b->GetId() != c->GetId() && a->GetId() != c->GetId(),
           "live entities have distinct IDs");

    const int freed = b->GetId();
    b.reset();
    FakeEntity d;
    expect(d.GetId() == freed, "freed ID is reused by the next entity");
    FakeEntity e;
    expect(e.GetId() != a->GetId() && e.GetId() != c->GetId() && e.GetId() != d.GetId(),
           "next ID does not collide with live entities");
}

void testTransforms()
{
    FakeEntity entity;
    entity.Translate(glm::vec3(1, 2, 3));
    entity.Translate(glm::vec3(1, 0, 0));
    expect(entity.GetTranslate() == glm::vec3(2, 2, 3), "Translate accumulates");
    entity.SetTranslate(glm::vec3(0, 0, 1));
    expect(entity.GetTranslate() == glm::vec3(0, 0, 1), "SetTranslate replaces");

    entity.Scale(2.0);
    entity.Scale(glm::vec3(1, 3, 1));
    expect(entity.GetCameraAttributes().scale == glm::vec3(2, 6, 2), "Scale multiplies");
    entity.SetScale(glm::vec3(1, 1, 1));
    expect(entity.GetCameraAttributes().scale == glm::vec3(1, 1, 1), "SetScale replaces");

    entity.RotateX(30);
    entity.RotateXInc(15);
    entity.RotateY(10);
    entity.RotateYInc(-20);
    entity.RotateZInc(5);
    const CameraAttributes ca = entity.GetCameraAttributes();
    expect(ca.rotateXdeg == 45 && ca.rotateYdeg == -10 && ca.rotateZdeg == 5, "rotations set and increment");
}

void testTextures()
{
    GlStub& gl = GlStub::Get();
    const auto image = writeTestImage();
    {
        FakeEntity entity;
        expect(entity.GetTexture() == 0 && !entity.usingTexture, "starts untextured");

        expect(!entity.AddTexture("/nonexistent/kubechaser.png"), "missing image fails");
        expect(entity.GetTexture() == 0 && !entity.usingTexture, "failed load leaves entity untouched");

        expect(entity.AddTexture(image.string()), "loads image texture");
        const GLuint owned = entity.GetTexture();
        expect(owned != 0 && gl.liveTextures.count(owned) == 1, "owned texture created");
        expect(entity.GetTextureName() == "kubechaser_entity_test.ppm", "texture name is file name");
        expect(!entity.ReleaseBorrowedTexture(owned), "owned texture is not released as borrowed");

        // Borrowing an asset's texture frees the owned one.
        Asset asset;
        asset.TextureName = "asset.png";
        asset.TextureNum = 4242;
        const int deletedBefore = gl.deletedTextures;
        entity.SetTexture(asset);
        expect(gl.deletedTextures == deletedBefore + 1 && gl.liveTextures.count(owned) == 0,
               "SetTexture deletes the previously owned texture");
        expect(entity.GetTexture() == 4242 && entity.GetTextureName() == "asset.png", "borrows asset texture");

        entity.da.texture = true;
        expect(!entity.ReleaseBorrowedTexture(1), "other texture IDs are ignored");
        expect(entity.ReleaseBorrowedTexture(4242), "borrowed texture released");
        expect(entity.GetTexture() == 0 && !entity.usingTexture && !entity.da.texture
                   && entity.GetTextureName() == "N/A",
               "release resets texture state");

        expect(entity.AddTexture(image.string()), "reload owned texture");
    }
    expect(gl.liveTextures.empty(), "destructor deletes owned texture");
    std::filesystem::remove(image);
}

void testTriangleAndQuad()
{
    GlStub& gl = GlStub::Get();
    const std::size_t liveBefore = gl.LiveObjects();
    auto camera = std::make_shared<Camera>(800, 600);
    double t = 0.0;
    {
        Triangle triangle;
        expect(triangle.VAO != 0 && triangle.VBO != 0, "triangle uploads geometry");
        expect(triangle.Intersect(camera, rayDownZ(0, 0), t), "ray hits triangle");
        expect(approx(t, 5.0), "triangle hit distance");
        expect(!triangle.Intersect(camera, rayDownZ(0.45f, 0.45f), t), "ray misses outside triangle");

        Triangle moved(glm::vec3(10, 0, 0));
        expect(moved.Intersect(camera, rayDownZ(10, 0), t), "offset constructor translates");
        expect(!moved.Intersect(camera, rayDownZ(0, 0), t), "translated triangle left origin");

        const bool picked = triangle.usingTexture;
        triangle.SetObjectPicked();
        expect(triangle.usingTexture != picked, "picking a triangle toggles its texture");

        Quad quad;
        expect(quad.VAO != 0 && quad.VBO != 0 && quad.EBO != 0, "quad uploads geometry");
        expect(quad.Intersect(camera, rayDownZ(0.45f, 0.45f), t), "ray hits quad corner");
        expect(!quad.Intersect(camera, rayDownZ(0.6f, 0), t), "ray misses outside quad");
        quad.SetScale(glm::vec3(2, 2, 2));
        expect(quad.Intersect(camera, rayDownZ(0.9f, 0), t), "scaled quad is larger");
    }
    expect(gl.LiveObjects() == liveBefore, "triangle and quad release their GL objects");
}

void testMesh()
{
    GlStub& gl = GlStub::Get();
    const std::size_t liveBefore = gl.LiveObjects();
    auto camera = std::make_shared<Camera>(800, 600);
    const std::string fixture = test::fixtureObj(Reader::MESH_DIR, "commented_triangle.obj");
    {
        Mesh empty;
        expect(empty.GetVertices().empty(), "default mesh has no vertices");

        Mesh missing("no_such_model.obj");
        expect(missing.GetVertices().empty(), "missing OBJ yields an empty mesh");

        Mesh mesh(fixture);
        expect(mesh.GetSourceFile() == fixture, "mesh remembers its source");
        expect(mesh.GetVertices().size() == 3, "fixture has one triangle");
        expect(mesh.GetBoundsMin() == glm::vec3(0, 0, 0) && mesh.GetBoundsMax() == glm::vec3(1, 1, 0),
               "fixture bounds");
        expect(mesh.ndcRatio == 1.0f, "ndcRatio is the largest coordinate");
        expect(!mesh.da.texture, "mesh without texture is untextured");
        const NVertex& v = mesh.GetVertices()[0];
        expect(approx(v.Normal[2], 1.0), "flat triangle normal points +Z");

        double t = 0.0;
        expect(mesh.Intersect(camera, rayDownZ(0.25f, 0.25f), t), "ray hits mesh bounds");
        expect(!mesh.Intersect(camera, rayDownZ(2.0f, 2.0f), t), "ray misses mesh bounds");

        // Parsed OBJs are cached and shared between meshes of the same file.
        Mesh again(fixture);
        expect(again.GetVertices().data() == mesh.GetVertices().data(), "second mesh reuses cached parse");
        Mesh::ClearCache();
        Mesh fresh(fixture);
        expect(fresh.GetVertices().data() != mesh.GetVertices().data(), "ClearCache forces a re-read");
        expect(mesh.GetVertices().size() == 3, "existing mesh survives cache clear");

        // Every shipped model loads with geometry.
        for (const auto& entry : std::filesystem::directory_iterator(Reader::MESH_DIR)) {
            if (entry.path().extension() != ".obj") {
                continue;
            }
            Mesh model(entry.path().filename().string());
            expect(!model.GetVertices().empty() && model.GetVertices().size() % 3 == 0,
                   "asset model loads: " + entry.path().filename().string());
            const glm::vec3 lo = model.GetBoundsMin(), hi = model.GetBoundsMax();
            expect(lo.x <= hi.x && lo.y <= hi.y && lo.z <= hi.z,
                   "asset bounds ordered: " + entry.path().filename().string());
        }
        Mesh::ClearCache();
    }
    expect(gl.LiveObjects() == liveBefore, "meshes release their GL objects");
}

void testGrid()
{
    GlStub& gl = GlStub::Get();
    const std::size_t liveBefore = gl.LiveObjects();
    auto camera = std::make_shared<Camera>(800, 600);
    {
        Grid grid(camera);
        expect(gl.LiveObjects() > liveBefore, "grid uploads geometry");
        grid.SetScale(5.0f);
        grid.SetNeutralColor(glm::vec4(0.5f));
    }
    expect(gl.LiveObjects() == liveBefore, "grid releases its GL objects");
}

} // namespace

int main()
{
    GlStub::Install();

    testIds();
    testTransforms();
    testTextures();
    testTriangleAndQuad();
    testMesh();
    testGrid();

    return test::finish();
}
