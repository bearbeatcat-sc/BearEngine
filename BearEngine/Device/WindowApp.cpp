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
	game_ = game;
	OutputDebugStringA("Hello,DirectX!!\n");

	int screenSize_Width = GetSystemMetrics(SM_CXSCREEN);
	int screenSize_Height = GetSystemMetrics(SM_CYSCREEN);
	
	window_size_ = { std::clamp(game_->windowSize_X,0,screenSize_Width),std::clamp(game_->windowSize_Y,0,screenSize_Height) };



	w_.cbSize = sizeof(WNDCLASSEX);
	w_.lpfnWndProc = (WNDPROC)WindowProc;
	w_.lpszClassName = game_->m_AppName;
	w_.hInstance = GetModuleHandle(nullptr);
	w_.hCursor = LoadCursor(NULL, IDC_ARROW);

	RegisterClassEx(&w_);
	RECT wrc = { 0,0,window_size_.window_Width,window_size_.window_Height };
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	window_size_.window_Width = wrc.right - wrc.left;
	window_size_.window_Height = wrc.bottom - wrc.top;

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

		bear_engine_->EngineUpdate();
		bear_engine_->EngineDrawBegin();
		bear_engine_->EngineDraw();
		game_->Update();
		bear_engine_->EngineDrawEnd();

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

WindowSize WindowApp::GetWindowSize()
{
	return window_size_;
}

WNDCLASSEX WindowApp::GetWndClassEx()
{
	return w_;
}

float WindowApp::GetAspect()
{
	return static_cast<float>(window_size_.window_Height) / static_cast<float>(window_size_.window_Width);
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
