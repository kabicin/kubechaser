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
    glViewport(m_x, m_y, m_width, m_height);
    glEnable(GL_SCISSOR_TEST);
    glScissor(m_x, m_y, m_width, m_height);
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