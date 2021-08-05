#include "Mesh.h"
#include "Device/DirectX/Core/Buffer.h"
#include "Device/DirectX/DirectXDevice.h"
#include "Device/DirectX/DirectXGraphics.h"
#include "Utility/Camera.h"
#include "Device/Texture.h"
#include "MeshDatas.h"
#include "Device/Lights/LightManager.h"
#include "Device/Lights/DirectionalLight.h"
#include "Utility/CameraManager.h"
#include "Utility/Time.h"

Mesh::Mesh(std::shared_ptr<MeshData> meshData, const std::string& effectName)
	:m_MeshDatas(meshData), m_EffectName(effectName), m_IsVisible(true),m_DestroyFlag(false),
	m_GeneratedTime(0.0f), m_Color(XMFLOAT4(1,1,1,1)), m_WorldMat(SimpleMath::Matrix::Identity)
{
	m_Camera = CameraManager::GetInstance().GetMainCamera();
	m_Light = LightManager::GetInstance().GetDirectionalLight();
	m_LightProjMat = XMMatrixOrthographicLH(40, 40, 1.0f, 100.0f);
	GenerateConstantBuffer();
	//GenerateCubeMapConstantBuffer();
}

Mesh::Mesh(std::shared_ptr<MeshData> meshData, const std::string& effectName, const std::string& cameraName)
	:m_MeshDatas(meshData), m_EffectName(effectName), m_IsVisible(true), m_DestroyFlag(false),
	m_GeneratedTime(0.0f), m_Color(XMFLOAT4(1, 1, 1, 1)), m_WorldMat(SimpleMath::Matrix::Identity)

{
	m_Camera = CameraManager::GetInstance().GetCamera(cameraName);
	m_Light = LightManager::GetInstance().GetDirectionalLight();
	m_LightProjMat = XMMatrixOrthographicLH(40, 40, 1.0f, 100.0f);
	GenerateConstantBuffer();
	//GenerateCubeMapConstantBuffer();
}

Mesh::~Mesh()
{

}


bool Mesh::SetBuffer(ID3D12GraphicsCommandList* cmd)
{
	UpdateConstBuff();

	cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmd->IASetVertexBuffers(0, 1, &m_MeshDatas->m_vbView);
	cmd->IASetIndexBuffer(&m_MeshDatas->m_ibView);

	return true;
}

bool Mesh::SetBuffer(ID3D12GraphicsCommandList* cmd, int index, const SimpleMath::Matrix& view, const SimpleMath::Matrix& proj)
{
	UpdateConstBuff(index,view,proj);

	cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmd->IASetVertexBuffers(0, 1, &m_MeshDatas->m_vbView);
	cmd->IASetIndexBuffer(&m_MeshDatas->m_ibView);

	return true;
}

bool Mesh::SetBuffer(ID3D12GraphicsCommandList* cmd, int index)
{
	UpdateConstBuff(index);

	cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmd->IASetVertexBuffers(0, 1, &m_MeshDatas->m_vbView);
	cmd->IASetIndexBuffer(&m_MeshDatas->m_ibView);

	return true;
}

void Mesh::Destroy()
{
	m_DestroyFlag = true;
}

void Mesh::SetDrawType(MeshDrawer::DrawType drawType)
{
	m_DrawType = drawType;
}

MeshDrawer::DrawType Mesh::GetDrawType()
{
	return m_DrawType;
}

void Mesh::SetMatrix(XMMATRIX mat)
{
	m_WorldMat = mat;
}

void Mesh::SetColor(XMFLOAT4 color)
{
	m_Color = color;
}

void Mesh::SetCamera(const std::string& cameraName)
{
	m_Camera = CameraManager::GetInstance().GetCamera(cameraName);
}

const std::string& Mesh::GetEffectName()
{
	return m_EffectName;
}

void Mesh::SetVisible(bool flag)
{
	m_IsVisible = flag;
}

bool Mesh::IsVisible()
{
	return m_IsVisible;
}

void Mesh::SetEffectName(const std::string& effectName)
{
	m_EffectName = effectName;
	m_GeneratedTime = 0.0f;
}

bool Mesh::GetDestroyFlag()
{
	return m_DestroyFlag;
}


std::shared_ptr<Buffer> Mesh::GetConstantBuffer()
{
	return m_ConstDataBuffer;
}

const std::vector<std::shared_ptr<Buffer>>& Mesh::GetCubeMapConstantBuffers()
{
	return m_CubeMapConstantBuffer;
}

