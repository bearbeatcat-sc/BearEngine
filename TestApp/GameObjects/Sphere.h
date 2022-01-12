#ifndef _SPHERE_H_
#define _SPHERE_H_

#include <Game_Object/Actor.h>
#include <SimpleMath.h>
#include <memory>

#include "Components/Physics/RigidBodyComponent.h"
#include "Device/Raytracing/DXRInstance.h"

using namespace DirectX::SimpleMath;

class MeshComponent;
class SphereCollisionComponent;
class OBBCollisionComponent;

class Sphere
	:public Actor
{
public:
	enum SphereType
	{
		SphereType_Normal,
		SphereType_NormalLowPoly,
		SphereType_GouraudNormal,
		SphereType_GouraudLowPoly,
		SphereType_NormalMesh,
		SphereType_GouraudMesh,
	};

	enum ActionType
	{
		ActionType_UpDown,
		ActionType_Thrust,
	};
	
	Sphere(Vector3 pos, SphereType type);
	~Sphere();
	void SetColor(const DirectX::SimpleMath::Color& color);

private:
	virtual void UpdateActor() override;
	virtual void Init() override;
	virtual void Shutdown() override;
	virtual void OnCollsion(Actor* other) override;

	void UpDown();
	void Thrust();
	
	void Move();

private:
	//std::shared_ptr<MeshComponent> m_pMeshComponent;
	
	std::shared_ptr<DXRInstance> _instance;
	SphereCollisionComponent* _sphereCollisionComponent;
	OBBCollisionComponent* _obbCollisionComponent;
	
	ActionType m_ActionType;
	SphereType m_Type;
	float m_MoveTime;
	DirectX::SimpleMath::Color mColor;
	Vector3 mInitPos;
	Vector3 mStartPos;
};

#endif