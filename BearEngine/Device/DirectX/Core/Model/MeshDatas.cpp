#include "MeshDatas.h"

#include "Device/DirectX/Core/Buffer.h"
#include "Device/DirectX/DirectXDevice.h"
#include "Device/DirectX/DirectXGraphics.h"
#include "Device/Texture.h"
#include "Utility/StringUtil.h"

#include "../../../../Utility/Math/MathUtility.h"

MeshData::MeshData()
{
}

MeshData::~MeshData()
{
	//delete m_VertexBuffer;
	//delete m_IndexBuffer;
	//delete m_MetarilBuffer;

	//if (m_TexuterBuffer != nullptr)	m_TexuterBuffer->Release();
	//if (m_WhiteTexBuffer != nullptr) m_WhiteTexBuffer->Release();

	m_MaterialBuffers.clear();
	m_MaterialDatas.clear();
	m_DrawMaterialDatas.clear();
	m_Textures.clear();
}

bool MeshData::GenerateMesh(std::vector<Vertex>& positions, std::vector<UINT>& _indices, std::unordered_map<std::string, MaterialData>& matData)
{
#pragma region Vertex
	m_Positions.resize(positions.size());
	size_t positionSize = m_Positions.size();

	m_VertexCount = positionSize;

	// 頂点情報だけをコピー
	for (int i = 0; i < positionSize; ++i)
	{
		m_Positions[i] = positions[i].pos;		
	}


	// 頂点情報の最小値と最大値でAABB生成
	auto min = MathUtility::GetMin(m_Positions);
	auto max = MathUtility::GetMax(m_Positions);
	CreateMeshAABB(min,max);


	// Vertexを設定
	m_VertexBuffer = std::make_shared<Buffer>();
	size_t size = sizeof(Vertex) * positions.size();
	m_VertexBuffer->init(D3D12_HEAP_TYPE_UPLOAD, size, D3D12_RESOURCE_STATE_GENERIC_READ);
	auto vertBuff = m_VertexBuffer->getBuffer();

	// バッファ生成失敗
	if (vertBuff == nullptr)
	{
		return false;
	}

	Vertex* vertMap = nullptr;



	if (FAILED(vertBuff->Map(0, nullptr, (void**)&vertMap)))
	{
		return false;
	}

	std::copy(positions.begin(), positions.end(), vertMap);


	vertBuff->Unmap(0, nullptr);


	m_vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	m_vbView.SizeInBytes = sizeof(positions[0]) * positions.size();
	m_vbView.StrideInBytes = sizeof(positions[0]);

#pragma endregion

	m_indexCount = _indices.size();

	// indexを設定
	m_IndexBuffer = std::make_shared<Buffer>();
	size = sizeof(UINT) * m_indexCount;
	m_IndexBuffer->init(D3D12_HEAP_TYPE_UPLOAD, size, D3D12_RESOURCE_STATE_GENERIC_READ);
	auto indexBuff = m_IndexBuffer->getBuffer();

	UINT* indexMap = nullptr;

	indexBuff->Map(0, nullptr, (void**)&indexMap);

	// TODO:インデックスバッファの生成方法
	std::copy(_indices.begin(), _indices.end(), indexMap);


	indexBuff->Unmap(0, nullptr);


	m_ibView.BufferLocation = indexBuff->GetGPUVirtualAddress();
	m_ibView.Format = DXGI_FORMAT_R16_UINT;
	m_ibView.SizeInBytes = sizeof(_indices[0]) * _indices.size();


	m_MaterialDatas = matData;



	GenerateTextureBuffer();

	if (!GenerateMaterialBuffer())
	{
		return false;
	}

	m_WhiteTexBuffer = CreateWhiteTexture();

	// マテリアルデータを描画用にコピー
	for (auto material : m_MaterialDatas)
	{
		m_DrawMaterialDatas.push_back(material.second);
	}

	std::sort(m_DrawMaterialDatas.begin(), m_DrawMaterialDatas.end());

	return true;
}



ComPtr<ID3D12Resource> MeshData::CreateWhiteTexture()
{
	D3D12_HEAP_PROPERTIES texHeapProp = {};

	texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	texHeapProp.VisibleNodeMask = 0;

	D3D12_RESOURCE_DESC resDEsc = {};
	resDEsc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDEsc.Width = 4;
	resDEsc.Height = 4;
	resDEsc.DepthOrArraySize = 1;
	resDEsc.SampleDesc.Count = 1;
	resDEsc.SampleDesc.Quality = 1;
	resDEsc.MipLevels = 1;
	resDEsc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDEsc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDEsc.Flags = D3D12_RESOURCE_FLAG_NONE;

	ComPtr<ID3D12Resource> whiteBuff = nullptr;

	auto result = DirectXDevice::GetInstance().GetDevice()->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDEsc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&whiteBuff));

	if (FAILED(result))
	{
		return nullptr;
	}

	std::vector<unsigned char> data(4 * 4 * 4);
	std::fill(data.begin(), data.end(), 0xff);

	result = whiteBuff->WriteToSubresource(
		0,
		nullptr,
		data.data(),
		4 * 4,
		data.size());

	return whiteBuff;
}

