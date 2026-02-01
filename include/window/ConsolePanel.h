#pragma once
#ifndef CONSOLEPANEL_H
#define CONSOLEPANEL_H

#include "BaseWindow.h"
#include "../logger/Logger.h"

class ConsolePanel : public BaseWindow
{
private:
	int logState = -1;
public:
	ConsolePanel(int x, int y, int width, int height);
	~ConsolePanel();
	void Render();
};


#endif