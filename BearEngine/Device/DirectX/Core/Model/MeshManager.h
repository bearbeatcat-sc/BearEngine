#ifndef _MESH_MANAGER_H_
#define _MESH_MANAGER_H_

#include <map>
#include <DirectXMath.h>
#include <memory>
#include <string>
#include <SimpleMath.h>

#include "Mesh.h"
#include "MeshDatas.h"
#include "MeshDrawer.h"
#include "../../../Singleton.h"


class MeshManager
	:public Singleton<MeshManager>

{
public:
	friend class Singleton<MeshManager>;

	bool Init();
	bool loadMesh(const std::string& filePath, const std::string& fileName, const std::string& modelName);
	std::shared_ptr<Mesh> GetMesh(const std::string& modelname, const std::string& effectName);
	//bool LoadPrimitive(MeshData::ModelData model, std::string modelName);
	std::shared_ptr<Mesh> GetCubeMesh(const std::string& effectName);
	std::shared_ptr<Mesh> GetSpehereMesh(int tesselation, const std::string& effectName);
	std::shared_ptr<Mesh> GetTriangleMesh(const SimpleMath::Vector3* points, const std::string& effectName);	
	std::shared_ptr<Mesh> GetPlaneMesh(const std::string& effectName);
	
	const std::shared_ptr<MeshData> GetMeshData(const std::string& modelname);
	const std::shared_ptr<MeshData> GetSpehereMeshData(const int tesselation);
	const std::shared_ptr<MeshData> GetPlaneMeshData();

	std::shared_ptr<MeshData> FindSpehere(int tesselation);
	ID3D12DescriptorHeap* GetDescriptrHeap();


protected:


	bool LoadObj(const std::string& filePath, const std::string& fileName, const std::string& modelName,bool isSmooth = false);
	bool LoadPmd(const std::string& filePath, const std::string& modelName);
	void SmoothNormal(MeshData::ModelData& meshData);
	void CreateCubeMeshData();
	void CreatePlaneMeshData();
	void CreateDescriptorHeap();


	MeshManager();
	~MeshManager();
	std::map<std::string, std::shared_ptr<MeshData>> m_ModelDatas;

	// 各種、プリミティブデータ
	std::map<int, std::shared_ptr<MeshData>> m_SpehereModelDatas;
	std::shared_ptr<MeshData> m_CubeModelData;
	std::shared_ptr<MeshData> m_TriangleModelData;
	std::shared_ptr<MeshData> m_PlaneModelData;
};

#endif