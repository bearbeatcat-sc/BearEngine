#ifndef _MESH_H_
#define _MESH_H_

#include <unordered_map>
#include <d3d12.h>
#include <DirectXMath.h>
#include <memory>
#include <SimpleMath.h>

#include "../PSOManager.h"
#include "MeshDrawer.h"
#include "MeshDatas.h"

using namespace DirectX;

class Buffer;
class Camera;
class MeshData;
class Light;


class Mesh
{
public:
	Mesh(std::shared_ptr<MeshData> meshData,const std::string& effectName);
	Mesh(std::shared_ptr<MeshData> meshData,const std::string& effectName,const std::string& cameraName);
	virtual ~Mesh();
	bool SetBuffer(ID3D12GraphicsCommandList* cmd);
	bool SetBuffer(ID3D12GraphicsCommandList* cmd,int index,const SimpleMath::Matrix& view, const SimpleMath::Matrix& proj);
	bool SetBuffer(ID3D12GraphicsCommandList* cmd,int index);
	void Destroy();
	
	void SetDrawType(MeshDrawer::DrawType drawType);
	MeshDrawer::DrawType GetDrawType();
	
	void SetMatrix(XMMATRIX mat);
	void SetColor(XMFLOAT4 color);
	void SetCamera(const std::string& cameraName);
	const std::string& GetEffectName();
	void SetVisible(bool flag);
	bool IsVisible();
	void SetEffectName(const std::string& effectName);
	bool GetDestroyFlag();
	std::shared_ptr<Buffer> GetConstantBuffer();
	const std::vector<std::shared_ptr<Buffer>>& GetCubeMapConstantBuffers();
	std::unordered_map<std::string, std::shared_ptr<Buffer>>& GetMaterialBuffer();
	std::unordered_map<std::string, ComPtr<ID3D12Resource>>& GetTextureBuff();
	std::vector<MaterialData>& GetDrawMaterialDatas();
	std::unordered_map<std::string, MaterialData>& GetMaterialDatas();
	ID3D12Resource* GetWhiteTextureBuff();

	std::shared_ptr<Buffer> GetIndexBuffer();
	std::shared_ptr<Buffer> GetVertexBuffer();
	
	MeshDrawer::CPU_GPU_Handles* GetConstantDescHandle();
	MeshDrawer::CPU_GPU_Handles* GetMatDescHandle();
	void SetConstantDescHandle(MeshDrawer::CPU_GPU_Handles* handle);
	void SetMatDescHandle(MeshDrawer::CPU_GPU_Handles* handle);
	const MeshData::MeshAABB GetMeshAABB();
	const int GetIndexCount();

protected:
	bool GenerateConstantBuffer();
	bool GenerateCubeMapConstantBuffer();
	virtual void UpdateConstBuff();
	virtual void UpdateConstBuff(int index,const SimpleMath::Matrix& view, const SimpleMath::Matrix& proj);
	virtual void UpdateConstBuff(int index);
	

private:

	struct ConstDatas
	{
		DirectX::SimpleMath::Matrix World;
		DirectX::SimpleMath::Matrix ViewProj;
		DirectX::SimpleMath::Matrix ShadowMat;
		XMFLOAT4 Color;
		XMFLOAT4 EyePos;
		float Time = 0.0f;
	};



	std::shared_ptr<Buffer> m_ConstDataBuffer;

	std::vector<std::shared_ptr<Buffer>> m_CubeMapConstantBuffer;
	std::shared_ptr<MeshData> m_MeshDatas;

	DirectX::SimpleMath::Matrix m_WorldMat;
	DirectX::SimpleMath::Matrix m_VPMat;
	DirectX::SimpleMath::Matrix m_LightProjMat;

	XMFLOAT4 m_Color;
	std::shared_ptr<DirectionalLight> m_Light;
	std::shared_ptr<Camera> m_Camera;
	std::string m_EffectName;

	bool m_DestroyFlag;
	bool m_IsVisible;

	MeshDrawer::CPU_GPU_Handles* m_ConstantDescHandle;
	MeshDrawer::CPU_GPU_Handles* m_MatDescHandle;

	float m_GeneratedTime;

	MeshDrawer::DrawType m_DrawType;

};

#endif