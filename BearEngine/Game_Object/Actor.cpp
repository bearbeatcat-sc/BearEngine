#include "Actor.h"
#include "../Components/Component.h"
#include "ActorManager.h"
#include <DirectXMath.h>
#include "../Utility/Timer.h"

Actor::Actor()
	:m_first(true), m_Rotation(DirectX::SimpleMath::Quaternion::Identity), m_IsActive(true)
{
	m_WorldMatrix = DirectX::SimpleMath::Matrix::Identity;

}

Actor::~Actor()
{
	if (m_DetroyTimer) {
		delete m_DetroyTimer;
	}
}

void Actor::Update()
{
	if (m_first)
	{
		Init();
		m_first = false;
	}

	if (m_DetroyTimer)
	{
		if (DeathTimerUpdate())
		{
			Destroy();
		}
	}

	if (!m_IsActive)return;
	UpdateComponents();
	UpdateActor();

}

void Actor::UpdateComponents()
{
	for (auto component : m_Components)
	{
		component->Update();
	}
}

void Actor::Destroy()
{
	destroyFlag = true;
}

void Actor::Destroy(float time)
{
	m_DetroyTimer = new Timer(time);
}

void Actor::SetParent(Actor* parent)
{
	//parent->SetChild(this);
	m_Parent = parent;
}

void Actor::SetChild(Actor* child)
{
	m_Children.push_back(child);
	child->SetParent(this);
	ActorManager::GetInstance().AddActor(child);
}

std::vector<Actor*>& Actor::GetChildren()
{
	return m_Children;
}

const DirectX::SimpleMath::Vector3& Actor::GetPosition() const
{
	return m_Position;
}

void Actor::SetPosition(const DirectX::SimpleMath::Vector3& pos)
{
	m_Position = pos;
	SetWorldMatrix();
}

const DirectX::SimpleMath::Vector3& Actor::GetScale() const
{
	return m_Scale;
}

const DirectX::SimpleMath::Quaternion& Actor::GetRotation() const
{
	return m_Rotation;
}

void Actor::SetRotation(const DirectX::SimpleMath::Quaternion& rotation)
{
	m_Rotation = rotation;
	SetWorldMatrix();
}

void Actor::SetScale(const DirectX::SimpleMath::Vector3& scale)
{
	m_Scale = scale;
	SetWorldMatrix();
}


const  DirectX::SimpleMath::Matrix& Actor::GetWorldMatrix()
{
	//if (m_Parent == nullptr || m_Parent->GetDestroyFlag())
	//{
	//	return m_WorldMatrix;
	//}

	//return m_WorldMatrix * m_Parent->GetWorldMatrix();

	return m_WorldMatrix;
}

void Actor::SetWorldMatrix()
{
	//m_WorldMatrix = Matrix4::CreateScale(m_Scale) * Matrix4::CreateFromQuaternion(Quaternion(m_Axis,m_Angle)) * Matrix4::CreateTranslation(m_Position);
	m_WorldMatrix = DirectX::SimpleMath::Matrix::CreateScale(m_Scale) * DirectX::SimpleMath::Matrix::CreateFromQuaternion(m_Rotation) * DirectX::SimpleMath::Matrix::CreateTranslation(m_Position);
}

void Actor::SetWorldMatrix(const DirectX::SimpleMath::Matrix& mat)
{
	m_WorldMatrix = mat;
}

DirectX::SimpleMath::Vector3 Actor::GetForward()
{
	return DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3::Forward, m_Rotation);
	//return DirectX::SimpleMath::Vector3::Backward;
}

DirectX::SimpleMath::Vector3 Actor::GetBackward()
{
	return DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3::Backward, m_Rotation);
}

void Actor::AddComponent(std::shared_ptr<Component> component)
{
	int myOreder = component->GetUpdateOrder();

	//for (auto iter = m_Components.begin(); iter != m_Components.end(); iter++)
	//{
	//	// 追加するコンポーネントより優先度が高かったら次
	//	if (myOreder < (*iter)->GetUpdateOrder())
	//	{
	//		break;
	//	}

	//	m_Components.insert(iter, component);
	//}

	m_Components.push_back(component);
}

void Actor::RemoveComponent(std::shared_ptr<Component> component)
{
	// コンポネートを探す
	auto iter = std::find(m_Components.begin(), m_Components.end(), component);

	if (iter != m_Components.end())
	{
		m_Components.erase(iter);
	}
}

void Actor::RemoveComponent()
{
	//for (auto itr = m_Components.begin(); itr != m_Components.end(); itr++)
	//{
	//	//delete (*itr);
	//	m_Components.erase(itr);

	//	if (m_Components.size() == 0)return;
	//}

	m_Components.clear();
}

void Actor::RemoveChild()
{
	for (auto itr = m_Children.begin(); itr != m_Children.end(); itr++)
	{
		//delete (*itr);
		(*itr)->Destroy();
		m_Children.erase(itr);
	}
}

void Actor::SetActive(bool flag)
{
	m_IsActive = flag;
}

bool Actor::GetDestroyFlag()
{
	return destroyFlag;
}

void Actor::SetTag(const std::string& tagName)
{
	m_Tag = tagName;
}

std::string Actor::GetTag()
{
	return m_Tag;
}

bool Actor::IsContainsTag(const std::string& key)
{
	if (m_Tag.find(key) != std::string::npos)
	{
		return true;
	}

	return false;
}

void Actor::Clean()
{
	Shutdown();
	RemoveChild();
	RemoveComponent();
}

bool Actor::DeathTimerUpdate()
{
	m_DetroyTimer->Update();

	return m_DetroyTimer->IsTime();
}
