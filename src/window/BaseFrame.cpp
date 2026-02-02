#include "window/BaseFrame.h"

BaseFrame::BaseFrame(int x, int y, int width, int height)
    : m_x(x), m_y(y), m_width(width), m_height(height)
{
}

BaseFrame::~BaseFrame()
{

}

void BaseFrame::Resize(int x, int y, int width, int height)
{
    m_x = x;
    m_y = y;
    m_width = width;
    m_height = height;
}

void BaseFrame::ClearBackground(float r, float g, float b, float a)
{
    ImGuiIO& io = ImGui::GetIO();
    int fbX = static_cast<int>(m_x * io.DisplayFramebufferScale.x);
    int fbY = static_cast<int>(m_y * io.DisplayFramebufferScale.y);
    int fbW = static_cast<int>(m_width * io.DisplayFramebufferScale.x);
    int fbH = static_cast<int>(m_height * io.DisplayFramebufferScale.y);
    glViewport(0, 0, fbW, fbH);
    glEnable(GL_SCISSOR_TEST);
    glScissor(fbX, fbY, fbW, fbH);
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);
}

void BaseFrame::DisplayLocation()
{
    std::cout << "x: " << m_x << " y: " << m_y << " w: " << m_width << " h: " << m_height << std::endl;
}

void BaseFrame::AttachLogger(std::shared_ptr<Logger> n_logger)
{
    logger = n_logger;
}

void BaseFrame::Hide()
{
    hidden = true;
}

void BaseFrame::Show()
{
    hidden = false;
}

bool BaseFrame::Hidden()
{
    return hidden;
}

std::shared_ptr<Logger> BaseFrame::logger;
