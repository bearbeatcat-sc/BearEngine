#include "DirectXInput.h"
#include "../WindowApp.h"
#include <assert.h>
#include "../../Utility/Time.h"

static DIMOUSESTATE currentMouseState;
static DIMOUSESTATE previousMouseState;

DirectXInput::DirectXInput()
{
}

DirectXInput::~DirectXInput()
{
	m_MouseDevice->Unacquire();
	m_Devkeyboard->Unacquire();

	m_Buttons.clear();

	for (auto vibration : m_Vibrations)
	{
		delete vibration;
	}

	m_Vibrations.clear();

}

bool DirectXInput::CreateKeyBoardDevice()
{

	if (FAILED(m_pDevice->CreateDevice(GUID_SysKeyboard, &m_Devkeyboard, NULL)))
	{
		assert(0);
	}

	if (FAILED(m_Devkeyboard->SetDataFormat(&c_dfDIKeyboard)))
	{
		assert(0);
	}

	if (FAILED(m_Devkeyboard->SetDataFormat(&c_dfDIKeyboard)))
	{
		assert(0);
	}

	if (FAILED(m_Devkeyboard->SetCooperativeLevel(
		WindowApp::GetInstance().GetHWND(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY)))
	{
		assert(0);
	}

	if (FAILED(m_Devkeyboard->Acquire()))
	{
		return false;
	}

	if (FAILED(m_Devkeyboard->Poll()))
	{
		return false;
	}


	return true;
}

bool DirectXInput::CreateMouseDevice()
{
	if (FAILED(m_pDevice->CreateDevice(GUID_SysMouse, &m_MouseDevice, NULL)))
	{
		assert(0);
	}

	if (FAILED(m_MouseDevice->SetDataFormat(&c_dfDIMouse)))
	{
		assert(0);
	}

	if (FAILED(m_MouseDevice->SetCooperativeLevel(
		WindowApp::GetInstance().GetHWND(),
		DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
	{
		assert(0);
	}

	if (FAILED(m_MouseDevice->Acquire()))
	{
		return false;
	}

	if (FAILED(m_MouseDevice->Poll()))
	{
		return false;
	}

	return true;
}

bool DirectXInput::InitDirectInput()
{

	if (FAILED(DirectInput8Create(
		WindowApp::GetInstance().GetWndClassEx().hInstance,
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)&m_pDevice,
		nullptr)))
	{
		assert(0);
	}

	if (FAILED(CreateMouseDevice()))
	{
		assert(0);
	}

	if (FAILED(CreateKeyBoardDevice()))
	{
		assert(0);
	}

	// リストにボタンを追加して、更新時に使う
	m_Buttons.push_back(GAMEPAD_V_BUTTON_UP);
	m_Buttons.push_back(GAMEPAD_V_BUTTON_DOWN);
	m_Buttons.push_back(GAMEPAD_V_BUTTON_LEFT);
	m_Buttons.push_back(GAMEPAD_V_BUTTON_RIGHT);

	m_Buttons.push_back(GAMEPAD_V_BUTTON_START);
	m_Buttons.push_back(GAMEPAD_V_BUTTON_BACK);

	m_Buttons.push_back(GAMEPAD_V_BUTTON_LEFT_THUB);
	m_Buttons.push_back(GAMEPAD_V_BUTTON_RIGHT_THUB);
	m_Buttons.push_back(GAMEPAD_V_BUTTON_LEFT_SHOULDER);
	m_Buttons.push_back(GAMEPAD_V_BUTTON_RIGHT_SHOULDER);

	m_Buttons.push_back(GAMEPAD_V_BUTTON_A);
	m_Buttons.push_back(GAMEPAD_V_BUTTON_B);
	m_Buttons.push_back(GAMEPAD_V_BUTTON_X);
	m_Buttons.push_back(GAMEPAD_V_BUTTON_Y);

	m_Vibrations.resize(4);

	return true;
}

bool DirectXInput::UpdateDirectInput()
{

	BYTE key[256] = {};

	if (FAILED(m_Devkeyboard->GetDeviceState(sizeof(key), key)))
	{
		m_Devkeyboard->Acquire();
	}

	for (int i = 0; i < 256; ++i)
	{
		m_PreviousKeyState[i] = m_CurrentKeyState[i];
	}

	for (int i = 0; i < 256; ++i)
	{
		if (key[i] == 0x80)
		{
			m_CurrentKeyState[i] = true;
			continue;
		}

		m_CurrentKeyState[i] = false;
	}

	previousMouseState = currentMouseState;
	if (FAILED(m_MouseDevice->GetDeviceState(sizeof(DIMOUSESTATE), &currentMouseState)))
	{
		m_MouseDevice->Acquire();
	}


	return true;
}

bool DirectXInput::UpdateXINPUT()
{
	XInputGetState(0, &m_State);

	for (int i = 0; i < _countof(m_CurrentButtonState); ++i)
	{
		m_PreviousButtonState[i] = m_CurrentButtonState[i];
	}

	m_previousTriggerState = m_currentTriggerState;

	if (m_State.Gamepad.bLeftTrigger > 1.0f)
	{
		m_currentTriggerState.LeftTriggerFlag = true;
	}
	else
	{
		m_currentTriggerState.LeftTriggerFlag = false;
	}

	if (m_State.Gamepad.bRightTrigger > 1.0f)
	{
		m_currentTriggerState.RightTriggerFlag = true;
	}
	else
	{
		m_currentTriggerState.RightTriggerFlag = false;
	}

	for (int i = 0; i < m_Buttons.size(); ++i)
	{
		if (m_State.Gamepad.wButtons == m_Buttons[i])
		{
			m_CurrentButtonState[i] = true;
			continue;
		}

		m_CurrentButtonState[i] = false;
	}

	return true;
}

void DirectXInput::UpdateInput()
{
	UpdateDirectInput();
	UpdateXINPUT();
	VibrationUpdate();
}

bool DirectXInput::IsKeyDown(int keyNum)
{
	return m_CurrentKeyState[keyNum] && !m_PreviousKeyState[keyNum];
}

bool DirectXInput::IsKey(int keyNum)
{
	return m_CurrentKeyState[keyNum];
}

bool DirectXInput::IsMouseButtonDown(MouseButton button) {
	return !(previousMouseState.rgbButtons[(int)button] & 0x80) &&
		currentMouseState.rgbButtons[(int)button] & 0x80;
}

bool DirectXInput::IsMouseButton(MouseButton button) {
	return currentMouseState.rgbButtons[(int)button] & (0x80);
}

bool DirectXInput::IsMouseButtonUp(MouseButton button) {
	return previousMouseState.rgbButtons[(int)button] & (0x80) &&
		!(currentMouseState.rgbButtons[(int)button] & 0x80);
}

float DirectXInput::GetWheelValue() {
	return (float)currentMouseState.lZ;
}

bool DirectXInput::IsActiveGamePad()
{
	return XInputGetState(0, &m_State) != ERROR_DEVICE_NOT_CONNECTED;
}

DirectX::SimpleMath::Vector2 DirectXInput::GetMouseMove() {
	return DirectX::SimpleMath::Vector2((float)currentMouseState.lX, (float)currentMouseState.lY);
}

DirectX::SimpleMath::Vector2 DirectXInput::GetCursorPos() {
	POINT p{};
	::GetCursorPos(&p);
	ScreenToClient(WindowApp::GetInstance().GetHWND(), &p);
	return DirectX::SimpleMath::Vector2((float)p.x, (float)p.y);
}

float DirectXInput::GetGamePadValue(GamePad_ThubStick stick)
{


	switch (stick)
	{
	case GAMEPAD_ThubStick_LX:
		// 正規化？
		return m_State.Gamepad.sThumbLX / 32767.0;
		break;
	case GAMEPAD_ThubStick_LY:
		// 正規化？
		return m_State.Gamepad.sThumbLY / 32767.0;
		break;

	case GAMEPAD_ThubStick_RX:
		// 正規化？
		return m_State.Gamepad.sThumbRX / 32767.0;
		break;

	case GAMEPAD_ThubStick_RY:
		// 正規化？
		return m_State.Gamepad.sThumbRY / 32767.0;
		break;
	}


	return 0.0f;
}

bool DirectXInput::IsDownTrigger(GamePad_Triggers trigger)
{
	switch (trigger)
	{
	case GamePad_RightTrigger:
		return m_currentTriggerState.RightTriggerFlag
			&& !m_previousTriggerState.RightTriggerFlag;

	case GamePad_LeftTrigger:
		return m_currentTriggerState.LeftTriggerFlag
			&& !m_previousTriggerState.LeftTriggerFlag;
	}

	return false;
}

bool DirectXInput::IsUpTrigger(GamePad_Triggers trigger)
{
	switch (trigger)
	{
	case GamePad_RightTrigger:
		return m_previousTriggerState.RightTriggerFlag
			&& !m_currentTriggerState.RightTriggerFlag;

	case GamePad_LeftTrigger:
		return m_previousTriggerState.LeftTriggerFlag
			&& !m_currentTriggerState.LeftTriggerFlag;
	}

	return false;
}

float DirectXInput::IsDownTriggerValue(GamePad_Triggers trigger)
{
	switch (trigger)
	{
	case GamePad_RightTrigger:
		return m_State.Gamepad.bRightTrigger;

	case GamePad_LeftTrigger:
		return m_State.Gamepad.bLeftTrigger;
	}
}

bool DirectXInput::IsTrigger(GamePad_Triggers trigger)
{
	switch (trigger)
	{
	case GamePad_RightTrigger:
		return m_currentTriggerState.RightTriggerFlag;

	case GamePad_LeftTrigger:
		return m_currentTriggerState.LeftTriggerFlag;
	}

	return false;
}

bool DirectXInput::isButtonDown(GamePad_Buttons button)
{
	return m_CurrentButtonState[button] && !m_PreviousButtonState[button];
}

bool DirectXInput::isButton(GamePad_Buttons button)
{
	return m_CurrentButtonState[button];
}

void DirectXInput::VibrationUpdate()
{
	for (auto vaibration : m_Vibrations)
	{
		if (vaibration == nullptr) continue;

		vaibration->Update();
	}
}

float DirectXInput::GetVibration(int user)
{
	if (m_Vibrations[user] == nullptr)return 0.0f;

	return m_Vibrations[user]->GetPower();
}

void DirectXInput::OnVibration(int user, int leftPower, int rightPower, float time)
{
	if (m_Vibrations[user] == nullptr)
	{
		m_Vibrations[user] = new GamePad_Vibration();
	}

	m_Vibrations[user]->Set(user, leftPower, rightPower, time);
}

GamePad_Vibration::~GamePad_Vibration()
{

}

GamePad_Vibration::GamePad_Vibration()
	:m_UserIndex(0), m_Timer(-1.0f), m_Time(0.0f)
{
}


void GamePad_Vibration::Update()
{
	// マイナスなら稼働させない
	if (m_Time <= 0.0f)return;

	m_Timer += Time::DeltaTime;

	if (m_Timer >= m_Time)
	{
		XINPUT_VIBRATION vaibration = { 0,0 };

		XInputSetState(m_UserIndex, &vaibration);
		m_Timer = -1.0f;
		m_Time = 0.0f;
		m_LeftPower = 0.0f;
		m_RightPower = 0.0f;
	}
}

void GamePad_Vibration::Set(int userIndex, int leftPower, int rightPower, float time)
{


	bool isUpdate = false;

	if (leftPower >= m_LeftPower)
	{
		m_LeftPower = leftPower;
		isUpdate = true;
	}
	if (rightPower >= m_RightPower)
	{
		m_RightPower = rightPower;
		isUpdate = true;
	}

	// 現在のステートより強い振動だったら、初期化する
	if (isUpdate)
	{
		m_Time = time;
		m_UserIndex = userIndex;
		m_Timer = 0.0f;
	}

	XINPUT_VIBRATION vibration =
	{
		m_LeftPower,
		m_RightPower
	};

	XInputSetState(m_UserIndex, &vibration);
}

bool GamePad_Vibration::IsVibration()
{
	return m_Time != 0.0f;
}

float GamePad_Vibration::GetPower()
{
	return m_LeftPower + m_RightPower;
}
