#pragma once

#include <d3d12.h>
#include <string>
#include <wrl/client.h>
#include "DXRPipeLine.h"
#include <Device/TextureManager.h>


struct DXRMeshData
{
public:
	DXRMeshData(const std::wstring& hitGroupName,const PhysicsBaseMaterial& mat)
		:_hitGroupName(hitGroupName),_mat(mat)
	{
		_texture = TextureManager::GetInstance().GetWhiteTex();
	};

	DXRMeshData(const std::wstring& hitGroupName, const PhysicsBaseMaterial& mat,const std::string& textureName)
		:_hitGroupName(hitGroupName), _mat(mat)
	{
		_texture = TextureManager::GetInstance().GetTexture(textureName);
	};
	
	~DXRMeshData() = default;

	void UpdateMaterial(const PhysicsBaseMaterial& mat)
	{
		_mat = mat;
		DXRPipeLine::GetInstance().OnUpdateMaterial();
	}
	
	D3D12_GPU_DESCRIPTOR_HANDLE _vbView;
	D3D12_GPU_DESCRIPTOR_HANDLE _ibView;
	D3D12_GPU_DESCRIPTOR_HANDLE _textureView;

	// DXR用
	Microsoft::WRL::ComPtr<ID3D12Resource> _blas;
	ComPtr<ID3D12Resource> _texture;
	std::shared_ptr<AccelerationStructureBuffers> _asBuffer;
	std::wstring _hitGroupName;
	int _hitGropIndex;
	std::string _meshName;
	PhysicsBaseMaterial _mat;
};
