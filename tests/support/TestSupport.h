#ifndef KUBECHASER_TEST_SUPPORT_H
#define KUBECHASER_TEST_SUPPORT_H

// Shared helpers for the app-level tests (entities, scenes, camera).
//
// GlStub replaces glad's function pointers with no-op fakes so code that
// creates or destroys GPU objects can run without an OpenGL context. It hands
// out fresh object names and counts how many were created and deleted, which
// lets tests check that entities release exactly what they allocate. Any GL
// function not stubbed here stays null and crashes loudly if called.

#include <glad/glad.h>

#include <cmath>
#include <filesystem>
#include <iostream>
#include <set>
#include <string>

namespace test {

inline int failures = 0;

inline void expect(bool ok, const std::string& what)
{
    if (!ok) {
        std::cerr << "FAIL: " << what << "\n";
        ++failures;
    }
}

inline bool approx(double a, double b, double eps = 1e-4)
{
    return std::abs(a - b) <= eps;
}

// Mesh resolves OBJ paths relative to Reader::MESH_DIR (assets/models), so
// fixtures are passed as a path relative to that directory.
inline std::string fixtureObj(const std::string& meshDir, const std::string& name)
{
    const std::filesystem::path fixture = std::filesystem::path(KUBECHASER_TEST_FIXTURE_DIR) / name;
    return std::filesystem::relative(fixture, meshDir).generic_string();
}

inline int finish()
{
    if (failures != 0) {
        std::cerr << failures << " check(s) failed\n";
        return 1;
    }
    return 0;
}

struct GlStub
{
    GLuint nextName = 1;
    std::set<GLuint> liveBuffers, liveVertexArrays, liveTextures;
    int deletedBuffers = 0, deletedVertexArrays = 0, deletedTextures = 0;
    int clearColorCalls = 0;

    static GlStub& Get()
    {
        static GlStub stub;
        return stub;
    }

    std::size_t LiveObjects() const
    {
        return liveBuffers.size() + liveVertexArrays.size() + liveTextures.size();
    }

    static void gen(GLsizei n, GLuint* names, std::set<GLuint>& live)
    {
        for (GLsizei i = 0; i < n; ++i) {
            names[i] = Get().nextName++;
            live.insert(names[i]);
        }
    }

    // Like GL, deleting name 0 or an unknown name is silently ignored.
    static void del(GLsizei n, const GLuint* names, std::set<GLuint>& live, int& counter)
    {
        for (GLsizei i = 0; i < n; ++i) {
            if (live.erase(names[i]) != 0) {
                ++counter;
            }
        }
    }

    static void APIENTRY GenBuffers(GLsizei n, GLuint* b) { gen(n, b, Get().liveBuffers); }
    static void APIENTRY GenVertexArrays(GLsizei n, GLuint* a) { gen(n, a, Get().liveVertexArrays); }
    static void APIENTRY GenTextures(GLsizei n, GLuint* t) { gen(n, t, Get().liveTextures); }
    static void APIENTRY DeleteBuffers(GLsizei n, const GLuint* b) { del(n, b, Get().liveBuffers, Get().deletedBuffers); }
    static void APIENTRY DeleteVertexArrays(GLsizei n, const GLuint* a) { del(n, a, Get().liveVertexArrays, Get().deletedVertexArrays); }
    static void APIENTRY DeleteTextures(GLsizei n, const GLuint* t) { del(n, t, Get().liveTextures, Get().deletedTextures); }
    static void APIENTRY ClearColor(GLfloat, GLfloat, GLfloat, GLfloat) { ++Get().clearColorCalls; }

    static void APIENTRY BindBuffer(GLenum, GLuint) {}
    static void APIENTRY BindVertexArray(GLuint) {}
    static void APIENTRY BindTexture(GLenum, GLuint) {}
    static void APIENTRY BufferData(GLenum, GLsizeiptr, const void*, GLenum) {}
    static void APIENTRY VertexAttribPointer(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*) {}
    static void APIENTRY EnableVertexAttribArray(GLuint) {}
    static void APIENTRY DisableVertexAttribArray(GLuint) {}
    static void APIENTRY TexImage2D(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*) {}
    static void APIENTRY GenerateMipmap(GLenum) {}
    static void APIENTRY TexParameteri(GLenum, GLenum, GLint) {}

    static void Install()
    {
        glad_glGenBuffers = &GenBuffers;
        glad_glGenVertexArrays = &GenVertexArrays;
        glad_glGenTextures = &GenTextures;
        glad_glDeleteBuffers = &DeleteBuffers;
        glad_glDeleteVertexArrays = &DeleteVertexArrays;
        glad_glDeleteTextures = &DeleteTextures;
        glad_glClearColor = &ClearColor;
        glad_glBindBuffer = &BindBuffer;
        glad_glBindVertexArray = &BindVertexArray;
        glad_glBindTexture = &BindTexture;
        glad_glBufferData = &BufferData;
        glad_glVertexAttribPointer = &VertexAttribPointer;
        glad_glEnableVertexAttribArray = &EnableVertexAttribArray;
        glad_glDisableVertexAttribArray = &DisableVertexAttribArray;
        glad_glTexImage2D = &TexImage2D;
        glad_glGenerateMipmap = &GenerateMipmap;
        glad_glTexParameteri = &TexParameteri;
    }
};

} // namespace test

#endif
