#pragma once
#include "Singleton.h"
#include <Windows.h>
#include "../Game.h"
#include "Components/Animations/Vector3AnimationCommand.h"

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
	const SimpleMath::Vector2& GetDebugGameWindowSize();
	WNDCLASSEX GetWndClassEx();
	float GetAspect();
	const std::string FileOpen();
	void MsgBox(const std::string&& msg, const std::string& caption);

	static WindowSize window_size;
	static SimpleMath::Vector2 _debugGameWindowSize;
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

