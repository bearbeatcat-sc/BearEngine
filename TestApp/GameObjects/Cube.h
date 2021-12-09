#pragma once


#include <Game_Object/Actor.h>
#include <SimpleMath.h>
#include <memory>

using namespace DirectX;

class DXRInstance;
class Timer;
class CollisionComponent;
class RigidBodyComponent;

class Cube
	:public Actor
{
public:
	Cube(const SimpleMath::Vector3& pos, const SimpleMath::Vector3& scale,float destroyTime,const std::string& meshName,bool moveFlag = true);
	~Cube() = default;
	void OnStatic(bool posflag,bool rotateFlag);

private:
	virtual void UpdateActor() override;
	virtual void Init() override;
	virtual void Shutdown() override;
	virtual void OnCollsion(Actor* other) override;

	void Generate();

private:	
	std::shared_ptr<DXRInstance> _instance;
	std::shared_ptr<Timer> _DestroyTimer;
	std::shared_ptr<Timer> _GenerateTimer;
	SimpleMath::Vector3 _initScale;
	SimpleMath::Vector3 _Acc;
	bool _IsGenerate;
	bool _IsMove;
	bool _IsWhite;
	
	bool _IsStaticPosition;
	bool _IsStaticRotate;

private:
	CollisionComponent* m_pCollisionComponent;
	std::shared_ptr<RigidBodyComponent> _rigidBodyComponent;

	std::string _DXRMeshName;
};
