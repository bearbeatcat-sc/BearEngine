#pragma once

#include "../../../../../Device/Singleton.h"

#include <unordered_map>
#include "../MeshDatas.h"

using namespace DirectX;


class ObjLoader
	:public Singleton<ObjLoader>
{
public:
	friend class Singleton<ObjLoader>;
	void loadObj(MeshData::ModelData& modelData, const std::string& filePath, const std::string& fileName);
	std::unordered_map<std::string,MaterialData> loadMtl(const std::string& filePath, const std::string& fileName);

protected:
	ObjLoader();
	~ObjLoader();


};