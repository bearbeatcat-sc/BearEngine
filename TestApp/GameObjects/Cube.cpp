#include "Cube.h"

#include "Device/Raytracing/DXRPipeLine.h"
#include "Utility/Random.h"
#include "Utility/Time.h"
#include "Utility/Timer.h"

Cube::Cube(const SimpleMath::Vector3& pos, const SimpleMath::Vector3& scale, float destroyTime, const std::string& meshName, bool moveFlag)
	:_initScale(scale), _IsGenerate(false), _IsMove(moveFlag), _DXRMeshName(meshName)
{
	SetPosition(pos);
	SetScale(scale);

	_DestroyTimer = std::make_shared<Timer>(destroyTime);
	_GenerateTimer = std::make_shared<Timer>(0.3f);

	_Acc = SimpleMath::Vector3(0, 4.0f, 0.0f);
}

void Cube::UpdateActor()
{
	if (!_IsMove)return;

	_Acc += SimpleMath::Vector3(0, -2.0f, 0) * Time::DeltaTime;
	m_Position += Time::DeltaTime * _Acc;

	if (!_IsGenerate)
	{
		Generate();
		return;
	}

	_DestroyTimer->Update();
	if (_DestroyTimer->IsTime())
	{
		_instance->Destroy();
		Destroy();
	}

	//m_Scale = SimpleMath::Vector3::Lerp(_initScale, SimpleMath::Vector3(0.0f), _DestroyTimer->GetRatio());
	auto mtx = SimpleMath::Matrix::CreateFromQuaternion(m_Rotation) * SimpleMath::Matrix::CreateScale(m_Scale) * SimpleMath::Matrix::CreateTranslation(m_Position);
	_instance->SetMatrix(mtx);
}

void Cube::Generate()
{
	_GenerateTimer->Update();

	if (_GenerateTimer->IsTime())
	{
		_GenerateTimer->Reset();
		_IsGenerate = true;
		return;
	}

	m_Scale = SimpleMath::Vector3::Lerp(SimpleMath::Vector3(0.0f), _initScale, _GenerateTimer->GetRatio() * 1.1f);
	auto mtx = SimpleMath::Matrix::CreateFromQuaternion(m_Rotation) * SimpleMath::Matrix::CreateScale(m_Scale) * SimpleMath::Matrix::CreateTranslation(m_Position);
	_instance->SetMatrix(mtx);
}

void Cube::Init()
{
	_instance = DXRPipeLine::GetInstance().AddInstance(_DXRMeshName, 0);


	auto mtx = SimpleMath::Matrix::CreateFromQuaternion(m_Rotation) * SimpleMath::Matrix::CreateScale(m_Scale) * SimpleMath::Matrix::CreateTranslation(m_Position);
	_instance->SetMatrix(mtx);
	_instance->CreateRaytracingInstanceDesc();


}

void Cube::Shutdown()
{
}

void Cube::OnCollsion(Actor* other)
{
}
