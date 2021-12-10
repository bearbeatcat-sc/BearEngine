#pragma once
#include "Singleton.h"
#include "Sprite.h"

class SpriteDrawer
	: public Singleton<SpriteDrawer>
{
public:
	friend class Singleton<SpriteDrawer>;
	bool Init();
	void Render2DSprite();
	void Render3DSprite();
	void Update();
	void AddSprite(std::shared_ptr<Sprite> sprite);
	void CheckState();


protected:
	SpriteDrawer();
	~SpriteDrawer();

private:
	void AddSprite3D(const std::shared_ptr<Sprite>& sprite);
	void AddSprite2D(const std::shared_ptr<Sprite>& sprite);
	bool IsAddSprite();


	
	bool GenerateHandles();
	bool InitConstantHeaps();
	bool GenerateConstantView(std::shared_ptr<Sprite> sprite);
	bool RenderCall2D();
	bool RenderCall3D();
	void UpdateTexture(std::shared_ptr<Sprite> sprite);

private:
	ComPtr<ID3D12DescriptorHeap> _descHeap;
	std::vector<MeshDrawer::CPU_GPU_Handles> _constantHandles;
	std::vector<std::shared_ptr<Sprite>> _2dSprites;
	std::vector<std::shared_ptr<Sprite>> _3dSprites;

	int m_ObjectCount = 128 * 4;

};