std::unordered_map<std::string, std::shared_ptr<Buffer>>& Mesh::GetMaterialBuffer()
{
	return m_MeshDatas->GetMaterialBuffer();
}

std::unordered_map<std::string, ComPtr<ID3D12Resource>>& Mesh::GetTextureBuff()
{
	return m_MeshDatas->GetTextureBuff();
}

std::vector<MeshData::MaterialData>& Mesh::GetDrawMaterialDatas()
{
	return m_MeshDatas->GetDrawMaterialDatas();
}

std::unordered_map<std::string, MeshData::MaterialData>& Mesh::GetMaterialDatas()
{
	return m_MeshDatas->GetMaterialDatas();
}

ID3D12Resource* Mesh::GetWhiteTextureBuff()
{
	return m_MeshDatas->GetWhiteTextureBuff();
}

MeshDrawer::CPU_GPU_Handles* Mesh::GetConstantDescHandle()
{
	return m_ConstantDescHandle;
}

MeshDrawer::CPU_GPU_Handles* Mesh::GetMatDescHandle()
{
	return m_MatDescHandle;
}

void Mesh::SetConstantDescHandle(MeshDrawer::CPU_GPU_Handles* handle)
{
	m_ConstantDescHandle = handle;
}

void Mesh::SetMatDescHandle(MeshDrawer::CPU_GPU_Handles* handle)
{
	m_MatDescHandle = handle;
}

const MeshData::MeshAABB Mesh::GetMeshAABB()
{
	auto meshAABB = m_MeshDatas->GetMeshAABB();
	auto min = meshAABB.m_Min;
	auto max = meshAABB.m_Max;
	auto center = meshAABB.m_Center;

	// ÉèÅ[ÉãÉhçsóÒÇ≈ïœä∑
	auto temp_min = SimpleMath::Vector3::Transform(min, m_WorldMat);
	auto temp_max = SimpleMath::Vector3::Transform(max, m_WorldMat);
	auto temp_center = SimpleMath::Vector3::Transform(center, m_WorldMat);

	MeshData::MeshAABB retAABB;
	retAABB.m_Min = temp_min;
	retAABB.m_Max = temp_max;
	retAABB.m_Center = temp_center;

	return retAABB;
}

bool Mesh::GenerateConstantBuffer()
{
	m_ConstDataBuffer = std::make_shared<Buffer>();
	UINT buffSize = (sizeof(m_ConstDataBuffer) + 0xff) & ~0xff;
	m_ConstDataBuffer->init(D3D12_HEAP_TYPE_UPLOAD, buffSize, D3D12_RESOURCE_STATE_GENERIC_READ);
	auto constBuff = m_ConstDataBuffer->getBuffer();

	ConstDatas* constMap = nullptr;
	constBuff->Map(0, nullptr, (void**)&constMap);
	constMap->ViewProj = XMMatrixIdentity();
	constMap->Color = m_Color;
	constBuff->Unmap(0, nullptr);

	return true;
}

bool Mesh::GenerateCubeMapConstantBuffer()
{
	m_CubeMapConstantBuffer.resize(6);

	for(int i = 0; i < 6; ++i)
	{
		m_CubeMapConstantBuffer[i] = std::make_shared<Buffer>();
		UINT buffSize = (sizeof(m_CubeMapConstantBuffer[i]) + 0xff) & ~0xff;
		m_CubeMapConstantBuffer[i]->init(D3D12_HEAP_TYPE_UPLOAD, buffSize, D3D12_RESOURCE_STATE_GENERIC_READ);
		auto constBuff = m_CubeMapConstantBuffer[i]->getBuffer();

		ConstDatas* constMap = nullptr;
		constBuff->Map(0, nullptr, (void**)&constMap);
		constMap->ViewProj = XMMatrixIdentity();
		constMap->Color = m_Color;
		constBuff->Unmap(0, nullptr);
	}
	


	return true;
}


