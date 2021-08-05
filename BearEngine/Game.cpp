#include "Game.h"

Game::Game( const WCHAR* appName, int windowSize_X, int windowSize_Y)
	: m_AppName(appName), windowSize_X(windowSize_X), windowSize_Y(windowSize_Y)
{
	_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);
}

Game::~Game()
{
}

