#ifndef _SKYBOX_H_
#define _SKYBOX_H_

#include "SimpleMath.h"
#include <d3d12.h>
#include <DirectXMath.h>
#include "Device/DirectX/Core/PSOManager.h"
#include <memory>
#include "../Device/DirectX/Core/Model/MeshDatas.h"

using namespace DirectX;

class Camera;
class Buffer;

class SkyBox
{
public:
	SkyBox(std::string filePath, std::shared_ptr<Camera> camera);
	SkyBox(std::string filePath, std::shared_ptr<Camera> camera,SimpleMath::Vector3 cubeScale);
	~SkyBox();
	void Init();
	void Update();
	void Draw();
	void SetAdjustmentPosition(SimpleMath::Vector3 pos);
	void SetScale(SimpleMath::Vector3 scale);
	void SetColor(SimpleMath::Color color);
	ID3D12Resource* GetCubeResouce();
	ID3D12DescriptorHeap* GETDescHeap();
	

private:

	bool generateVertexBuffer();
	bool generateIndexBuffer();
	bool generateConstantBuffer();
	bool generateTextureBuffer();
	void updateConstBuff();
	bool CreatePSO();

private:
	PSO m_PSO;

	struct ConstBufferData
	{
		XMMATRIX mat;
		XMFLOAT4 color;
		XMFLOAT3 eyePos;
	};

	std::shared_ptr<Buffer> m_ConstantBuffer;
	std::shared_ptr<Buffer> m_VertexBuffer;
	std::shared_ptr<Buffer> m_IndexBuffer;
	ComPtr<ID3D12Resource> cubeMap;


	D3D12_CPU_DESCRIPTOR_HANDLE HeapHandle;
	ComPtr<ID3D12DescriptorHeap> basicDescHeap;

	D3D12_VERTEX_BUFFER_VIEW m_vbView;
	D3D12_INDEX_BUFFER_VIEW m_ibView;

	MeshData::ModelData m_MeshDatas;
	std::string m_TextureName;

	SimpleMath::Matrix m_WorldMat;
	std::shared_ptr<Camera> m_Camera;

	SimpleMath::Color m_Color;
	int m_IndexCount;

	SimpleMath::Vector3 m_Scale;
	SimpleMath::Vector3 m_AdjustmentPosition;

	const wchar_t* basicVertexShaderPath;
	const wchar_t* basicPixelShaderPath;
};

#endif