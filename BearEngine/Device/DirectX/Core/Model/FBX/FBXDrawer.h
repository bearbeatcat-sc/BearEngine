#pragma once
#include "FBXModel.h"
#include "../../../../Singleton.h"
#include "../../../DirectXGraphics.h"
#include "../../PSOManager.h"
#include <map>
#include <memory>

class FBXDrawer
	:public Singleton<FBXDrawer>
{
public:
	friend class Singleton<FBXDrawer>;
	bool Init();
	bool Load(const char* filePath, std::string modelName);
	void Draw(std::string modelName, XMMATRIX mat);
protected:
	FBXDrawer();
	virtual ~FBXDrawer();


private:

	struct ConstBufferData
	{
		XMMATRIX mat;
	};
	std::shared_ptr<Buffer> m_ConstantBuffer;
	D3D12_CPU_DESCRIPTOR_HANDLE basicHeapHandle;
	ComPtr<ID3D12DescriptorHeap> basicDescHeap;

	bool generateConstantBuffer();
	bool initPSO();

private:
	PSO m_PSO;
	std::map<std::string,std::shared_ptr<FBXModel>> models;
};
