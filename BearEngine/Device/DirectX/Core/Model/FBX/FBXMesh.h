#pragma once
#include <fbxsdk.h>
#include <vector>
#include <d3d12.h>
#include <DirectXMath.h>
#include <memory>

using namespace DirectX;

class Buffer;


class FBX_Mesh
{
public :
	FBX_Mesh(FbxMesh* mesh,FbxNode* node);
	~FBX_Mesh();
	D3D12_VERTEX_BUFFER_VIEW getVertexBufferView();
	D3D12_INDEX_BUFFER_VIEW getIndexBufferView();
	int getIndexCount();
	int getVertexCount();

private:

	void GetVertex();
	void GetIndex();
	void GetNormal();
	void GenerateVertexData();

private:

	struct Vertex
	{
		XMFLOAT3 pos;
		XMFLOAT3 normal;
	};

	std::shared_ptr<Buffer> m_VertexBuffer;
	std::shared_ptr<Buffer> m_IndexBuffer;

	D3D12_VERTEX_BUFFER_VIEW m_vbView;
	D3D12_INDEX_BUFFER_VIEW m_ibView;

	std::vector< int> indices;
	std::vector<Vertex> vertexDatas;
	std::vector<XMFLOAT3> vertices;
	std::vector<XMFLOAT3> normals;

	int m_indexCount;
	int m_vertexCount;
	FbxMesh* mesh;
	FbxNode* m_node;
};