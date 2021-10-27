#include "Cube.h"

#include "Device/Raytracing/DXRPipeLine.h"
#include "Utility/Random.h"
#include "Utility/Time.h"
#include "Utility/Timer.h"

Cube::Cube(const SimpleMath::Vector3& pos, const SimpleMath::Vector3& scale, float destroyTime)
	:_initScale(scale)
{
	SetPosition(pos);
	SetScale(scale);

	_DestroyTimer = std::make_shared<Timer>(destroyTime);
}

void Cube::UpdateActor()
{
	_DestroyTimer->Update();
	if(_DestroyTimer->IsTime())
	{
		_instance->Destroy();
		Destroy();
	}

	m_Position += Time::DeltaTime * SimpleMath::Vector3(0, 1.0f, 0);
	m_Scale = SimpleMath::Vector3::Lerp(_initScale, SimpleMath::Vector3(0.0f), _DestroyTimer->GetRatio());
	auto mtx = SimpleMath::Matrix::CreateScale(m_Scale) * SimpleMath::Matrix::CreateTranslation(m_Position);
	_instance->SetMatrix(mtx);
}

void Cube::Init()
{
	_instance = DXRPipeLine::GetInstance().AddInstance("Cube", 0);

	auto mtx = SimpleMath::Matrix::CreateScale(m_Scale) * SimpleMath::Matrix::CreateTranslation(m_Position);
	_instance->SetMatrix(mtx);
	_instance->CreateRaytracingInstanceDesc();

}

void Cube::Shutdown()
{
}

void Cube::OnCollsion(Actor* other)
{
}
