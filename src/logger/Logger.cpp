#include "logger/Logger.h"

Logger::Logger()
{
    
}

void Logger::LogMessage(std::string message)
{
    m_consoleBuffer.append(message);
    m_consoleBuffer.append("\n");
    m_scrollState++;
}

std::string Logger::Output()
{
    return m_consoleBuffer;
}

int Logger::GetState()
{
    return  m_scrollState;
}
