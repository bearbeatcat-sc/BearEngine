#ifndef _ACTOR_H_
#define _ACTOR_H_

#include <vector>
#include "../../DirectXTK12/Inc/SimpleMath.h"
#include <SimpleMath.h>
#include <string>
#include <memory>

using namespace DirectX;


class Component;
class Timer;

class Actor
{
public:
	Actor(const std::string& actorName = "Actor");
	virtual ~Actor();

	void Update();
	void UpdateComponents();
	void Destroy();
	void Destroy(float time);

	virtual void UpdateActor() = 0;
	virtual void Init() = 0;
	virtual void Shutdown() = 0;
	virtual void OnCollsion(Actor* other) = 0;

	void SetParent(Actor* parent);
	void SetChild(Actor* child);	
	std::vector<Actor*>& GetChildren();

	const DirectX::SimpleMath::Vector3 GetPosition();
	const DirectX::SimpleMath::Vector3 GetLocalPosition();
	void SetPosition(const DirectX::SimpleMath::Vector3& pos);

	const DirectX::SimpleMath::Vector3& GetScale() ;
	void SetScale(const DirectX::SimpleMath::Vector3& scale);

	const DirectX::SimpleMath::Quaternion& GetRotation() ;
	const DirectX::SimpleMath::Vector3& GetEulerRotation();
	
	void SetRotation(const DirectX::SimpleMath::Vector3& rotate);
	void SetRotation(const DirectX::SimpleMath::Quaternion rotate);

	const DirectX::SimpleMath::Matrix GetWorldMatrix() ;
	void SetWorldMatrix() ;
	void SetWorldMatrix(const DirectX::SimpleMath::Matrix& mat);

	DirectX::SimpleMath::Vector3 GetForward();
	DirectX::SimpleMath::Vector3 GetBackward();

	void AddComponent(std::shared_ptr<Component> component);
	void RemoveComponent(std::shared_ptr<Component> component);
	void RemoveComponent();
	void RemoveChild();
	void UpdateChild();

	void SetActive(bool flag);
	bool GetDestroyFlag();
	void SetTag(const std::string& tagName);
	std::string GetTag();
	bool IsContainsTag(const std::string& key);
	void Clean();
	void SetActorName(const std::string& actoName);

	//#ifdef _DEBUG

	void RenderDebug(int& index, int& selected);

	void RenderChildDebug(int& index, int& selected);
	void RenderHierarchy(int index);
//#endif

private:
	bool DeathTimerUpdate();

protected:
	DirectX::SimpleMath::Vector3 m_Position;
	DirectX::SimpleMath::Vector3 m_Scale;
	DirectX::SimpleMath::Vector3 m_EulerRotation;
	DirectX::SimpleMath::Quaternion m_Rotation;
	
	bool destroyFlag;
	std::string m_Tag;
	Actor* m_Parent;
	std::shared_ptr<Timer> m_DetroyTimer;
	std::string _ActoName;

private:
	std::vector<std::shared_ptr<Component>> m_Components;
	std::vector<Actor*> m_Children;
	bool m_first;

	DirectX::SimpleMath::Matrix m_WorldMatrix;
	bool m_IsActive;

	bool _isShowHierarchy;
};

#endif