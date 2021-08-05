#include "WindowApp.h"

#include "../Utility/Time.h"
#include "../BearEngine.h"

#include "DirectX/DirectXDevice.h"
#include "DirectX/DirectXGraphics.h"
#include "DirectX/DirectXInput.h"
#include "../Utility/StringUtil.h"

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_win32.h"
#include "../imgui/imgui_impl_dx12.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

HRESULT WindowApp::Run(Game* game)
{
	m_Game = game;
	OutputDebugStringA("Hello,DirectX!!\n");

	windowSize = { m_Game->windowSize_X,m_Game->windowSize_Y };

	m_w.cbSize = sizeof(WNDCLASSEX);
	m_w.lpfnWndProc = (WNDPROC)WindowProc;
	m_w.lpszClassName = m_Game->m_AppName;
	m_w.hInstance = GetModuleHandle(nullptr);
	m_w.hCursor = LoadCursor(NULL, IDC_ARROW);

	RegisterClassEx(&m_w);
	RECT wrc = { 0,0,windowSize.window_Width,windowSize.window_Height };
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	m_Hwnd = CreateWindow(
		m_w.lpszClassName,
		m_Game->m_AppName,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		m_w.hInstance,
		nullptr
	);

	ShowWindow(m_Hwnd, SW_SHOW);

	m_pBearEngine = new BearEngine();
	m_pBearEngine->InitEngine();

	if (ImGui::CreateContext() == nullptr)
	{
		assert(0);
		return false;
	}

	ImGui_ImplWin32_Init(m_Hwnd);

	bool result = ImGui_ImplDX12_Init(DirectXDevice::GetInstance().GetDevice(),
		3,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DirectXGraphics::GetInstance().GetImGUIDescriptrHeap(),
		DirectXGraphics::GetInstance().GetImGUIDescriptrHeap()->GetCPUDescriptorHandleForHeapStart(),
		DirectXGraphics::GetInstance().GetImGUIDescriptrHeap()->GetGPUDescriptorHandleForHeapStart());



	m_Game->Init();

	Time time;

	MSG msg{};



	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		time.Update();
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		m_pBearEngine->EngineUpdate();
		m_pBearEngine->EngineDrawBegin();
		m_pBearEngine->EngineDraw();
		m_Game->Update();
		m_pBearEngine->EngineDrawEnd();

		if (msg.message == WM_QUIT)
		{
			m_Game->Destroy();
			break;
		}
	}

	ImGui_ImplDX12_Shutdown();
	ImGui::DestroyContext();
	UnregisterClass(m_w.lpszClassName, m_w.hInstance);

	return S_OK;
}

HWND WindowApp::GetHWND()
{
	return m_Hwnd;
}

WindowSize WindowApp::GetWindowSize()
{
	return windowSize;
}

WNDCLASSEX WindowApp::GetWndClassEx()
{
	return m_w;
}

float WindowApp::GetAspect()
{
	return static_cast<float>(windowSize.window_Height) / static_cast<float>(windowSize.window_Width);
}

const std::string WindowApp::FileOpen()
{
	OPENFILENAME ofn;
	WCHAR szFile[260];
	HWND hwnd = NULL;
	HANDLE hf;

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = szFile;


	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = L"All\0*.*\0text\0*.TXT\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;


	if (GetOpenFileName(&ofn) == TRUE)
	{
		//hf = CreateFile(ofn.lpstrFile,
		//	GENERIC_READ,
		//	0,
		//	(LPSECURITY_ATTRIBUTES)NULL,
		//	OPEN_EXISTING,
		//	FILE_ATTRIBUTE_NORMAL,
		//	(HANDLE)NULL);
	}

	char* dest = (char*)malloc(100);
	size_t size;

	wcstombs_s(&size, dest, (size_t)100, szFile, 100);

	std::string filePath = dest;

	if(dest)
	{
		free(dest);
	}
	
	return filePath;
}



WindowApp::WindowApp()
{

}



WindowApp::~WindowApp()
{
	delete m_Game;
	delete m_pBearEngine;
}
