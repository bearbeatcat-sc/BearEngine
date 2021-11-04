#include "CameraAsistant.h"

#include <Utility/Camera.h>
#include <Utility/CameraManager.h>
#include <imgui/imgui.h>

#include <Utility/Math/MathUtility.h>

#include "Device/DirectX/DirectXInput.h"

CameraAsistant::CameraAsistant()
{
	m_Camera = CameraManager::GetInstance().GetMainCamera();
	m_Camera->SetPosition(SimpleMath::Vector3(0, 0, 0));
	m_Camera->SetTarget(SimpleMath::Vector3(0.0f, 0.0f, 1.0f));
	m_Camera->SetNear(0.1f);
	m_Camera->SetFar(100.0f);

	_Pitch = 0.0f;
	_Yaw = 3.2f;

	ViewMove(SimpleMath::Vector3(0.0f));
}

CameraAsistant::~CameraAsistant()
{
}

void CameraAsistant::Update()
{
#ifdef _DEBUG
	auto pos = m_Camera->GetPosition();
	auto target = m_Camera->GetTarget();
	float _near = m_Camera->GetNear();
	float _far = m_Camera->GetFar();
	float _scale = m_Camera->GetSpriteScale();
	
	float temp_pos[]
	{
		pos.x,
		pos.y,
		pos.z,
	};

	float temp_target[]
	{
		target.x,
		target.y,
		target.z,
	};


	ImGui::Begin("Camera", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::DragFloat3("Position", temp_pos, 0.001f);
	ImGui::DragFloat3("Target", temp_target, 0.001f);
	ImGui::DragFloat("Near", &_near, 0.001f,0.1f);
	ImGui::DragFloat("Far", &_far, 0.001f);
	ImGui::DragFloat("SpriteScale", &_scale, 0.0001f);
	ImGui::End();

	_near = MathUtility::Clamp(_near, 0.001f, _far);
	
	m_Camera->SetPosition(SimpleMath::Vector3(temp_pos));
	m_Camera->SetTarget(SimpleMath::Vector3(temp_target));
	m_Camera->SetNear(_near);
	m_Camera->SetSpriteScale(_scale);
	m_Camera->SetFar(_far);
#endif

	if(DirectXInput::GetInstance().IsKey(DIK_W))
	{
		PositionMove(SimpleMath::Vector3::Backward);
	}

	if (DirectXInput::GetInstance().IsKey(DIK_S))
	{
		PositionMove(SimpleMath::Vector3::Forward);
	}

	if (DirectXInput::GetInstance().IsKey(DIK_A))
	{
		PositionMove(SimpleMath::Vector3::Right);
	}

	if (DirectXInput::GetInstance().IsKey(DIK_D))
	{
		PositionMove(SimpleMath::Vector3::Left);
	}


	if(DirectXInput::GetInstance().IsKey(DIK_LEFTARROW))
	{
		ViewMove(SimpleMath::Vector3::Left);
	}

	if (DirectXInput::GetInstance().IsKey(DIK_RIGHTARROW))
	{
		ViewMove(SimpleMath::Vector3::Right);
	}

	if (DirectXInput::GetInstance().IsKey(DIK_UPARROW))
	{
		ViewMove(SimpleMath::Vector3::Up);
	}

	if (DirectXInput::GetInstance().IsKey(DIK_DOWNARROW))
	{
		ViewMove(SimpleMath::Vector3::Down);
	}
}

void CameraAsistant::PositionMove(const SimpleMath::Vector3& moveVec)
{
	auto currentPos = m_Camera->GetPosition();
	auto currentTarget = m_Camera->GetTarget();

	auto qu = MathUtility::LookAt(currentPos, currentTarget);

	auto vec = SimpleMath::Vector3::Transform(moveVec, qu);
	vec.Normalize();

	auto newPos = currentPos + vec;
	auto newTarget = currentTarget + vec;
	m_Camera->SetPosition(newPos);
	m_Camera->SetTarget(newTarget);
}


void CameraAsistant::ViewMove(const SimpleMath::Vector3& moveVec)
{
	auto currentTarget = m_Camera->GetTarget();

	_Pitch += moveVec.y * 0.01f;
	_Yaw += moveVec.x * 0.01f;

	SimpleMath::Quaternion qu = SimpleMath::Quaternion::CreateFromYawPitchRoll(_Yaw, _Pitch, 0.0f);

	SimpleMath::Vector3 viewForward = SimpleMath::Vector3::Transform(SimpleMath::Vector3::Forward, qu);
	SimpleMath::Vector3 newTarget = m_Camera->GetPosition() + viewForward;


	m_Camera->SetTarget(newTarget);
}
