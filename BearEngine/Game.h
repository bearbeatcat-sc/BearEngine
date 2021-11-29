#pragma once

#include <iostream>
#include "Device/WindowApp.h"

class Game
{
public:
	Game(const WCHAR* appName,int windowSize_X,int windowSize_Y);
	virtual ~Game();
	virtual void Init() = 0;
	virtual void Update() = 0;
	virtual void Destroy() = 0;

	const WCHAR* m_AppName;
	int windowSize_X;
	int windowSize_Y;
};

