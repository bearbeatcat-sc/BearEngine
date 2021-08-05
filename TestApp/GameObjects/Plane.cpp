#include "Plane.h"

#include "Utility/CameraManager.h"
#include "Components/MeshComponent.h"
#include "Components/Collsions/CollisionManager.h"

#include "InkManager.h"

Plane::Plane(const SimpleMath::Vector3& pos, const SimpleMath::Vector3& scale, InkManager* pInkManager)
	:mScale(scale),m_pInkManager(pInkManager)
{
	SetPosition(pos);
	SetScale(scale);
}

Plane::~Plane()
{
	m_pInkManager = 0;
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
	
	for(int z = 0; z < zCount; ++z)
	{
		for (int y = 0; y < yCount; ++y)
		{
			for(int x = 0; x < xCount; ++x)
			{
				auto wallParam = ParticleEmitter::WallPalam();
				wallParam.position = SimpleMath::Vector4(basePos.x + (x * wallScale), basePos.y + (y * wallScale), basePos.z + (z * wallScale), 1);
				wallParam.scale = SimpleMath::Vector4(50.0f, 50.0f, 50.0f, 1); // 今は仮
				wallParam.pressure = 0.0f;

				m_pInkManager->PushWallData(wallParam);
			}

		}
	}
	


}

void Plane::Shutdown()
{
}

void Plane::OnCollsion(Actor* other)
{
}
