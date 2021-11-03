#include "MeshManager.h"
#include "OBJ/ObjLoader.h"
#include "PMD/PMDLoader.h"
#include "GeomtryGenerater.h"
#include "Device/DirectX/DirectXDevice.h"
#include "Device/DirectX/Core/Buffer.h"
#include "Device/Raytracing/DXRPipeLine.h"

MeshManager::MeshManager()
{


}

MeshManager::~MeshManager()
{
	m_ModelDatas.clear();
}


bool MeshManager::Init()
{
	m_ModelDatas.clear();

	// キューブメッシュ用のデータを生成
	CreateCubeMeshData();

	return true;
}

bool MeshManager::loadMesh(const std::string& filePath, const std::string& fileName, const std::string& modelName)
{
	if (fileName.find("obj") != std::string::npos)
	{
		LoadObj(filePath, fileName, modelName);
		return true;
	}

	if (fileName.find("pmd") != std::string::npos)
	{
		LoadPmd(filePath + fileName, modelName);
		return true;
	}
}

std::shared_ptr<Mesh> MeshManager::GetMesh(const std::string& modelname, const std::string& effectName)
{
	auto model = m_ModelDatas.at(modelname);

	//TODO :　std::make_sharedを使うとメッシュが表示されない？
	std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(model, effectName);

	return mesh;
}

std::shared_ptr<Mesh> MeshManager::GetCubeMesh(const std::string& effectName)
{
	//MeshData::ModelData model;
	//GeometryGenerator::GenerateCubeMesh(model,size);
	//std::shared_ptr <MeshData> data = std::make_shared<MeshData>();
	//data->GenerateMesh(model.vertices, model.m_Indices, model.m_MaterialDatas);

	std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(m_CubeModelData, effectName);
	return mesh;
}

std::shared_ptr<Mesh> MeshManager::GetSpehereMesh(int tesselation, const std::string& effectName)
{
	auto result = FindSpehere(tesselation);

	// すでに同じ分割数のSPHEREがある場合は、再利用
	if (result == nullptr)
	{
		MeshData::ModelData model;
		GeometryGenerator::GenerateSpehereMesh(model, 1.0f, tesselation);


		std::shared_ptr <MeshData> data = std::make_shared<MeshData>();
		data->GenerateMesh(model.vertices, model.m_Indices, model.m_MaterialDatas);
		m_SpehereModelDatas.emplace(tesselation, data);
		
		//DXRPipeLine::GetInstance().CreateResourceView(data);
		result = data;
	}

	//MeshData::ModelData model;
	// GeometryGenerator::GenerateSpehereMesh(model,1.0f, tesselation);
	//std::shared_ptr <MeshData> data = std::make_shared<MeshData>();
	//data->GenerateMesh(model.vertices, model.m_Indices, model.m_MaterialDatas);
	//std::shared_ptr<Mesh> mesh = std::shared_ptr<Mesh>(new Mesh(result, effectName));
	std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(result, effectName);
	return mesh;
}

std::shared_ptr<Mesh> MeshManager::GetTriangleMesh(const SimpleMath::Vector3* points, const std::string& effectName)
{
	MeshData::ModelData model;
	GeometryGenerator::GenerateTriangleMesh(model, points);
	std::shared_ptr <MeshData> data = std::make_shared<MeshData>();
	data->GenerateMesh(model.vertices, model.m_Indices, model.m_MaterialDatas);

	std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(data, effectName);
	return mesh;
}

std::shared_ptr<Mesh> MeshManager::GetPlaneMesh(const std::string& effectName)
{
	MeshData::ModelData model;
	GeometryGenerator::GenerateSquareMesh(model);
	std::shared_ptr <MeshData> data = std::make_shared<MeshData>();
	data->GenerateMesh(model.vertices, model.m_Indices, model.m_MaterialDatas);

	std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(data, effectName);
	return mesh;
}

const std::shared_ptr<MeshData> MeshManager::GetMeshData(const std::string& modelname)
{
	auto result = m_ModelDatas.find(modelname);

	if (result == m_ModelDatas.end())
	{
		return nullptr;
	}

	return m_ModelDatas.at(modelname);
}

bool MeshManager::LoadObj(const std::string& filePath, const std::string& fileName, const std::string& modelName, bool isSmooth)
{
	MeshData::ModelData model;
	ObjLoader::GetInstance().loadObj(model, filePath, fileName);
	std::shared_ptr <MeshData> data = std::make_shared<MeshData>();

	// スムースモードなら法線情報の平均化を行う
	//if (isSmooth)
	//{

	//}

	data->GenerateMesh(model.vertices, model.m_Indices, model.m_MaterialDatas);
	m_ModelDatas.emplace(modelName, data);
	//DXRPipeLine::GetInstance().CreateResourceView(data);
	
	return true;
}

bool MeshManager::LoadPmd(const std::string& filePath, const std::string& modelName)
{
	//Mesh::ModelData data = PMDLoader::GetInstance().LoadPMD(filePath, modelName);
	//m_ModelDatas.emplace(modelName, data);

	//return true;

	return false;
}

void MeshManager::SmoothNormal(MeshData::ModelData& meshData)
{

}

std::shared_ptr<MeshData> MeshManager::FindSpehere(int tesselation)
{
	auto result = m_SpehereModelDatas.find(tesselation);

	if (result == m_SpehereModelDatas.end())
	{
		return nullptr;
	}

	return m_SpehereModelDatas.at(tesselation);
}

void MeshManager::CreateCubeMeshData()
{
	MeshData::ModelData model;
	GeometryGenerator::GenerateCubeMesh(model, SimpleMath::Vector3(1.0f, 1.0f, 1.0f));
	m_CubeModelData = std::make_shared<MeshData>();
	m_CubeModelData->GenerateMesh(model.vertices, model.m_Indices, model.m_MaterialDatas);

	m_ModelDatas.emplace("CubeModelData", m_CubeModelData);

	//DXRPipeLine::GetInstance().CreateResourceView(m_CubeModelData);
}



//bool MeshManager::LoadPrimitive(MeshData::ModelData model, std::string modelName)
//{
//	MeshData* data = new MeshData();
//	data->GenerateMesh(model.vertices, model.indices, model.materialData);
//	m_ModelDatas.emplace(modelName, data);
//
//	return true;
//}

