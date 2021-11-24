#pragma once

#include "Singleton.h"
#include "imgui/imgui.h"

class GUISystem
	:public Singleton<GUISystem>

{
public:
	friend class Singleton<GUISystem>;

	
	GUISystem() = default;
	~GUISystem() = default;

	void BeginGUI();
	void DrawGUI();
	void RenderRenderingDebugWindow();
	void DrawDebug();
	void EndGUI();

private:
	void DrawMenuBar();

	bool _isShowDebugRenderingWindow;
	bool _isShowLog;
};
