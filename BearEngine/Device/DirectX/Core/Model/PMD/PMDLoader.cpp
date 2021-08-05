#include "PMDLoader.h"
#pragma warning(disable: 4996)



PMDLoader::PMDLoader()
{
}

PMDLoader::~PMDLoader()
{
}

MeshData::ModelData PMDLoader::LoadPMD(std::string filePath, std::string modelName)
{
	// シグネチャとヘッダー情報を読み取る
	char signature[3] = {};
	auto fp = fopen((filePath).c_str(), "rb");
	PMDHeader pmdHeader;

	if (!fp)return MeshData::ModelData{};

	fread(signature, sizeof(signature), 1, fp);
	fread(&pmdHeader, sizeof(pmdHeader), 1, fp);

	constexpr size_t pmdVertex_Size = 38;

	unsigned int vertNum;
	fread(&vertNum, sizeof(vertNum), 1, fp);

	std::vector<PMDVetex> vertcies;
	vertcies.resize(vertNum);

	//std::vector<unsigned char> vertices(vertNum * pmdVertex_Size);
	//fread(vertices.data(), vertices.size(), 1, fp);

	for (int i = 0; i < vertNum; ++i)
	{
		fread(&vertcies[i], sizeof(PMDVetex), 1, fp);
	}


	MeshData::Vertex vertex;

	std::vector<MeshData::Vertex> verts;

	for (int i = 0; i < vertcies.size(); i++)
	{
		MeshData::Vertex vert;
		vert.pos = vertcies[i].pos;
		vert.normal = vertcies[i].normal;
		verts.push_back(vert);
	}



	std::vector<unsigned short> indices;
	unsigned int indicesNum;
	fread(&indicesNum, sizeof(indicesNum), 1, fp);

	indices.resize(indicesNum);
	fread(indices.data(), indices.size() * sizeof(indices[0]), 1, fp);

	//MeshData* mesh = new MeshData();
	//mesh->GenerateMesh(verts, indices, MeshData::MaterialData{});

	MeshData::ModelData modelData;

	modelData.vertices = verts;
	//modelData.indices = indices;

	return modelData;
}
