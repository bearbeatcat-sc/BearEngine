#ifndef _DEBUG_DRAWER_H_
#define _DEBUG_DRAWER_H_

#include "../../../Singleton.h"
#include <SimpleMath.h>
#include <memory>
#include "../PSOManager.h"

using namespace DirectX;

class Camera;
class Buffer;

class DebugDrawer
	:public Singleton<DebugDrawer>
{
public:

	struct DrawCubeCommand
	{
		SimpleMath::Vector3 size;
		SimpleMath::Vector3 pos;
	};

	struct DrawSpehereCommand
	{
		float radius;
		int tessletion;
		SimpleMath::Vector3 pos;
	};

	friend class Singleton<DebugDrawer>;
	void DrawCube(SimpleMath::Vector3 size, SimpleMath::Vector3 pos);
	void Draw();
	bool Init(const wchar_t* vertexShaderPath, const wchar_t* pixelShaderPath);

private:
	bool GenerateVertexBuffer(std::vector<XMFLOAT3>& vertices);
	bool GenerateIndexBuffer(const std::vector<unsigned short>& indices);
	void GenerateCubeData();
	bool GenerateConstantView();
	bool InitConstantHeaps();
	bool GenerateConstantBuffers();
	void SetConstantBuff(int index, DebugDrawer::DrawCubeCommand cube);

protected:
	DebugDrawer();
	~DebugDrawer();

private:

	struct ConstBufferData
	{
		XMMATRIX worldMat;
		XMMATRIX vpMat;
	};

	std::shared_ptr<Camera> m_Camera;
	PSO m_PSO;
	D3D12_VERTEX_BUFFER_VIEW m_vbView;
	D3D12_INDEX_BUFFER_VIEW m_ibView;

	std::vector<DebugDrawer::DrawCubeCommand> m_CubeCommand;
	std::vector<DebugDrawer::DrawSpehereCommand> m_SpehereCommand;


	std::shared_ptr<Buffer> m_VertexBuffer;
	std::shared_ptr<Buffer> m_IndexBuffer;

	ComPtr<ID3D12DescriptorHeap> m_ConstDescHeap;

	int m_DrawObjectCount = 128 * 4;

	std::vector<std::shared_ptr<Buffer>> m_ConstantBuffers;

	int m_IndexCount;
};

#endif