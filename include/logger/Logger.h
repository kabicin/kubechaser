#pragma once
#ifndef LOGGER_H
#define LOGGER_H
#include <cstddef>
#include <deque>
#include <string>
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
#endif
