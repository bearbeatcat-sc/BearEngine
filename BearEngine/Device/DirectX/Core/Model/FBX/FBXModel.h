#pragma once
#include <DirectXMath.h>
#include <vector>
#include <d3d12.h>
#include <fbxsdk.h>
#include <map>
#include <memory>



using namespace DirectX;

class Buffer;
class FBX_Mesh;

class FBXModel
{
public:
	FBXModel();
	~FBXModel();
	bool load(const char* filePath);
	std::vector<std::shared_ptr<FBX_Mesh>> getMeshs();
private:
	void GenerateMeshDatas();
	void CollectMeshNode(FbxNode* node, std::map < std::string, FbxNode*>& list);

private:
	std::vector<std::shared_ptr<FBX_Mesh>> meshs;
	std::map<std::string, FbxNode*> nodes;

};