#pragma once
#include "Singleton.h"
#include <Windows.h>
#include "../Game.h"

class Game;
class BearEngine;

struct WindowSize
{
	int window_Width;
	int window_Height;
};

class WindowApp :
	public Singleton<WindowApp>
{
public:
	friend class Singleton<WindowApp>;

	HRESULT Run(Game* m_Game);
	HWND GetHWND();
	WindowSize GetWindowSize();
	WNDCLASSEX GetWndClassEx();
	float GetAspect();
	const std::string FileOpen();

protected:
	WindowApp();
	virtual ~WindowApp();


private:
	HWND m_Hwnd;
	WNDCLASSEX m_w;
	WindowSize windowSize;
	Game* m_Game;
	BearEngine* m_pBearEngine;

};

