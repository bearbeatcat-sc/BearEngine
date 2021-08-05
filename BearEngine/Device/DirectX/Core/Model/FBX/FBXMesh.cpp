#include "FBXMesh.h"
#include "../../../../../Device/DirectX/Core/Buffer.h"

FBX_Mesh::FBX_Mesh(FbxMesh* mesh,FbxNode* node)
	:mesh(mesh), m_node(node)
{
	GetVertex();
	GetIndex();
	GetNormal();
	//GenerateVertexData();
}

FBX_Mesh::~FBX_Mesh()
{
	mesh = 0;
	m_node = 0;
}



D3D12_VERTEX_BUFFER_VIEW FBX_Mesh::getVertexBufferView()
{
	return m_vbView;
}

D3D12_INDEX_BUFFER_VIEW FBX_Mesh::getIndexBufferView()
{
	return m_ibView;
}

int FBX_Mesh::getIndexCount()
{
	return m_indexCount;
}

int FBX_Mesh::getVertexCount()
{
	return m_vertexCount;
}

void FBX_Mesh::GetVertex()
{
	int polygonCount = mesh->GetPolygonCount();

	vertexDatas.resize(mesh->GetControlPointsCount());

	// 頂点情報の取得はいい感じ 
	for (int i = 0; i < polygonCount; i++)
	{
		int polygonSize = mesh->GetPolygonSize(i);

		for (int j = 0; j < polygonSize; j++)
		{
			int index = mesh->GetPolygonVertex(
				i, j
			);

			FbxVector4 controlPoint = mesh->GetControlPointAt(index);
			XMFLOAT3 vertex;
			vertex.x = controlPoint[0];
			vertex.y = controlPoint[1];
			vertex.z = controlPoint[2];

			Vertex vert;
			vert.pos = vertex;

			vertexDatas[index] = vert;


		}
	}


	// Vertexを設定
	m_VertexBuffer = std::shared_ptr<Buffer>(new Buffer());
	int size = sizeof(Vertex) * vertexDatas.size();
	m_VertexBuffer->init(D3D12_HEAP_TYPE_UPLOAD, size, D3D12_RESOURCE_STATE_GENERIC_READ);
	auto vertBuff = m_VertexBuffer->getBuffer();

	Vertex* vertMap = nullptr;

	if (FAILED(vertBuff->Map(0, nullptr, (void**)&vertMap)))
	{
		return;

	}

	for (int i = 0; i < vertexDatas.size(); i++)
	{
		vertMap[i] = vertexDatas[i];
	}

	vertBuff->Unmap(0, nullptr);

	m_vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	m_vbView.SizeInBytes = sizeof(vertexDatas[0]) * vertexDatas.size();
	m_vbView.StrideInBytes = sizeof(vertexDatas[0]);
}

void FBX_Mesh::GetIndex()
{

	// インデックス数を算出
	int indexesCount = 0;
	int polygonsCount = mesh->GetPolygonCount();
	int indexArray[6] = { 0,1,2,0,2,3 };

	for (int i = 0; i < polygonsCount; i++)
	{
		int polygonSize = mesh->GetPolygonSize(i);

		switch (polygonSize)
		{
		case 3:
			indexesCount += 3;
			break;
		case 4:
			indexesCount += 6;
			break;
		}
	}

	indices.resize(indexesCount);

	int index = 0;
	for (int i = 0; i < polygonsCount; i++)
	{
		int polygonSize = mesh->GetPolygonSize(i);

		switch (polygonSize)
		{		
		case 3:
			for (int j = 0; j < 3; j++)
			{
				indices[index] = mesh->GetPolygonVertex(i, j);
				index++;
			}
			break;

		case 4:
			for (int j = 0; j < 6; j++)
			{
				indices[index] = mesh->GetPolygonVertex(i, indexArray[j]);
				index++;
			}
			break;
		}
	}






	m_indexCount = indices.size();

	// indexを設定
	m_IndexBuffer = std::shared_ptr<Buffer>(new Buffer());
	int size = sizeof(int) * m_indexCount;
	m_IndexBuffer->init(D3D12_HEAP_TYPE_UPLOAD, size, D3D12_RESOURCE_STATE_GENERIC_READ);
	auto indexBuff = m_IndexBuffer->getBuffer();

	unsigned short* indexMap = nullptr;
	indexBuff->Map(0, nullptr, (void**)&indexMap);

	for (int i = 0; i < indices.size(); i++)
	{
		indexMap[i] = indices[i];
	}

	indexBuff->Unmap(0, nullptr);


	m_ibView.BufferLocation = indexBuff->GetGPUVirtualAddress();
	m_ibView.Format = DXGI_FORMAT_R16_UINT;
	m_ibView.SizeInBytes = sizeof(indices[0]) * indices.size();

}

void FBX_Mesh::GetNormal()
{

	auto element = mesh->CreateElementNormal();
	auto mappingMode = element->GetMappingMode();
	auto referenceMode = element->GetReferenceMode();
	const auto& indexArray = element->GetIndexArray();
	const auto& directArray = element->GetDirectArray();

	assert((referenceMode == FbxGeometryElement::eDirect) || (referenceMode == FbxGeometryElement::eIndexToDirect));


	if (mappingMode == FbxGeometryElement::eByControlPoint)
	{
		for (auto index : indices)
		{
			auto normalIndex = (referenceMode == FbxGeometryElement::eDirect)
				? index
				: indexArray.GetAt(index);

			auto normal = directArray.GetAt(normalIndex);
			XMFLOAT3 n;
			n.x = normal[0];
			n.y = normal[1];
			n.z = normal[2];
			normals.push_back(n);

		}
	}
	else if (mappingMode == FbxGeometryElement::eByPolygonVertex)
	{
		auto indexByPolygonVertex = 0;
		auto polygonCount = mesh->GetPolygonCount();

		for (int i = 0; i < polygonCount; ++i)
		{
			auto polygonSize = mesh->GetPolygonSize(i);

			for (int j = 0; j < polygonSize; ++j)
			{
				auto normalIndex = (referenceMode == FbxGeometryElement::eDirect)
					? indexByPolygonVertex
					: indexArray.GetAt(indexByPolygonVertex);

				auto normal = directArray.GetAt(normalIndex);

				XMFLOAT3 n;
				n.x = normal[0];
				n.y = normal[1];
				n.z = normal[2];
				normals.push_back(n);

				++indexByPolygonVertex;
			}
		}
	}
	else
	{
		return;
	}
}

void FBX_Mesh::GenerateVertexData()
{

}

