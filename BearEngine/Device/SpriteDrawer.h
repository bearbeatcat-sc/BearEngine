#pragma once
#include "Singleton.h"
#include "Sprite.h"

class SpriteDrawer
	: public Singleton<SpriteDrawer>
{
public:
	friend class Singleton<SpriteDrawer>;
	bool Init();
	void Draw();
	void Update();
	void AddSprite(std::shared_ptr<Sprite> sprite);
	void CheckState();


protected:
	SpriteDrawer();
	~SpriteDrawer();

private:
	bool GenerateHandles();
	bool InitConstantHeaps();
	bool GenerateConstantView(std::shared_ptr<Sprite> sprite);
	bool DrawCall();
	void UpdateTexture(std::shared_ptr<Sprite> sprite);

private:
	ComPtr<ID3D12DescriptorHeap> m_BasicDescHeap;
	std::vector<MeshDrawer::CPU_GPU_Handles> m_ConstantHandles;
	std::vector<std::shared_ptr<Sprite>> sprites;

	int m_ObjectCount = 128 * 4;

};