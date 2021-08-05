#include "Player.h"

#include "InkBall.h"
#include "Device/DirectX/DirectXInput.h"
#include "Game_Object/ActorManager.h"
#include "imgui/imgui.h"
#include "Utility/Camera.h"
#include "Utility/CameraManager.h"

#include "Utility/Timer.h"

Player::Player( InkManager* pInkManager)
	:Actor(),mThrowVec(),m_pInkManager(pInkManager)
{
	
}

Player::~Player()
{
	
}

void Player::Init()
{
	m_pTimer = std::make_shared<Timer>(1.0f);
}

void Player::UpdateActor()
{
	ThrowVecControl();
	Move();

	m_pTimer->Update();
	if(IsThrow())
	{
		Throw();
		m_pTimer->Reset();
	}
}

void Player::Shutdown()
{
	
}

void Player::OnCollsion(Actor* other)
{
	
}

void Player::Move()
{
	mCenterPosition = CameraManager::GetInstance().GetMainCamera()->GetPosition();
}

void Player::Throw()
{
	auto vec = SimpleMath::Vector3(mThrowVec.x, mThrowVec.y, 10.0f);

	auto inkBall = new InkBall(mCenterPosition, vec, SimpleMath::Vector3(0, -1.0f, -1.0f), m_pInkManager);
	inkBall->SetScale(SimpleMath::Vector3(0.3f, 0.3f, 0.3f));
	ActorManager::GetInstance().AddActor(inkBall);
}

void Player::ThrowVecControl()
{
	float x_vec = DirectXInput::GetInstance().GetGamePadValue(GAMEPAD_ThubStick_LX);
	float y_vec = DirectXInput::GetInstance().GetGamePadValue(GAMEPAD_ThubStick_LY);


	float throwVec[3] = { x_vec,y_vec,0 };

	ImGui::Begin("InkBall_Property", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::DragFloat3("", throwVec, 0.01f, -10.0f, 10.0f);
	ImGui::End();


	mThrowVec.x = x_vec;
	mThrowVec.y = y_vec;
}

bool Player::IsThrow()
{
	return (DirectXInput::GetInstance().IsDownTrigger(GamePad_Triggers::GamePad_RightTrigger) ||
		DirectXInput::GetInstance().IsKeyDown(DIK_SPACE)) && 
		m_pTimer->IsTime();
}




