#pragma once

#include "../Singleton.h"

// エンジンそのものの描画を行うパイプライン
class SystemRenderingPipeLine
	:public Singleton<SystemRenderingPipeLine>
{
public:
	friend class Singleton<SystemRenderingPipeLine>;
	SystemRenderingPipeLine() = default;
	~SystemRenderingPipeLine() = default;

	void InitPipeLine();
	void BeginRender();
	void EndRender();

private:
	void LoadAssets();
};
