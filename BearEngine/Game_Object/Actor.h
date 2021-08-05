#ifndef _ACTOR_H_
#define _ACTOR_H_

#include <vector>
#include "../../DirectXTK12/Inc/SimpleMath.h"
#include <string>
#include <memory>


class Component;
class Timer;

class Actor
{
public:
	Actor();
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

	const DirectX::SimpleMath::Vector3& GetPosition() const;
	void SetPosition(const DirectX::SimpleMath::Vector3& pos);

	const DirectX::SimpleMath::Vector3& GetScale() const;
	void SetScale(const DirectX::SimpleMath::Vector3& scale);

	const DirectX::SimpleMath::Quaternion& GetRotation() const;
	void SetRotation(const DirectX::SimpleMath::Quaternion& axis);

	const DirectX::SimpleMath::Matrix& GetWorldMatrix();
	void SetWorldMatrix();
	void SetWorldMatrix(const DirectX::SimpleMath::Matrix& mat);

	DirectX::SimpleMath::Vector3 GetForward();
	DirectX::SimpleMath::Vector3 GetBackward();

	void AddComponent(std::shared_ptr<Component> component);
	void RemoveComponent(std::shared_ptr<Component> component);
	void RemoveComponent();
	void RemoveChild();
	void SetActive(bool flag);
	bool GetDestroyFlag();
	void SetTag(const std::string& tagName);
	std::string GetTag();
	bool IsContainsTag(const std::string& key);
	void Clean();

private:
	bool DeathTimerUpdate();

protected:
	DirectX::SimpleMath::Vector3 m_Position;
	DirectX::SimpleMath::Vector3 m_Scale;
	DirectX::SimpleMath::Quaternion m_Rotation;
	bool destroyFlag;
	std::string m_Tag;
	Actor* m_Parent;
	Timer* m_DetroyTimer;

private:
	std::vector<std::shared_ptr<Component>> m_Components;
	std::vector<Actor*> m_Children;
	bool m_first;

	DirectX::SimpleMath::Matrix m_WorldMatrix;
	bool m_IsActive;
};

#endif