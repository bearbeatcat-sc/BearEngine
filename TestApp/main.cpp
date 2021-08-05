#include <Windows.h>
#include <Device/WindowApp.h>
#include "MainGame.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	MainGame* testGame = new MainGame();
	WindowApp::GetInstance().Run(testGame);

}

