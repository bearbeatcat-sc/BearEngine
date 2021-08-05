#ifndef _MESH_DATAS_H_
#define _MESH_DATAS_H_

#include "../../../../Device/DirectX/Core/PSOManager.h"

#include <SimpleMath.h>
#include <vector>
#include <d3d12.h>
#include <DirectXMath.h>
#include <memory>
#include <unordered_map>

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


	struct Material
	{
		XMFLOAT3 ambient;
		float pad1;
		XMFLOAT3 diffuse;
		float pad2;
		XMFLOAT3 specular;

		Material()
		{
			ambient = { 0.3f,0.3f,0.3f };
			diffuse = { 0.0f,0.0f,0.0f };
			specular = { 1.0f,1.0f,1.0f };
		}
	};

	struct MaterialData
	{
		Material material;
		std::string texFilePath;
		unsigned short indexCount;
		unsigned short m_StartIndex;

		bool operator<(const MaterialData& right) const
		{
			return m_StartIndex < right.m_StartIndex;
		}
	};

	struct ModelData
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned short> m_Indices;
		std::vector<XMFLOAT2> texCords;
		std::unordered_map<std::string, MeshData::MaterialData> m_MaterialDatas;


	};

	MeshData();
	~MeshData();
	bool GenerateMesh(std::vector<Vertex>& positions, std::vector<unsigned short>& _indices, std::unordered_map<std::string, MaterialData>& matData);
	ComPtr<ID3D12Resource> CreateWhiteTexture();
	std::unordered_map<std::string, std::shared_ptr<Buffer>>& GetMaterialBuffer();
	std::unordered_map<std::string, ComPtr<ID3D12Resource>>& GetTextureBuff();
	ComPtr<ID3D12Resource> GetFirstTextureBuffer();
	std::vector<MeshData::MaterialData>& GetDrawMaterialDatas();
	std::unordered_map<std::string, MeshData::MaterialData>& GetMaterialDatas();
	ID3D12Resource* GetWhiteTextureBuff();
	const std::vector<XMFLOAT3>& GetPositions();
	const MeshAABB& GetMeshAABB();

	D3D12_VERTEX_BUFFER_VIEW m_vbView;
	D3D12_INDEX_BUFFER_VIEW m_ibView;
	size_t m_indexCount;

private:
	bool GenerateMaterialBuffer();
	bool GenerateTextureBuffer();
	void CreateMeshAABB(const SimpleMath::Vector3& min, const SimpleMath::Vector3& max);


private:

	std::shared_ptr<Buffer> m_VertexBuffer;
	std::shared_ptr<Buffer> m_IndexBuffer;
	ComPtr<ID3D12Resource> m_WhiteTexBuffer;
	std::vector<XMFLOAT3> m_Positions;
	MeshAABB m_MeshAABB;


	std::unordered_map<std::string, std::shared_ptr<Buffer>> m_MaterialBuffers;
	std::unordered_map<std::string,MaterialData> m_MaterialDatas;
	std::vector<MaterialData> m_DrawMaterialDatas;
	std::unordered_map<std::string, ComPtr<ID3D12Resource>> m_Textures;

};

#endif