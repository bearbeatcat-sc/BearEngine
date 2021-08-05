#include "FBXModel.h"
#include "../../Buffer.h"
#include "FBXMesh.h"

FBXModel::FBXModel()
{
}

FBXModel::~FBXModel()
{
}

bool FBXModel::load(const char* filePath)
{
	auto fbxManager = FbxManager::Create();
	auto fbxScene = FbxScene::Create(fbxManager, "fbxscene");

	FbxString FileName(filePath);

	// FBXのインポート
	FbxImporter *fbxImporter = FbxImporter::Create(fbxManager, "imp");
	fbxImporter->Initialize(FileName.Buffer(), -1, fbxManager->GetIOSettings());

	fbxImporter->Import(fbxScene);
	fbxImporter->Destroy();



	// DirectX用に変換
	FbxAxisSystem SceneAxisSystem = fbxScene->GetGlobalSettings().GetAxisSystem();

	if (SceneAxisSystem != FbxAxisSystem::DirectX)
	{
		FbxAxisSystem::DirectX.ConvertScene(fbxScene);
	}

	CollectMeshNode(fbxScene->GetRootNode(), nodes);

	//FbxGeometryConverter converter(fbxManager);
	//converter.Triangulate(fbxScene, true);

	GenerateMeshDatas();

	return true;
}

std::vector<std::shared_ptr<FBX_Mesh>> FBXModel::getMeshs()
{
	return meshs;
}


void FBXModel::GenerateMeshDatas()
{

	std::vector<FbxMesh*> meshDatas;
	int count = 0;

	for (auto node :nodes)
	{
		std::shared_ptr<FBX_Mesh> temp = std::shared_ptr<FBX_Mesh>(new FBX_Mesh(static_cast<FbxMesh*>(node.second->GetNodeAttribute()), node.second));

		meshs.push_back(temp);
		count += temp->getVertexCount();
	}

}

void FBXModel::CollectMeshNode(FbxNode * node, std::map<std::string,FbxNode*>& list)
{
	int test = node->GetNodeAttributeCount();

	for (int i = 0; i < node->GetNodeAttributeCount(); i++)
	{
		FbxNodeAttribute* attribute = node->GetNodeAttributeByIndex(i);

		if (attribute->GetAttributeType() == FbxNodeAttribute::EType::eMesh)
		{
			list[node->GetName()] = node;
			break;
		}


	}

	for (int i = 0; i < node->GetChildCount(); i++)
	{
		CollectMeshNode(node->GetChild(i), list);
	}
}

