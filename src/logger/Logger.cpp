#include "logger/Logger.h"

Logger::Logger(std::size_t maxLines)
    : m_maxLines(maxLines == 0 ? 1 : maxLines)
{
}

void Logger::LogMessage(const std::string& message)
{
    m_consoleBuffer.append(message);
    m_consoleBuffer.append("\n");
    m_lineLengths.push_back(message.size() + 1);
    m_scrollState++;

    if (m_lineLengths.size() > m_maxLines) {
        std::size_t dropBytes = 0;
        while (m_lineLengths.size() > m_maxLines) {
            dropBytes += m_lineLengths.front();
            m_lineLengths.pop_front();
        }
        m_consoleBuffer.erase(0, dropBytes);
    }
}

const std::string& Logger::Output() const
{
    return m_consoleBuffer;
}

int Logger::GetState() const
{
    return m_scrollState;
}
