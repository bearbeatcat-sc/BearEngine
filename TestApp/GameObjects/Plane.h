#ifndef _PLANE_H_
#define _PLANE_H_

#include <Game_Object/Actor.h>
#include <SimpleMath.h>
#include <memory>

#include "Device/DirectX/Core/Model/MeshDrawer.h"


using namespace DirectX;

class MeshComponent;
class InkManager;

class Plane
	:public Actor
{
public:
	Plane(const SimpleMath::Vector3& pos, const SimpleMath::Vector3& scale);
	~Plane();
	void SetColor(const SimpleMath::Color& color);
	void SetDrawType(MeshDrawer::DrawType drawType);

private:
	virtual void UpdateActor() override;
	virtual void Init() override;
	virtual void Shutdown() override;
	virtual void OnCollsion(Actor* other) override;

private:
	std::shared_ptr<MeshComponent> m_pMeshComponent;
	
	SimpleMath::Vector3 mScale;
	MeshDrawer::DrawType mDrawType;
	SimpleMath::Color mColor;
};

#endif