#include "support/TestSupport.h"

#include "entity/Mesh.h"
#include "entity/Quad.h"
#include "entity/Triangle.h"
#include "scene/Scene.h"
#include "scene/SceneDocument.h"
#include "scene/SceneManager.h"
#include "scene/SceneNode.h"

#include <algorithm>
#include <memory>
#include <set>
#include <string>
#include <vector>

using test::approx;
using test::expect;
using test::GlStub;

namespace {

// Records draw calls into a shared log instead of touching GL.
class RecordingEntity : public Entity
{
public:
    RecordingEntity(std::vector<std::string>& log, std::string name) : log(log), name(std::move(name)) {}
    void Draw(const std::shared_ptr<ShaderFactory>&, const std::shared_ptr<Camera>&) override { log.push_back(name); }
    void SetObjectPicked() override {}
    bool Intersect(const std::shared_ptr<Camera>&, const Ray&, double&) override { return false; }

private:
    std::vector<std::string>& log;
    std::string name;
};

void testSceneDocumentAdd()
{
    SceneDocument doc;
    expect(doc.GetNumEntities() == 0 && doc.GetActiveEntity() == nullptr, "new document is empty");
    expect(doc.AddEntity(std::shared_ptr<Entity>()) == nullptr && doc.GetNumEntities() == 0,
           "null entity is rejected");

    auto triangle = doc.AddEntity("Triangle", glm::vec3(1, 2, 3));
    expect(std::dynamic_pointer_cast<Triangle>(triangle) != nullptr, "AddEntity(\"Triangle\") builds a Triangle");
    expect(triangle->GetTranslate() == glm::vec3(1, 2, 3), "offset is applied");
    expect(doc.GetActiveEntity() == triangle, "new entity becomes active");

    auto quad = doc.AddEntity("Quad");
    expect(std::dynamic_pointer_cast<Quad>(quad) != nullptr, "AddEntity(\"Quad\") builds a Quad");
    expect(doc.GetActiveEntity() == quad, "latest entity is active");

    expect(doc.AddEntity("Teapot") == nullptr, "unknown class name is rejected");
    expect(doc.AddEntity("Triangle", "file.obj") == nullptr, "only Mesh takes an object file");
    expect(doc.AddEntity("Quad", "file.obj", "tex.png") == nullptr, "only Mesh takes a texture file");

    const std::string obj = test::fixtureObj(Reader::MESH_DIR, "commented_triangle.obj");
    auto mesh = std::dynamic_pointer_cast<Mesh>(doc.AddEntity("Mesh", obj));
    expect(mesh != nullptr && mesh->GetVertices().size() == 3, "AddEntity(\"Mesh\", obj) loads the OBJ");
    expect(doc.GetNumEntities() == 3, "three entities added");
    expect(doc.GetEntities()[0] == triangle && doc.GetEntities()[1] == quad, "insertion order kept");
    Mesh::ClearCache();
}

void testSceneDocumentSelectionAndDelete()
{
    SceneDocument doc;
    std::vector<std::string> log;
    auto a = doc.AddEntity(std::make_shared<RecordingEntity>(log, "a"));
    auto b = doc.AddEntity(std::make_shared<RecordingEntity>(log, "b"));
    auto c = doc.AddEntity(std::make_shared<RecordingEntity>(log, "c"));

    doc.SetActiveEntity(0);
    expect(doc.GetActiveEntity() == a, "SetActiveEntity selects by index");
    doc.SetActiveEntity(3);
    expect(doc.GetActiveEntity() == nullptr, "out-of-range index clears selection");
    doc.SetActiveEntity(-1);
    expect(doc.GetActiveEntity() == nullptr, "negative index clears selection");

    doc.DeleteEntity(7);
    doc.DeleteEntity(-1);
    expect(doc.GetNumEntities() == 3, "out-of-range delete is ignored");

    doc.DeleteEntity(1);
    expect(doc.GetNumEntities() == 2 && doc.GetEntities()[0] == a && doc.GetEntities()[1] == c,
           "delete by index removes that entity");
    expect(doc.GetActiveEntity() == c, "last entity is active after delete");

    doc.DeleteEntity(b); // already removed
    expect(doc.GetNumEntities() == 2, "deleting an absent entity is ignored");
    doc.DeleteEntity(c);
    expect(doc.GetNumEntities() == 1 && doc.GetActiveEntity() == a, "delete by pointer");
    doc.DeleteEntity(a);
    expect(doc.GetNumEntities() == 0 && doc.GetActiveEntity() == nullptr, "deleting the last entity clears selection");

    doc.AddEntity(std::make_shared<RecordingEntity>(log, "d"));
    doc.Clear();
    expect(doc.GetNumEntities() == 0 && doc.GetActiveEntity() == nullptr, "Clear empties the document");
}

void testSceneDocumentDepth()
{
    SceneDocument doc;
    std::vector<std::string> log;
    expect(doc.GetMaxDepth() == 0.0 && doc.GetMinDepth() == 0.0, "empty document depth is zero");

    doc.AddEntity(std::make_shared<RecordingEntity>(log, "near"), glm::vec3(0, 0, 2));
    doc.AddEntity(std::make_shared<RecordingEntity>(log, "far"), glm::vec3(0, 0, -3));
    doc.AddEntity(std::make_shared<RecordingEntity>(log, "mid"), glm::vec3(0, 0, 0.5f));
    expect(approx(doc.GetMaxDepth(), 2.01), "max depth sits just above the front entity");
    expect(approx(doc.GetMinDepth(), -3.01), "min depth sits just below the back entity");
}

void testSceneNodeAndScene()
{
    std::vector<std::string> log;
    auto node = std::make_shared<SceneNode>();
    node->AddStaticObject(std::make_shared<RecordingEntity>(log, "static"));
    node->AddDynamicObject(std::make_shared<RecordingEntity>(log, "dynamic"));

    SceneDocument doc;
    doc.AddEntity(std::make_shared<RecordingEntity>(log, "doc1"));
    doc.AddEntity(std::make_shared<RecordingEntity>(log, "doc2"));
    expect(node->GetSceneDocument() == nullptr, "no document attached by default");
    node->AttachSceneDocument(&doc);
    expect(node->GetSceneDocument() == &doc, "document attached");

    node->Render(nullptr, nullptr);
    expect((log == std::vector<std::string>{"static", "dynamic", "doc1", "doc2"}),
           "node draws static, dynamic, then document entities in order");

    log.clear();
    Scene scene(node);
    scene.Render(nullptr, nullptr);
    expect(log.size() == 4, "scene renders its root node");

    // Entities deleted from the document stop being drawn.
    log.clear();
    doc.DeleteEntity(0);
    scene.Render(nullptr, nullptr);
    expect((log == std::vector<std::string>{"static", "dynamic", "doc2"}), "deleted entity is not drawn");

    log.clear();
    Scene empty;
    empty.Render(nullptr, nullptr); // no root: logs and returns
    expect(log.empty(), "scene without root draws nothing");
}

void testSceneManager()
{
    SceneManager manager(10.0f, 1);
    auto scene = std::make_shared<Scene>();
    manager.AddScene({1, 0, -2}, scene);
    expect(manager.GetScene({1, 0, -2}) == scene, "GetScene finds added scene");
    expect(manager.GetScene({0, 0, 0}) == nullptr, "GetScene misses unknown coord");
    auto replacement = std::make_shared<Scene>();
    manager.AddScene({1, 0, -2}, replacement);
    expect(manager.GetScene({1, 0, -2}) == replacement, "AddScene replaces existing scene");

    manager.UpdateActiveScenes(glm::vec3(15.0f, 42.0f, -5.0f));
    const auto& coords = manager.GetActiveCoords();
    expect(coords.size() == 9, "radius 1 activates a 3x3 block");
    std::set<std::pair<int, int>> xz;
    for (const auto& c : coords) {
        expect(c.y == 0, "active tiles are on the ground plane");
        xz.insert({c.x, c.z});
    }
    // Camera at x=15, z=-5 is in tile (1, -1) with 10-unit tiles.
    expect(xz.size() == 9 && xz.count({1, -1}) && xz.count({0, -2}) && xz.count({2, 0})
               && !xz.count({3, -1}),
           "active block is centred on the camera's tile");

    manager.SetActiveRadius(2);
    manager.UpdateActiveScenes(glm::vec3(0.0f));
    expect(manager.GetActiveCoords().size() == 25, "radius 2 activates a 5x5 block");

    manager.SetSceneSize(0.0f);
    manager.UpdateActiveScenes(glm::vec3(100.0f));
    expect(manager.GetActiveCoords().size() == 25, "non-positive scene size leaves active tiles unchanged");

    manager.RenderActive(nullptr, nullptr); // no camera: nothing to do

    // Hash/equality used by the scene map.
    SceneCoordHash hash;
    expect((SceneCoord{1, 2, 3} == SceneCoord{1, 2, 3}) && !(SceneCoord{1, 2, 3} == SceneCoord{3, 2, 1}),
           "SceneCoord equality");
    expect(hash({1, 2, 3}) == hash({1, 2, 3}), "SceneCoord hash is deterministic");
}

} // namespace

int main()
{
    GlStub::Install();
    const std::size_t liveBefore = GlStub::Get().LiveObjects();

    testSceneDocumentAdd();
    testSceneDocumentSelectionAndDelete();
    testSceneDocumentDepth();
    testSceneNodeAndScene();
    testSceneManager();

    expect(GlStub::Get().LiveObjects() == liveBefore, "scene objects release all GL objects");
    return test::finish();
}
