#pragma once
#ifndef LOGGER_H
#define LOGGER_H
#include <cstddef>
#include <deque>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
class Logger
{
public:
	static constexpr std::size_t kDefaultMaxLines = 1000;

private:
	std::string m_consoleBuffer;
	// Length (including the trailing '\n') of each line currently held in
	// m_consoleBuffer, oldest first, so old lines can be trimmed cheaply.
	std::deque<std::size_t> m_lineLengths;
	std::size_t m_maxLines;
	int m_scrollState = 0;
public:
	explicit Logger(std::size_t maxLines = kDefaultMaxLines);
	void LogMessage(const std::string& message);
	// Returns a reference to the internal buffer; no copy is made. Holds at
	// most the newest maxLines messages.
	const std::string& Output() const;
	// Total number of messages ever logged (not reduced by trimming), so
	// callers can detect new messages.
	int GetState() const;
};

// Process-wide diagnostics. Code that has no access to WindowServices (mesh
// loading, shader compilation, file readers, ...) logs through these so the
// messages reach both the terminal and the in-app console panel.
//
// Info goes to stdout; Warning and Error go to stderr with a "WARNING: " /
// "ERROR: " prefix (also used in the console). Messages logged before a sink
// is installed are buffered (up to Logger::kDefaultMaxLines) and replayed
// into the sink when SetSink is called. Not thread-safe; call from the main
// thread only.
namespace Log
{
	enum class Level { Info, Warning, Error };

	// Installs the logger that receives diagnostics. The sink is held weakly:
	// once the Logger is destroyed, messages go only to the terminal (and are
	// buffered again until a new sink is set). Pass nullptr to detach.
	void SetSink(const std::shared_ptr<Logger>& sink);

	void Write(Level level, const std::string& message);

	namespace detail
	{
		template <typename... Args>
		std::string Format(Args&&... args)
		{
			std::ostringstream stream;
			(stream << ... << std::forward<Args>(args));
			return stream.str();
		}
	}

	// Each argument is streamed with operator<< and the results concatenated
	// into a single message.
	template <typename... Args>
	void Info(Args&&... args) { Write(Level::Info, detail::Format(std::forward<Args>(args)...)); }
	template <typename... Args>
	void Warning(Args&&... args) { Write(Level::Warning, detail::Format(std::forward<Args>(args)...)); }
	template <typename... Args>
	void Error(Args&&... args) { Write(Level::Error, detail::Format(std::forward<Args>(args)...)); }
}
#endif