std::unordered_map<std::string, std::shared_ptr<Buffer>>& MeshData::GetMaterialBuffer()
{
	return m_MaterialBuffers;
}

std::unordered_map<std::string, ComPtr<ID3D12Resource>>& MeshData::GetTextureBuff()
{
	return m_Textures;
}

ComPtr<ID3D12Resource> MeshData::GetFirstTextureBuffer()
{
	if (m_MaterialDatas.size() == 0)return ComPtr<ID3D12Resource>();

	return m_Textures.at(m_MaterialDatas.begin()->first);
}

std::vector<MeshData::MaterialData>& MeshData::GetDrawMaterialDatas()
{
	return m_DrawMaterialDatas;
}

std::unordered_map<std::string, MeshData::MaterialData>& MeshData::GetMaterialDatas()
{
	return m_MaterialDatas;
}

ID3D12Resource* MeshData::GetWhiteTextureBuff()
{
	return m_WhiteTexBuffer.Get();
}

const std::vector<XMFLOAT3>& MeshData::GetPositions()
{
	return m_Positions;
}

const MeshData::MeshAABB& MeshData::GetMeshAABB()
{
	return m_MeshAABB;
}

std::shared_ptr<Buffer> MeshData::GetVertexBuffer()
{
	return m_VertexBuffer;
}

std::shared_ptr<Buffer> MeshData::GetIndexBuffer()
{
	return m_IndexBuffer;
}


bool MeshData::GenerateMaterialBuffer()
{
	for (const auto& material : m_MaterialDatas)
	{
		auto matearialBuffSize = sizeof(Material);
		matearialBuffSize = (matearialBuffSize + 0xff) & ~0xff;

		auto materialBuffer = std::make_shared<Buffer>();
		materialBuffer->init(D3D12_HEAP_TYPE_UPLOAD, matearialBuffSize, D3D12_RESOURCE_STATE_GENERIC_READ);

		Material* mapMaterial = nullptr;
		HRESULT result = materialBuffer->getBuffer()->Map(0, nullptr, (void**)&mapMaterial);

		*((Material*)mapMaterial) = material.second.material;

		materialBuffer->getBuffer()->Unmap(0, nullptr);

		m_MaterialBuffers.emplace(material.first, materialBuffer);
	}



	return true;
}

bool MeshData::GenerateTextureBuffer()
{
	for (const auto& material : m_MaterialDatas)
	{
		// マテリアルにテクスチャ情報がない場合は、白テクスチャ
		if (material.second.texFilePath == "")
		{
			m_Textures.emplace(material.first, CreateWhiteTexture());
			continue;
		}

		std::shared_ptr<Texture> tex = std::make_shared<Texture>();
		ComPtr<ID3D12Resource> textureResource;
		tex->Init(StringUtil::GetWideStringFromString(material.second.texFilePath));

		D3D12_HEAP_PROPERTIES texHeapProp = {};
		texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;
		texHeapProp.CPUPageProperty =
			D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
		texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
		texHeapProp.CreationNodeMask = 0;
		texHeapProp.VisibleNodeMask = 0;


		D3D12_RESOURCE_DESC resDesc = {};
		resDesc.Format = tex->GetData().format;
		resDesc.Width = tex->GetData().width;
		resDesc.Height = tex->GetData().height;
		resDesc.DepthOrArraySize = tex->GetData().arraySize;

		resDesc.SampleDesc.Count = 1;
		resDesc.SampleDesc.Quality = 0;

		resDesc.MipLevels = tex->GetData().mipLevels;
		resDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(tex->GetData().dimension);
		resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		auto result = DirectXDevice::GetInstance().GetDevice()->CreateCommittedResource(
			&texHeapProp,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			nullptr,
			IID_PPV_ARGS(&textureResource)
		);


		m_Textures.emplace(material.first, textureResource);


		if (result != S_OK) continue;

		result = textureResource->WriteToSubresource(
			0,
			nullptr,
			tex->GetImage()->pixels,
			tex->GetImage()->rowPitch,
			tex->GetImage()->slicePitch
		);

	}

	return true;
}

void MeshData::CreateMeshAABB(const SimpleMath::Vector3& min, const SimpleMath::Vector3& max)
{
	SimpleMath::Vector3 center = (min + max) * 0.5f;

	m_MeshAABB.m_Min = min;
	m_MeshAABB.m_Max = max;
	m_MeshAABB.m_Center = center;

}

