#ifndef _MESH_DRAWER_H_
#define _MESH_DRAWER_H_

#include <vector>
#include <deque>
#include <memory>
#include <map>
#include <DirectXMath.h>

#include "../../../Singleton.h"
#include "../PSOManager.h"
#include "../MeshEffect.h"
#include "MeshDatas.h"

using namespace DirectX;

class Buffer;
class Mesh;
class Camera;
class DirectionalLight;

class MeshDrawer
	:public Singleton<MeshDrawer>

{
public:
	struct CPU_GPU_Handles
	{
		D3D12_CPU_DESCRIPTOR_HANDLE m_CPUDescHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_GPUDescHandle;
		bool m_UseFlag;
	};

	enum DrawType
	{
		Object,
		Wall,
	};

	friend class Singleton<MeshDrawer>;
	bool Init();
	void Update();
	void Draw();
	void CubeMapDraw(int index);
	
	void DrawDebug();
	void ShadowDraw();
	void AddObjMesh(std::shared_ptr<Mesh> mesh);

protected:
	MeshDrawer();
	~MeshDrawer();

private:
	bool InitMateralHeaps();
	bool InitConstantHeaps();
	bool GenerateHandles();
	bool GenerateShadowPSO();
	bool GenerateViews(std::shared_ptr<Mesh> mesh);
	bool GenerateConstantView(std::shared_ptr<Mesh> mesh);
	bool GenerateMaterialView(std::shared_ptr<Mesh> mesh);
	void Delete();

	// 視錐台カリング用
	const std::vector<std::shared_ptr<Mesh>> FrustumCulling();
	bool IsInSideFrustum(const MeshData::MeshAABB& meshAABB);

private:
	std::vector<std::shared_ptr<Mesh>> m_meshs;
	std::shared_ptr<Camera> m_Camera;

	// Handle
	std::vector<MeshDrawer::CPU_GPU_Handles*> m_MatHandles;
	std::vector<MeshDrawer::CPU_GPU_Handles*> m_ConstantHandles;

	PSO m_ShadowPSO; // 影描画用

	ComPtr<ID3D12DescriptorHeap> m_BasicDescHeap;
	ComPtr<ID3D12DescriptorHeap> m_MaterialDescHeap;

	const int m_MaterialCount = 6;
	const int m_ContanstBuffCount = 1;
	const int m_CBVAndSRV = 2;

	enum RootParamterIndex
	{
		RootParamterIndex_Matrix = 0,
		RootParamterIndex_Material = 1,
		RootParamterIndex_ShadowTex = 2,
		RootParamterIndex_Light = 3,
		RootParamterIndex_CubeMapMatrix  = 4,
	};

	std::shared_ptr<DirectionalLight> m_DirLight;

	int m_DescHandleCount;
	int m_MatIncSize;

	int m_ObjectCount = 128 * 100;

	double m_CurrentDrawTime;
	int m_CurrentVisibleMeshCount;

};


#endif