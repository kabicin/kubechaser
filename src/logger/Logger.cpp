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

#include <iostream>

namespace
{
    std::weak_ptr<Logger>& Sink()
    {
        static std::weak_ptr<Logger> sink;
        return sink;
    }

    std::deque<std::string>& Pending()
    {
        static std::deque<std::string> pending;
        return pending;
    }
}

namespace Log
{
    void SetSink(const std::shared_ptr<Logger>& sink)
    {
        Sink() = sink;
        if (!sink) {
            return;
        }
        for (const std::string& message : Pending()) {
            sink->LogMessage(message);
        }
        Pending().clear();
    }

    void Write(Level level, const std::string& rawMessage)
    {
        // Values streamed with their own trailing std::endl (e.g. Material)
        // would otherwise leave blank lines in the console.
        std::string message = rawMessage;
        while (!message.empty() && (message.back() == '\n' || message.back() == '\r')) {
            message.pop_back();
        }

        std::string line;
        switch (level) {
        case Level::Info:
            line = message;
            std::cout << line << std::endl;
            break;
        case Level::Warning:
            line = "WARNING: " + message;
            std::cerr << line << std::endl;
            break;
        case Level::Error:
            line = "ERROR: " + message;
            std::cerr << line << std::endl;
            break;
        }

        if (std::shared_ptr<Logger> sink = Sink().lock()) {
            sink->LogMessage(line);
            return;
        }
        std::deque<std::string>& pending = Pending();
        pending.push_back(std::move(line));
        while (pending.size() > Logger::kDefaultMaxLines) {
            pending.pop_front();
        }
    }
}