void Mesh::UpdateConstBuff()
{
	m_VPMat = m_Camera->GetViewMat() * m_Camera->GetProjectMat();


	SimpleMath::Vector3 lightDir = m_Light->GetDirection();

	auto cameraPos = m_Camera->GetPosition();
	auto cameraTarget = m_Camera->GetTarget();

	auto lightPos = cameraTarget + lightDir * (cameraTarget - cameraPos).Length();

	DirectX::SimpleMath::Matrix lightView = XMMatrixLookAtLH(
		lightPos, cameraTarget, m_Camera->GetUp());


	//auto shadowMat = lightView * m_LightProjMat;
	auto shadowMat = lightView * m_LightProjMat;

	auto eyePos = m_Camera->GetPosition();


	////float lightScale = 1.5f;
	////XMMATRIX lightView, lightProj;
	////lightProj = XMMatrixPerspectiveFovLH(45, 1.0f, 40.0f, 300.0f);
	////lightView = XMMatrixLookAtLH(lightPos * lightScale,
	////	cameraTarget, m_Camera->GetUp());


	m_GeneratedTime += Time::DeltaTime;


	ConstDatas* constMap = nullptr;
	HRESULT result;

	auto buff = m_ConstDataBuffer->getBuffer();
	result = buff->Map(0, nullptr, (void**)&constMap);	
	constMap->ShadowMat = m_WorldMat * shadowMat;
	constMap->World = m_WorldMat;
	constMap->ViewProj =  m_VPMat;
	constMap->Color = m_Color;
	constMap->Time = m_GeneratedTime;
	constMap->EyePos = SimpleMath::Vector4(eyePos.x, eyePos.y, eyePos.z, 0);
	buff->Unmap(0, nullptr);

}

void Mesh::UpdateConstBuff(int index, const SimpleMath::Matrix& view, const SimpleMath::Matrix& proj)
{
	m_VPMat = view * proj;


	SimpleMath::Vector3 lightDir = m_Light->GetDirection();

	auto cameraPos = m_Camera->GetPosition();
	auto cameraTarget = m_Camera->GetTarget();

	auto lightPos = cameraTarget + lightDir * (cameraTarget - cameraPos).Length();

	DirectX::SimpleMath::Matrix lightView = XMMatrixLookAtLH(
		lightPos, cameraTarget, m_Camera->GetUp());


	//auto shadowMat = lightView * m_LightProjMat;
	auto shadowMat = lightView * m_LightProjMat;

	auto eyePos = m_Camera->GetPosition();


	////float lightScale = 1.5f;
	////XMMATRIX lightView, lightProj;
	////lightProj = XMMatrixPerspectiveFovLH(45, 1.0f, 40.0f, 300.0f);
	////lightView = XMMatrixLookAtLH(lightPos * lightScale,
	////	cameraTarget, m_Camera->GetUp());


	m_GeneratedTime += Time::DeltaTime;


	ConstDatas* constMap = nullptr;
	HRESULT result;

	auto buff = m_CubeMapConstantBuffer[index]->getBuffer();
	result = buff->Map(0, nullptr, (void**)&constMap);
	constMap->ShadowMat = m_WorldMat * shadowMat;
	constMap->World = m_WorldMat;
	constMap->ViewProj = m_VPMat;
	constMap->Color = m_Color;
	constMap->Time = m_GeneratedTime;
	constMap->EyePos = SimpleMath::Vector4(eyePos.x, eyePos.y, eyePos.z, 0);
	buff->Unmap(0, nullptr);
}

void Mesh::UpdateConstBuff(int index)
{
	m_VPMat = m_Camera->GetViewMat() * m_Camera->GetProjectMat();


	SimpleMath::Vector3 lightDir = m_Light->GetDirection();

	auto cameraPos = m_Camera->GetPosition();
	auto cameraTarget = m_Camera->GetTarget();

	auto lightPos = cameraTarget + lightDir * (cameraTarget - cameraPos).Length();

	DirectX::SimpleMath::Matrix lightView = XMMatrixLookAtLH(
		lightPos, cameraTarget, m_Camera->GetUp());


	//auto shadowMat = lightView * m_LightProjMat;
	auto shadowMat = lightView * m_LightProjMat;

	auto eyePos = m_Camera->GetPosition();


	////float lightScale = 1.5f;
	////XMMATRIX lightView, lightProj;
	////lightProj = XMMatrixPerspectiveFovLH(45, 1.0f, 40.0f, 300.0f);
	////lightView = XMMatrixLookAtLH(lightPos * lightScale,
	////	cameraTarget, m_Camera->GetUp());


	m_GeneratedTime += Time::DeltaTime;


	ConstDatas* constMap = nullptr;
	HRESULT result;

	auto buff = m_CubeMapConstantBuffer[index]->getBuffer();
	result = buff->Map(0, nullptr, (void**)&constMap);
	constMap->ShadowMat = m_WorldMat * shadowMat;
	constMap->World = m_WorldMat;
	constMap->ViewProj = m_VPMat;
	constMap->Color = m_Color;
	constMap->Time = m_GeneratedTime;
	constMap->EyePos = SimpleMath::Vector4(eyePos.x, eyePos.y, eyePos.z, 0);
	buff->Unmap(0, nullptr);

}

