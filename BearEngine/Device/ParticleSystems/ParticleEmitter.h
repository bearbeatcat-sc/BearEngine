#ifndef _GPU_PARTICLE_EMITTER_H_
#define _GPU_PARTICLE_EMITTER_H_

#include <string>
#include <SimpleMath.h>
#include <d3d12.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <memory>

#include "../DirectX/Core/Model/MeshDrawer.h"
#include "../Texture.h"
#include "ParticleManager.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

class Buffer;
class Camera;
class Timer;
class Random;
class FluidParticleAction;
class NormalParticleAction;
class MeshData;

class ParticleEmitter
{
public:
	ParticleEmitter(const std::string& effectName, const std::string& actionName);
	ParticleEmitter(const std::string& effectName, const std::string& actionName,const std::string& modelName);
	~ParticleEmitter();

	enum DrawParticleMode
	{
		DrawParticleMode_Normal = 0,
		DrawParticleMode_BillBoard = 1,
		DrawParticleMode_BillBoard_Y = 2,
	};

	struct ParticleEmiiterData
	{
		XMFLOAT4 BasePosition;
		XMFLOAT4 BaseColor;
		XMFLOAT4 BaseVelocity;
		XMFLOAT4 BaseScale;
		XMFLOAT4 BaseRotate;
		float RandomSeed;
		float BaseLifeTime;
		float TimeScale;
		int MaxParticleCount;
	};

	struct ParticleData
	{
		XMFLOAT4 Position;
		XMFLOAT4 Velocity;
		XMFLOAT4 Rotate;
		XMFLOAT4 Color;
		XMFLOAT4 Scale;
		bool activeFlag = false;
		float generateTime;
		float lifeTime;
	};

	struct ConstBufferData
	{
		SimpleMath::Matrix billboard_matrix;
		SimpleMath::Matrix v_matrix;
		SimpleMath::Matrix p_matrix;
		SimpleMath::Matrix vp_matrix;
		SimpleMath::Vector3 nearFar;
	};

	struct Vertex
	{
		XMFLOAT4 color;
	};

	void SetParticleMode(DrawParticleMode mode);
	void SetTexture(const std::string& textureName);

	bool Init();
	bool Init(const std::vector<XMFLOAT3>& vertices);
	bool Init(const std::string& modelName);
	
	void Draw();
	
	void Destroy();
	void OnMeshParticleMode(bool flag,const std::string& meshName);
	bool IsDestroyFlag();
	void UpdateNormalParticle(std::shared_ptr<NormalParticleAction> action, ID3D12GraphicsCommandList* commandList);
	const std::string& GetActionName();
	int GetParticleCount();

	bool GetDrawFlag();
	bool GetUpdateFlag();

	
	void SetDrawFlag(bool flag);
	void SetUpdateFlag(bool flag);
	void SetPosition(const SimpleMath::Vector3& pos);
	void SetRotate(const SimpleMath::Vector3& rotate);
	void SetColor(const SimpleMath::Color& color);
	void SetScale(const SimpleMath::Vector3& scale);
	void SetVelocity(const SimpleMath::Vector3& velocity);
	void SetLifeTime(float lifeTime);
	void SetParticleCount(int count);
	void SetActionName(const std::string& actionName);
	void SetTimeScale(float timeScale);
	void SetHandle(ParticleManager::EmitterDescHandles* handle);
	void SetEffectName(const std::string& effectName);
	std::shared_ptr<Buffer> GetConstantBuffer();
	ComPtr<ID3D12Resource> GetTextureBuffer();
	ParticleManager::EmitterDescHandles* GetHandle();
	const std::string& GetEffectName();
	void GenerateTexture();
	DrawParticleMode GetDrawParticleMode();

private:

	// Normal
	void UpdateParticle(std::shared_ptr<NormalParticleAction> action, ID3D12GraphicsCommandList* commandList);
	void InitParticle(std::shared_ptr<ParticleAction> action, ID3D12GraphicsCommandList* commandList);

	bool DrawCall();
	void UpdateConstantBuffer();
	void UpdateEmitterBuffer();
	void GenerateVertexBuff();
	void GenerateConBuff();
	void GenerateComputeBuffer();
	void GenerateCBParticleData();

	void GenerateComputeHeap();
	void GenerateTextureView();
	void GenerateVerticesPositionBuffer(const std::vector<XMFLOAT3>& vertices);

private:

	D3D12_VERTEX_BUFFER_VIEW m_vbView;
	D3D12_INDEX_BUFFER_VIEW m_ibView;

	std::shared_ptr<Buffer> m_VertexBuffer;
	std::shared_ptr<Buffer> m_ConstantBuffer;
	ComPtr<ID3D12Resource> m_EmmiterParamsBuffer;

	ComPtr<ID3D12Resource> m_ParticleDataBuff0;
	ComPtr<ID3D12Resource> m_ParticleDataBuff1;
	ComPtr<ID3D12Resource> m_UploadConstBuff0;
	ComPtr<ID3D12Resource> m_UploadConstBuff1;

	ComPtr<ID3D12Resource> m_UploadVerticesDataBuff;
	ComPtr<ID3D12Resource> m_VerticesDataBuff;

	//ComPtr<ID3D12DescriptorHeap> m_BasicDescHeap;
	ComPtr<ID3D12DescriptorHeap> m_Heap;
	ComPtr<ID3D12Resource> m_TextureBuffer;

	enum DescriptorHeapIndex
	{
		UavParticlePosVelo0 = 0,
		UavParticlePosVelo1 = 1,
		
		SrvParticlePosVelo0 = 2,
		SrvParticlePosVelo1 = 3,

		SrvVerticesPosition = 4,
		EmitterData = 5,
		DescriptorCount = 6,
	};

	//enum RootParameterIndex
	//{
	//	RootParam_ParticleUpdateParam = 0,
	//	RootParam_SRV = 1,
	//	RootParam_UAV = 2,
	//	RootParam_VerticesPosition = 3,
	//	RootParam_EmiiterDataParam = 4,
	//};

	int m_ObjectCount;
	UINT m_UavIndex;

	std::shared_ptr<Camera> m_Camera;

private:

	std::string m_TextureName;
	bool m_IsUpdate;
	bool m_IsDraw;
	bool m_IsVerticesMode;
	bool m_DestroyFlag;
	bool m_DummyFlag;
	DrawParticleMode m_DrawPrticleMode;

	SimpleMath::Vector4 m_Pos;
	SimpleMath::Vector4 m_Rotate;
	SimpleMath::Color m_Color;
	SimpleMath::Vector4 m_Scale;
	SimpleMath::Vector4 m_Velocity;
	float m_BaseLifeTime;
	float m_TimeScale;

	std::string m_EffectName;
	ParticleManager::EmitterDescHandles* m_pHandle;
	//std::string m_ParticleUpdateComputeShader;
	//std::string m_ParticleInitComputeShader;

	bool m_IsGenerted;
	int m_VerticesCount;
	std::string m_ActionName;

	bool m_IsMeshEmitter;
	std::string m_UseMeshName;
	std::shared_ptr<MeshData> m_MeshData;

	bool m_IsMeshParticleMode;


};

#endif