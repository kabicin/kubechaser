// Renderer smoke test against a real OpenGL 4.1 context.
//
// Creates a hidden GLFW window, generates and links every shader variant the
// ShaderFactory can produce, and draws entities into an offscreen framebuffer
// to check that pixels actually land where expected.
//
// Exits with code 77 (reported by CTest as "skipped") when no OpenGL 4.1
// context can be created, e.g. on a headless machine without a display. On
// Linux CI run it under xvfb-run with Mesa to get a software context, and set
// KUBECHASER_REQUIRE_GL=1 to turn a missing context into a failure instead.

#include "support/TestSupport.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "entity/Grid.h"
#include "entity/Mesh.h"
#include "entity/Quad.h"
#include "entity/Triangle.h"
#include "scene/Scene.h"
#include "scene/SceneDocument.h"
#include "scene/SceneManager.h"
#include "scene/SceneNode.h"
#include "shader/ShaderFactory.h"
#include "utility/Camera.h"

#include <array>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

using test::expect;

namespace {

constexpr int kSize = 64;

int skip(const char* reason)
{
    const char* require = std::getenv("KUBECHASER_REQUIRE_GL");
    if (require != nullptr && *require != '\0' && std::string(require) != "0") {
        std::cerr << "FAIL: " << reason << " (KUBECHASER_REQUIRE_GL is set)\n";
        return 1;
    }
    std::cerr << "SKIP: " << reason << "\n";
    return 77;
}

void expectNoGlError(const std::string& what)
{
    GLenum err;
    bool clean = true;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "GL error 0x" << std::hex << err << std::dec << " during " << what << "\n";
        clean = false;
    }
    expect(clean, "no GL errors: " + what);
}

class Framebuffer
{
public:
    Framebuffer()
    {
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glGenRenderbuffers(2, rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo[0]);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kSize, kSize);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo[0]);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo[1]);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, kSize, kSize);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo[1]);
        complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
        glViewport(0, 0, kSize, kSize);
    }
    ~Framebuffer()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteRenderbuffers(2, rbo);
        glDeleteFramebuffers(1, &fbo);
    }
    void Clear()
    {
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    std::array<unsigned char, 4> Pixel(int x, int y)
    {
        std::array<unsigned char, 4> rgba{};
        glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());
        return rgba;
    }
    bool complete = false;

private:
    GLuint fbo = 0;
    GLuint rbo[2] = {};
};

bool isBlack(const std::array<unsigned char, 4>& p)
{
    return p[0] == 0 && p[1] == 0 && p[2] == 0;
}

void solidColor(DrawAttributes& da, glm::vec3 color)
{
    da.color = true;
    da.useVertexColor = false;
    da.solidColor = color;
    da.lightSwitch = 0;
    da.lightBlinn = false;
    da.texture = false;
}

// Every combination of the attributes that select a shader variant must
// compile and link.
void testShaderVariants(const std::shared_ptr<ShaderFactory>& factory)
{
    for (unsigned int bits = 0; bits < (1u << 6); ++bits) {
        DrawAttributes da;
        da.tesselate = bits & 1;
        da.normal = bits & 2;
        da.texture = bits & 4;
        da.lightSwitch = (bits & 8) ? 0x7 : 0;
        da.color = bits & 16;
        da.instanced = bits & 32;
        factory->Use(da);

        const GLuint program = factory->GetShaderProgram();
        GLint linked = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linked);
        GLint current = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &current);
        expect(program != 0 && linked == GL_TRUE, "shader variant links: bits " + std::to_string(bits));
        expect(static_cast<GLuint>(current) == program, "shader variant in use: bits " + std::to_string(bits));
        expect(factory->IsTesselationEnabled() == da.tesselate,
               "tessellation flag tracks variant: bits " + std::to_string(bits));
    }
    expectNoGlError("shader variants");
}

