#ifndef BASEFRAME_H
#define BASEFRAME_H
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "dearimgui.h"
#include "shader/Shader.h"
#include "logger/Logger.h"
#include "entity/Entity.h"
#include "Asset.h"
#include <memory>

class BaseFrame
{
private:
    int m_x, m_y, m_width, m_height;
    bool hidden = false;

public:
    static std::shared_ptr<Logger> logger;
    BaseFrame(int x, int y, int width, int height);
    virtual ~BaseFrame();
    virtual void Resize(int x, int y, int width, int height);
    
    virtual void Render() = 0;

    void DisplayLocation();
    void ClearBackground(float r, float g, float b, float a);
    static void AttachLogger(std::shared_ptr<Logger> logger);

    // getters
    int GetX() const { return m_x; }
    int GetY() const { return m_y; }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

    // setters
    void SetX(int newX) { m_x = newX; }
    void SetY(int newY) { m_y = newY; }
    void SetWidth(int newWidth) { m_width = newWidth; }
    void SetHeight(int newHeight) { m_height = newHeight; }

    // window visibility
    void Hide();
    void Show();
    bool Hidden();
};

#endif
