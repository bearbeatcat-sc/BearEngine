#ifndef _GEOMETORY_GENEREATOR
#define _GEOMETORY_GENEREATOR

#include "SimpleMath.h"
#include "../Model/MeshDatas.h"



class GeometryGenerator 
{
public:
	
	//struct MeshDatas
	//{
	//	std::vector<MeshData::Vertex> vertcies;
	//	std::vector<unsigned short> indices;
	//};

	static void GenerateCubeMesh(MeshData::ModelData& modelData, DirectX::SimpleMath::Vector3 size);
	static void GenerateCubeMeshData(MeshData::ModelData& modelData, DirectX::SimpleMath::Vector3 size);
	static void GenerateSphereMeshData(MeshData::ModelData& modelData, float diametro, size_t tessellation);
	
	static void GenerateSquareMeshData(MeshData::ModelData& modelData);
	static void GenerateTriangleMeshData(MeshData::ModelData& modelData, const DirectX::SimpleMath::Vector3* points);
	
	static void GenerateCubeDatas(std::vector<XMFLOAT3>& positions, std::vector<UINT>& indices, DirectX::SimpleMath::Vector3 size);

private:
	static void ReverseWinding(std::vector<UINT>& indicies, std::vector<MeshData::Vertex>& vertcies);
	static void ReverseWinding(std::vector<UINT>& indicies);

};

#endif