void testDrawEntities(const std::shared_ptr<ShaderFactory>& factory)
{
    Framebuffer fb;
    expect(fb.complete, "offscreen framebuffer complete");
    glEnable(GL_DEPTH_TEST);

    auto camera = std::make_shared<Camera>(kSize, kSize);
    const int c = kSize / 2;

    // Triangle at the origin fills the centre of the view, not the corners.
    {
        Triangle triangle;
        solidColor(triangle.da, glm::vec3(1, 0, 0));
        fb.Clear();
        triangle.Draw(factory, camera);
        expectNoGlError("triangle draw");
        auto centre = fb.Pixel(c, c);
        expect(centre[0] > 200 && centre[1] < 50 && centre[2] < 50, "triangle draws red at centre");
        expect(isBlack(fb.Pixel(1, kSize - 2)), "triangle leaves top-left corner clear");

        // Moving it out of view leaves the centre clear.
        triangle.Translate(glm::vec3(100, 0, 0));
        fb.Clear();
        triangle.Draw(factory, camera);
        expect(isBlack(fb.Pixel(c, c)), "translated triangle moves out of view");

        triangle.da.wireframe = true;
        triangle.SetTranslate(glm::vec3(0));
        fb.Clear();
        triangle.Draw(factory, camera);
        expectNoGlError("wireframe triangle draw");
    }

    // Quad, plain and instanced.
    {
        Quad quad;
        solidColor(quad.da, glm::vec3(0, 1, 0));
        fb.Clear();
        quad.Draw(factory, camera);
        auto centre = fb.Pixel(c, c);
        expect(centre[1] > 200 && centre[0] < 50, "quad draws green at centre");

        fb.Clear();
        quad.DrawInstanced(factory, camera, {glm::vec3(-100, 0, 0), glm::vec3(0, 0, 0)});
        expectNoGlError("instanced quad draw");
        expect(!isBlack(fb.Pixel(c, c)), "instanced quad draws the on-screen instance");

        fb.Clear();
        quad.DrawInstanced(factory, camera, {glm::vec3(-100, 0, 0)});
        expect(isBlack(fb.Pixel(c, c)), "instanced quad honours offsets");

        // The instanced path must leave the plain path working.
        fb.Clear();
        quad.Draw(factory, camera);
        expect(!isBlack(fb.Pixel(c, c)), "plain draw works after instanced draw");
    }

    // Lit mesh from the test fixture (a unit right triangle in the XY plane).
    {
        Mesh mesh(test::fixtureObj(Reader::MESH_DIR, "commented_triangle.obj"));
        fb.Clear();
        mesh.Draw(factory, camera);
        expectNoGlError("lit mesh draw");
        // (0.25, 0.25, 0) is inside the triangle; project it to a pixel.
        glm::vec4 clip = camera->GetProjection() * camera->GetView() * glm::vec4(0.25f, 0.25f, 0, 1);
        int px = static_cast<int>((clip.x / clip.w * 0.5f + 0.5f) * kSize);
        int py = static_cast<int>((clip.y / clip.w * 0.5f + 0.5f) * kSize);
        expect(!isBlack(fb.Pixel(px, py)), "lit mesh draws inside its triangle");
        expect(isBlack(fb.Pixel(1, 1)), "lit mesh leaves bottom-left corner clear");
        Mesh::ClearCache();
    }

    // Grid with axes and instanced lines.
    {
        Grid grid(camera);
        fb.Clear();
        grid.Draw(factory, camera, true);
        grid.DrawLinesInstanced(factory, camera, {glm::vec3(0), glm::vec3(1, 0, 0)});
        expectNoGlError("grid draw");
    }

    // Scene graph and the tiled world renderer.
    {
        SceneDocument doc;
        auto quad = std::dynamic_pointer_cast<Quad>(doc.AddEntity("Quad"));
        solidColor(quad->da, glm::vec3(0, 0, 1));
        auto node = std::make_shared<SceneNode>();
        node->AttachSceneDocument(&doc);
        auto scene = std::make_shared<Scene>(node);

        fb.Clear();
        scene->Render(factory, camera);
        auto centre = fb.Pixel(c, c);
        expect(centre[2] > 200 && centre[0] < 50, "scene renders document entities");

        scene->SetShowGrid(true, camera);
        scene->Render(factory, camera);
        expectNoGlError("scene with grid");

        SceneManager manager(10.0f, 2);
        manager.AddScene({0, 0, 0}, scene);
        manager.SetShowGrid(true, camera);
        manager.UpdateActiveScenes(camera->GetCameraPos());
        fb.Clear();
        manager.RenderActive(factory, camera);
        expectNoGlError("scene manager render");
        expect(camera->cma.translate == glm::vec3(0), "RenderActive restores the camera model offset");
    }
}

} // namespace

int main()
{
    if (!glfwInit()) {
        return skip("glfwInit failed (no display?)");
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(kSize, kSize, "kubechaser renderer_test", nullptr, nullptr);
    if (window == nullptr) {
        glfwTerminate();
        return skip("could not create an OpenGL 4.1 core context");
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return skip("failed to load OpenGL functions");
    }
    std::cout << "OpenGL " << glGetString(GL_VERSION) << " on " << glGetString(GL_RENDERER) << "\n";

    {
        auto factory = std::make_shared<ShaderFactory>();
        expectNoGlError("ShaderFactory construction");
        testShaderVariants(factory);
        testDrawEntities(factory);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return test::finish();
}
