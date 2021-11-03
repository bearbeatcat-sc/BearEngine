#include "CameraAsistant.h"

#include <Utility/Camera.h>
#include <Utility/CameraManager.h>
#include <imgui/imgui.h>

#include <Utility/Math/MathUtility.h>

CameraAsistant::CameraAsistant()
{
	m_Camera = CameraManager::GetInstance().GetMainCamera();
	m_Camera->SetPosition(SimpleMath::Vector3(0, 0, 0));
	m_Camera->SetTarget(SimpleMath::Vector3(0.0f, 0.0f, 1.0f));
	m_Camera->SetNear(0.1f);
	m_Camera->SetFar(100.0f);
}

CameraAsistant::~CameraAsistant()
{
}

void CameraAsistant::Update()
{
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
}
