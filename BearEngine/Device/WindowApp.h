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
	const WindowSize& GetWindowSize();
	WNDCLASSEX GetWndClassEx();
	float GetAspect();
	const std::string FileOpen();
	void MsgBox(const std::string&& msg, const std::string& caption);

	static WindowSize window_size;
	static int sync_interval;

protected:
	WindowApp();
	virtual ~WindowApp();


private:
	HWND hwnd_;
	WNDCLASSEX w_;
	Game* game_;
	BearEngine* bear_engine_;
	
};

