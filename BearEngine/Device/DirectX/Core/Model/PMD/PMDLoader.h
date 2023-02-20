#ifndef _PMD_LOADER_H_
#define _PMD_LOADER_H_

#include "../../../../Singleton.h"
#include <map>
#include <DirectXMath.h>
#include "../MeshDatas.h"
#include <memory>
#include <vector>
#include <string>

using namespace DirectX;

class PMDLoader
	:public Singleton<PMDLoader>
{
public:
	struct PMDHeader
	{
		float verson;
		char model_name[20];
		char comment[256];
	};

	struct PMDVetex
	{
		XMFLOAT3 pos;
		XMFLOAT3 normal;
		XMFLOAT2 uv;
		unsigned short boneNo[2];
		unsigned char boneWeight;
		unsigned char edgeFlag;
	};
	friend class Singleton<PMDLoader>;
	MeshData::ModelData LoadPMD(std::string filePath, std::string modelName);


private:
	PMDLoader();
	~PMDLoader();
};

#endif