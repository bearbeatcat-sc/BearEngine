#include "Plane.h"

#include "Utility/CameraManager.h"
#include "Components/MeshComponent.h"
#include "Components/Collsions/CollisionManager.h"


Plane::Plane(const SimpleMath::Vector3& pos, const SimpleMath::Vector3& scale)
	:mScale(scale)
{
	SetPosition(pos);
	SetScale(scale);
}

Plane::~Plane()
{
}

void Plane::SetColor(const SimpleMath::Color& color)
{
	mColor = color;
}

void Plane::SetDrawType(MeshDrawer::DrawType drawType)
{
	mDrawType = drawType;
}

void Plane::UpdateActor()
{
}

void Plane::Init()
{
	m_pMeshComponent = std::shared_ptr<MeshComponent>(new MeshComponent(this,CameraManager::GetInstance().GetMainCamera(),"NormalMeshEffect"));
	m_pMeshComponent->SetMatrix(GetWorldMatrix());
	m_pMeshComponent->SetColor(mColor);
	m_pMeshComponent->SetDrawType(mDrawType);
	AddComponent(m_pMeshComponent);

	auto scale = GetScale();
	const float wallScale = 0.22f;

	const float yCount = scale.y / wallScale;
	const float xCount = scale.x / wallScale;
	const float zCount = scale.z / wallScale;

	const SimpleMath::Vector3 basePos = GetPosition() - (scale * 0.5f);
	


}

void Plane::Shutdown()
{
}

void Plane::OnCollsion(Actor* other)
{
}
