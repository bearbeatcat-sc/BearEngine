#include "WindowApp.h"

#include <algorithm>

#include "../Utility/Time.h"
#include "../BearEngine.h"

#include "DirectX/DirectXDevice.h"
#include "DirectX/DirectXGraphics.h"
#include "DirectX/DirectXInput.h"
#include "../Utility/StringUtil.h"

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_win32.h"
#include "../imgui/imgui_impl_dx12.h"
#include "Device/GUISystem.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
WindowSize WindowApp::window_size;
int WindowApp::sync_interval;

LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_SIZE:
		WindowApp::window_size.window_Width = LOWORD(lparam);
		WindowApp::window_size.window_Height = HIWORD(lparam);
		return 0;
	}

	ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

HRESULT WindowApp::Run(Game* game)
{
	game_ = game;
	OutputDebugStringA("Hello,DirectX!!\n");

	// デスクトップのRectを取得
	RECT desktopRect;
	GetWindowRect(GetDesktopWindow(), &desktopRect);

	const int desktop_width = desktopRect.right - desktopRect.left;
	const int desktop_height = desktopRect.bottom - desktopRect.top;

	// デスクトップサイズよりもゲーム側の指定ウィンドウサイズが大きかったら制限する。
	window_size = { std::clamp(game->windowSize_X,0,desktop_width),std::clamp(game->windowSize_Y,0,desktop_height) };


	w_.cbSize = sizeof(WNDCLASSEX);
	w_.lpfnWndProc = (WNDPROC)WindowProc;
	w_.lpszClassName = game_->m_AppName;
	w_.hInstance = GetModuleHandle(nullptr);
	w_.hCursor = LoadCursor(NULL, IDC_ARROW);

	RegisterClassEx(&w_);
	RECT wrc = { 0,0,window_size.window_Width,window_size.window_Height };
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	hwnd_ = CreateWindow(
		w_.lpszClassName,
		game_->m_AppName,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		w_.hInstance,
		nullptr
	);




	ShowWindow(hwnd_, SW_SHOW);

	// クライアントサイズに補正する
	RECT clientSize;
	GetClientRect(hwnd_, &clientSize);
	window_size.window_Width = clientSize.right - clientSize.left;
	window_size.window_Height = clientSize.bottom - clientSize.top;


	// リフレッシュレートに合わせて制限
	auto hdc = GetDC(hwnd_);
	auto rate = GetDeviceCaps(hdc, VREFRESH);

	int interval = 0;
	if (rate <= 60) interval = 1;
	if (rate >= 120) interval = 2;

	sync_interval = interval;
	
	bear_engine_ = new BearEngine();
	bear_engine_->InitEngine();

	if (ImGui::CreateContext() == nullptr)
	{
		assert(0);
		return false;
	}

	ImGui_ImplWin32_Init(hwnd_);

	bool result = ImGui_ImplDX12_Init(DirectXDevice::GetInstance().GetDevice(),
		3,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DirectXGraphics::GetInstance().GetImGUIDescriptrHeap(),
		DirectXGraphics::GetInstance().GetImGUIDescriptrHeap()->GetCPUDescriptorHandleForHeapStart(),
		DirectXGraphics::GetInstance().GetImGUIDescriptrHeap()->GetGPUDescriptorHandleForHeapStart());



	game_->Init();

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

#ifdef _DEBUG
		GUISystem::GetInstance().BeginGUI();
#endif	
		bear_engine_->EngineUpdate();
		bear_engine_->BeginRender();
		game_->Update();
		bear_engine_->EndRender();

		if (msg.message == WM_QUIT)
		{
			game_->Destroy();
			break;
		}
	}

	ImGui_ImplDX12_Shutdown();
	ImGui::DestroyContext();
	UnregisterClass(w_.lpszClassName, w_.hInstance);

	return S_OK;
}

HWND WindowApp::GetHWND()
{
	return hwnd_;
}

const WindowSize& WindowApp::GetWindowSize()
{
	return window_size;
}

WNDCLASSEX WindowApp::GetWndClassEx()
{
	return w_;
}

float WindowApp::GetAspect()
{
	return static_cast<float>(window_size.window_Height) / static_cast<float>(window_size.window_Width);
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

void WindowApp::MsgBox(const std::string&& msg,const std::string& caption)
{
	MessageBoxA(hwnd_, msg.c_str(), caption.c_str(), MB_OK);
}


WindowApp::WindowApp()
{

}



WindowApp::~WindowApp()
{
	delete game_;
	delete bear_engine_;
}
