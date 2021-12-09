#include "Actor.h"
#include "../Components/Component.h"
#include "ActorManager.h"
#include <DirectXMath.h>
#include <tchar.h>

#include "../Utility/Timer.h"
#include <imgui/imgui.h>

#include "Game.h"

Actor::Actor(const std::string& actorName)
	:m_Position(DirectX::SimpleMath::Vector3::Zero), m_Scale(DirectX::SimpleMath::Vector3::One), m_Rotation(DirectX::SimpleMath::Vector3::Zero), _ActoName(actorName), m_first(true), m_WorldMatrix(DirectX::SimpleMath::Matrix::Identity), m_IsActive(true),
	_isShowHierarchy(false), m_Parent(nullptr), destroyFlag(false)
{

}

Actor::~Actor()
{

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
	UpdateChild();
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
	m_DetroyTimer = std::make_shared<Timer>(time);
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
	//ActorManager::GetInstance().AddActor(child);
}

std::vector<Actor*>& Actor::GetChildren()
{
	return m_Children;
}

const DirectX::SimpleMath::Vector3 Actor::GetPosition()
{
	if (m_Parent == nullptr)
	{
		return m_Position;
	}

	auto matrix = GetWorldMatrix();

	return DirectX::SimpleMath::Vector3(matrix._41, matrix._42, matrix._43);
}

void Actor::SetPosition(const DirectX::SimpleMath::Vector3& pos)
{
	m_Position = pos;
	SetWorldMatrix();
}

const DirectX::SimpleMath::Vector3& Actor::GetScale()
{

	if (m_Parent == nullptr)
	{
		return m_Scale;
	}

	auto matrix = GetWorldMatrix();

	auto size_x = DirectX::SimpleMath::Vector3(matrix._11, matrix._12, matrix._13).Length();
	auto size_y = DirectX::SimpleMath::Vector3(matrix._11, matrix._12, matrix._13).Length();
	auto size_z = DirectX::SimpleMath::Vector3(matrix._11, matrix._12, matrix._13).Length();

	return DirectX::SimpleMath::Vector3(size_x, size_y, size_z);
}

const DirectX::SimpleMath::Quaternion Actor::GetRotation()
{
	return DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(m_Rotation.x, m_Rotation.y, m_Rotation.z);

}

const DirectX::SimpleMath::Vector3& Actor::GetVecRotation()
{
	return m_Rotation;
}

void Actor::SetRotation(const DirectX::SimpleMath::Vector3 rotate)
{
	m_Rotation = rotate;
	SetWorldMatrix();
}

void Actor::SetScale(const DirectX::SimpleMath::Vector3& scale)
{
	m_Scale = scale;
	SetWorldMatrix();
}


const DirectX::SimpleMath::Matrix Actor::GetWorldMatrix()
{
	SetWorldMatrix();

	if (m_Parent == nullptr )
	{
		return m_WorldMatrix;
	}

	// 今回は同じ用に扱う。	
	return m_WorldMatrix * m_Parent->GetWorldMatrix();
}

void Actor::SetWorldMatrix()
{
	//m_WorldMatrix = Matrix4::CreateScale(m_Scale) * Matrix4::CreateFromQuaternion(Quaternion(m_Axis,m_Angle)) * Matrix4::CreateTranslation(m_Position);
	m_WorldMatrix = DirectX::SimpleMath::Matrix::CreateScale(m_Scale) * DirectX::SimpleMath::Matrix::CreateFromQuaternion(GetRotation()) * DirectX::SimpleMath::Matrix::CreateTranslation(m_Position);
}

void Actor::SetWorldMatrix(const DirectX::SimpleMath::Matrix& mat)
{
	m_WorldMatrix = mat;
}

DirectX::SimpleMath::Vector3 Actor::GetForward()
{
	return DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3::Forward, GetRotation());
	//return DirectX::SimpleMath::Vector3::Backward;
}

DirectX::SimpleMath::Vector3 Actor::GetBackward()
{
	return DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3::Backward, GetRotation());
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
	for (auto itr = m_Children.begin(); itr != m_Children.end();)
	{
		(*itr)->Clean();
		delete (*itr);
		(*itr) = nullptr;
		itr = m_Children.erase(itr);
	}
}

void Actor::UpdateChild()
{
	for (auto itr = m_Children.begin(); itr != m_Children.end(); itr++)
	{
		(*itr)->Update();
	}
}

//#ifdef _DEBUG
void Actor::RenderChildDebug(int& index, int& selected)
{
	for (auto itr = m_Children.begin(); itr != m_Children.end(); itr++)
	{
		(*itr)->RenderDebug(index, selected);
	}
}

void Actor::RenderHierarchy(int index)
{
	if (!_isShowHierarchy)return;

	float f_position[3] =
	{
		m_Position.x,
		m_Position.y,
		m_Position.z
	};

	float f_scale[3] =
	{
		m_Scale.x,
		m_Scale.y,
		m_Scale.z
	};

	float f_rotation[3] =
	{
		m_Rotation.x,
		m_Rotation.y,
		m_Rotation.z
	};

	bool isChange = false;

	std::string windowName = "Hierarchy:" + _ActoName;

	ImGui::Begin(windowName.c_str(), &_isShowHierarchy);

	if (ImGui::DragFloat3("Position", f_position, 0.01f))
		isChange = true;

	if (ImGui::DragFloat3("Scale", f_scale, 0.01f))
		isChange = true;

	if (ImGui::DragFloat3("Rotation", f_rotation, 0.01f))
		isChange = true;

	for (auto component : m_Components)
	{
		component->DrawProperties();
	}

	if (isChange)
	{
		m_Position = DirectX::SimpleMath::Vector3(f_position);
		m_Rotation = DirectX::SimpleMath::Vector3(f_rotation);
		m_Scale = DirectX::SimpleMath::Vector3(f_scale);
	}
	ImGui::End();
}

//#endif

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

void Actor::SetActorName(const std::string& actoName)
{
	_ActoName = actoName;
}

//#ifdef _DEBUG
void Actor::RenderDebug(int& index, int& selected)
{
	index++;

	ImGui::PushID((_ActoName + std::to_string(index)).c_str());

	if (ImGui::Selectable(_ActoName.c_str(), selected == index, ImGuiSelectableFlags_SpanAllColumns))
	{
		selected = index;
		_isShowHierarchy = true;
	}

	if (m_Children.size() > 0)
	{
		if (ImGui::TreeNode("Child"))
		{
			RenderChildDebug(index, selected);
			ImGui::TreePop();
		}
	}

	if (selected == index)
	{
		RenderHierarchy(index);
	}

	ImGui::PopID();

}
//#endif

bool Actor::DeathTimerUpdate()
{
	m_DetroyTimer->Update();

	return m_DetroyTimer->IsTime();
}
