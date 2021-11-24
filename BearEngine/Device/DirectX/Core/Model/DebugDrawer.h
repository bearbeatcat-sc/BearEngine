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
		SimpleMath::Matrix rotation;
	};

	struct DrawSpehereCommand
	{
		float radius;
		SimpleMath::Vector3 pos;
	};

	friend class Singleton<DebugDrawer>;
	void DrawCube(const SimpleMath::Vector3 size, const SimpleMath::Vector3 pos,
	              const SimpleMath::Quaternion rotate_qu);
	void DrawCube(const SimpleMath::Vector3 size, const SimpleMath::Vector3 pos, const SimpleMath::Matrix rotateMat);
	void DrawSphere(const float radius, const SimpleMath::Vector3 pos);
	
	void Draw();
	bool Init(const wchar_t* vertexShaderPath, const wchar_t* pixelShaderPath);

private:
	bool GenerateVertexBuffer(std::vector<XMFLOAT3>& vertices, ComPtr<ID3D12Resource> vertexBuffer, D3D12_VERTEX_BUFFER_VIEW& vertex_buffer_view);
	bool GenerateIndexBuffer(const std::vector<UINT>& indices, ComPtr<ID3D12Resource> indexBuffer, const int indexCount, D3D12_INDEX_BUFFER_VIEW&
	                         index_buffer_view);	
	void GenerateCubeData();
	void GenerateSphereData();

	void GeneratePipeline(const wchar_t* vertexShaderPath, const wchar_t* pixelShaderPath);
	bool GenerateConstantView();

	const int RenderCube(ID3D12GraphicsCommandList* tempCommand, UINT matIncSize, D3D12_GPU_DESCRIPTOR_HANDLE& handle, const int offset);
	const int RenderSphere(ID3D12GraphicsCommandList* tempCommand, UINT matIncSize, D3D12_GPU_DESCRIPTOR_HANDLE& handle, const int offset);

	bool InitConstantHeaps();
	bool GenerateConstantBuffers();
	
	void SetCubeConstantBuffer(int index, DebugDrawer::DrawCubeCommand cube);
	void SetSphereConstantBuffer(int index, DrawSpehereCommand sphere);
	
	bool IsAddCommand();


protected:
	DebugDrawer();
	~DebugDrawer();

private:

	struct ConstBufferData
	{
		XMMATRIX worldMat;
		XMMATRIX vpMat;
	};

	std::shared_ptr<Camera> _camera;
	PSO _pso;
	
	D3D12_VERTEX_BUFFER_VIEW _cubeVertexBufferView;
	D3D12_INDEX_BUFFER_VIEW _cubeIndexBufferView;
	std::shared_ptr<Buffer> _cubeVertexBuffer;
	std::shared_ptr<Buffer> _cubeIndexBuffer;
	int _cubeIndexCount;

	D3D12_VERTEX_BUFFER_VIEW _sphereVertexBufferView;
	D3D12_INDEX_BUFFER_VIEW _sphereIndexBufferView;
	std::shared_ptr<Buffer> _sphereVertexBuffer;
	std::shared_ptr<Buffer> _sphereIndexBuffer;
	int _sphereIndexCount;

	std::vector<DebugDrawer::DrawCubeCommand> _draw_cube_commands;
	std::vector<DebugDrawer::DrawSpehereCommand> _draw_sphere_commands;



	ComPtr<ID3D12DescriptorHeap> _constDescHeap;

	int m_DrawObjectCount = 128 * 4;

	std::vector<std::shared_ptr<Buffer>> _constantBuffers;

};

#endif