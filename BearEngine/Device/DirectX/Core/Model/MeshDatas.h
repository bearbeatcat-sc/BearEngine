#ifndef _MESH_DATAS_H_
#define _MESH_DATAS_H_

#include <SimpleMath.h>
#include <vector>
#include <d3d12.h>
#include <DirectXMath.h>
#include <memory>
#include <unordered_map>
#include <wrl/client.h>

#include "Device/Raytracing/PhysicsBaseMaterial.h"
#include "Material.h"

using namespace DirectX;

class Buffer;
class Camera;

class MeshData
{
public:

	// 視錐台カリング用に使用
	struct MeshAABB
	{
		DirectX::SimpleMath::Vector3 m_Center;
		DirectX::SimpleMath::Vector3 m_Min;
		DirectX::SimpleMath::Vector3 m_Max;
	};

	struct Vertex
	{
		XMFLOAT3 pos;
		XMFLOAT3 normal;
		XMFLOAT2 uv;
	};


	struct ModelData
	{
		std::vector<Vertex> vertices;
		std::vector<UINT> m_Indices;
		std::vector<XMFLOAT2> texCords;
		std::unordered_map<std::string, MaterialData> m_MaterialDatas;
	};


	MeshData();
	~MeshData();
	bool GenerateMesh(std::wstring& modelName, std::vector<Vertex>& positions, std::vector<UINT>& _indices, std::unordered_map<std::string, MaterialData>& matData);
	void SetPhysicsBaseMaterial(const PhysicsBaseMaterial& testMat);
	const PhysicsBaseMaterial& GetPhysicsBaseMaterial();
	
	const UINT GetVertexCount();
	const UINT GetIndexCount();

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateWhiteTexture();
	std::unordered_map<std::string, std::shared_ptr<Buffer>>& GetMaterialBuffer();
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12Resource>>& GetTextureBuff();
	Microsoft::WRL::ComPtr<ID3D12Resource> GetFirstTextureBuffer();
	std::vector<MaterialData>& GetDrawMaterialDatas();
	std::unordered_map<std::string,MaterialData>& GetMaterialDatas();
	ID3D12Resource* GetWhiteTextureBuff();
	const std::vector<XMFLOAT3>& GetPositions();
	const MeshAABB& GetMeshAABB();

	std::shared_ptr<Buffer> GetVertexBuffer();
	std::shared_ptr<Buffer> GetIndexBuffer();

	D3D12_VERTEX_BUFFER_VIEW m_vbView;
	D3D12_INDEX_BUFFER_VIEW m_ibView;
	UINT m_indexCount;
	UINT m_VertexCount;
	
	D3D12_GPU_DESCRIPTOR_HANDLE m_vb_h_gpu_descriptor_handle;
	D3D12_CPU_DESCRIPTOR_HANDLE m_vb_h_cpu_descriptor_handle;

	D3D12_GPU_DESCRIPTOR_HANDLE m_ib_h_gpu_descriptor_handle;
	D3D12_CPU_DESCRIPTOR_HANDLE m_ib_h_cpu_descriptor_handle;

private:
	bool GenerateMaterialBuffer();
	bool GenerateTextureBuffer();
	void CreateMeshAABB(const SimpleMath::Vector3& min, const SimpleMath::Vector3& max);


private:

	std::shared_ptr<Buffer> m_VertexBuffer;
	std::shared_ptr<Buffer> m_IndexBuffer;

	PhysicsBaseMaterial _RaytraceMaterial;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_WhiteTexBuffer;
	std::vector<XMFLOAT3> m_Positions;
	MeshAABB m_MeshAABB;


	std::unordered_map<std::string, std::shared_ptr<Buffer>> m_MaterialBuffers;
	std::unordered_map<std::string, MaterialData> m_MaterialDatas;
	std::vector<MaterialData> m_DrawMaterialDatas;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12Resource>> m_Textures;


};

#endif