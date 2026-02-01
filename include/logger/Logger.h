#pragma once
#ifndef LOGGER_H
#define LOGGER_H
#include <string>
class Logger
{
private:
	std::string m_consoleBuffer;
	int m_scrollState = 0;
public:
	Logger();
	void LogMessage(std::string message);
	std::string Output();
	int GetState();
};
#endif

