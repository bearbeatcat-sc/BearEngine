#include "MeshComponent.h"
#include "../Utility/Camera.h"
#include "../Device/DirectX/Core/Model/GeomtryGenerater.h"
#include "../Game_Object/Actor.h"

MeshComponent::MeshComponent(Actor* user, const std::string& modelName,std::shared_ptr<Camera> camera, const std::string& effectName, int updateOrder)
	:Component(user,updateOrder),m_Camera(camera),
	m_Scale(SimpleMath::Vector3(1,1,1)),m_Rotation(SimpleMath::Vector3(0,0,0))
{
	// OBJ形式などのメッシュを取得
	m_Mesh = MeshManager::GetInstance().GetMesh(modelName,effectName);
	MeshDrawer::GetInstance().AddObjMesh(m_Mesh);
	
}

MeshComponent::MeshComponent(Actor* user, std::shared_ptr<Camera> camera,const std::string& effectName, int updateOrder)
	:Component(user, updateOrder), m_Camera(camera),
	m_Scale(SimpleMath::Vector3(1, 1, 1)), m_Rotation(SimpleMath::Vector3(0, 0, 0))
{
	m_Mesh = MeshManager::GetInstance().GetCubeMesh(effectName);
	MeshDrawer::GetInstance().AddObjMesh(m_Mesh);
}

MeshComponent::MeshComponent(Actor* user, std::shared_ptr<Camera> camera, const SimpleMath::Vector3* points, const std::string& effectName, int updateOrder)
	:Component(user, updateOrder), m_Camera(camera),
	m_Scale(SimpleMath::Vector3(1, 1, 1)), m_Rotation(SimpleMath::Vector3(0, 0, 0))
{
	m_Mesh = MeshManager::GetInstance().GetTriangleMesh(points, effectName);
	MeshDrawer::GetInstance().AddObjMesh(m_Mesh);
}

MeshComponent::MeshComponent(Actor* user, std::shared_ptr<Camera> camera, size_t tessellation, const std::string& effectName, int updateOrder)
	:Component(user, updateOrder), m_Camera(camera),
	m_Scale(SimpleMath::Vector3(1, 1, 1)), m_Rotation(SimpleMath::Vector3(0, 0, 0))
{
	m_Mesh = MeshManager::GetInstance().GetSpehereMesh(tessellation, effectName);
	MeshDrawer::GetInstance().AddObjMesh(m_Mesh);
}

MeshComponent::~MeshComponent()
{
	m_Mesh->Destroy();
	m_Mesh = 0;
}

void MeshComponent::Update()
{
	//SetMatrix(m_User->GetWorldMatrix());
}

void MeshComponent::SetMatrix(const SimpleMath::Matrix& mat)
{
	m_Mesh->SetMatrix(mat);
}

void MeshComponent::SetPosition(const SimpleMath::Vector3& pos)
{
	m_Position = pos;
	SetMatrix(DirectX::SimpleMath::Matrix::CreateScale(m_Scale) * DirectX::SimpleMath::Matrix::CreateFromQuaternion(m_Rotation) * DirectX::SimpleMath::Matrix::CreateTranslation(m_Position));
}

void MeshComponent::SetScale(const DirectX::SimpleMath::Vector3& scale)
{
	m_Scale = scale;
	SetMatrix(DirectX::SimpleMath::Matrix::CreateScale(m_Scale) * DirectX::SimpleMath::Matrix::CreateFromQuaternion(m_Rotation) * DirectX::SimpleMath::Matrix::CreateTranslation(m_Position));
}

void MeshComponent::SetRotation(const DirectX::SimpleMath::Quaternion& rotation)
{
	m_Rotation = rotation;
	SetMatrix(DirectX::SimpleMath::Matrix::CreateScale(m_Scale) * DirectX::SimpleMath::Matrix::CreateFromQuaternion(m_Rotation) * DirectX::SimpleMath::Matrix::CreateTranslation(m_Position));
}

void MeshComponent::SetDrawType(MeshDrawer::DrawType drawType)
{
	m_Mesh->SetDrawType(drawType);
}

void MeshComponent::SetColor(const SimpleMath::Color& color)
{
	m_Mesh->SetColor(XMFLOAT4(color));
}

void MeshComponent::SetEffect(const std::string& effectName)
{
	m_Mesh->SetEffectName(effectName);
}

void MeshComponent::SetVisible(bool flag)
{
	m_Mesh->SetVisible(flag);
}

void MeshComponent::DrawProperties()
{
	
}
