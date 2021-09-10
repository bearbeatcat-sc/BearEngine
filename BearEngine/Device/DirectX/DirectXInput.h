#pragma once
#include <dinput.h>
#include <wrl/client.h>
#include <Xinput.h>
#include <vector>
#include <SimpleMath.h>

#include "../Singleton.h"

#define DIRECTINPUT_VERSION 0x0800

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"xinput.lib")

using Microsoft::WRL::ComPtr;

enum GamePad_ThubStick
{
	GAMEPAD_ThubStick_LX,
	GAMEPAD_ThubStick_LY,
	GAMEPAD_ThubStick_RX,
	GAMEPAD_ThubStick_RY,
};

enum GamePad_VirtualButtons
{
	GAMEPAD_V_BUTTON_UP = 0x0001,
	GAMEPAD_V_BUTTON_DOWN = 0x0002,
	GAMEPAD_V_BUTTON_LEFT = 0x0004,
	GAMEPAD_V_BUTTON_RIGHT = 0x0008,

	GAMEPAD_V_BUTTON_START = 0x0010,
	GAMEPAD_V_BUTTON_BACK = 0x0020,

	GAMEPAD_V_BUTTON_LEFT_THUB = 0x0040,
	GAMEPAD_V_BUTTON_RIGHT_THUB = 0x0080,

	GAMEPAD_V_BUTTON_LEFT_SHOULDER = 0x0100,
	GAMEPAD_V_BUTTON_RIGHT_SHOULDER = 0x0200,

	GAMEPAD_V_BUTTON_A = 0x1000,
	GAMEPAD_V_BUTTON_B = 0x2000,
	GAMEPAD_V_BUTTON_X = 0x4000,
	GAMEPAD_V_BUTTON_Y = 0x8000,
};

enum GamePad_Buttons
{
	GAMEPAD_BUTTON_UP,
	GAMEPAD_BUTTON_DOWN,
	GAMEPAD_BUTTON_LEFT,
	GAMEPAD_BUTTON_RIGHT,

	GAMEPAD_BUTTON_START,
	GAMEPAD_BUTTON_BACK,

	GAMEPAD_BUTTON_LEFT_THUB,
	GAMEPAD_BUTTON_RIGHT_THUB,

	GAMEPAD_BUTTON_LEFT_SHOULDER,
	GAMEPAD_BUTTON_RIGHT_SHOULDER,

	GAMEPAD_BUTTON_A,
	GAMEPAD_BUTTON_B,
	GAMEPAD_BUTTON_X,
	GAMEPAD_BUTTON_Y,
};

enum GamePad_Triggers
{
	GamePad_LeftTrigger,
	GamePad_RightTrigger,
};

class GamePad_Vibration
{
public:
	GamePad_Vibration();
	~GamePad_Vibration();
	void Update();
	void Set(int userIndex, int leftPower, int rightPower, float time);
	void Add(int userIndex, int leftPower, int rightPower, float time);
	bool IsVibration();
	float GetPower();

private:
	int m_UserIndex;
	float m_LeftPower;
	float m_RightPower;
	float m_Time;
	float m_Timer;
};

enum class MouseButton {
	LEFT = 0,
	RIGHT = 1,
	CENTER = 2,
};

struct TriggerState
{
	bool RightTriggerFlag;
	bool LeftTriggerFlag;
};

class DirectXInput
	:public Singleton<DirectXInput>
{
public:
	friend class Singleton <DirectXInput>;
	bool InitDirectInput();

	void UpdateInput();
	bool IsKeyDown(int keyNum);
	bool IsKey(int keyNum);
	bool IsMouseButtonDown(MouseButton button);
	bool IsMouseButton(MouseButton button);
	bool IsMouseButtonUp(MouseButton button);
	float GetWheelValue();
	bool IsActiveGamePad();
	DirectX::SimpleMath::Vector2 GetMouseMove();
	DirectX::SimpleMath::Vector2 GetCursorPos();

	float GetGamePadValue(GamePad_ThubStick stick);
	bool IsDownTrigger(GamePad_Triggers trigger);
	bool IsUpTrigger(GamePad_Triggers trigger);
	float IsDownTriggerValue(GamePad_Triggers trigger);
	bool IsTrigger(GamePad_Triggers trigger);
	bool isButtonDown(GamePad_Buttons button);
	bool isButton(GamePad_Buttons button);
	void OnVibration(int user, int leftPower, int rightPower, float time);
	void VibrationUpdate();
	float GetVibration(int user);

protected:
	DirectXInput();
	virtual ~DirectXInput();
	bool CreateKeyBoardDevice();
	bool CreateMouseDevice();
	bool UpdateDirectInput();
	bool UpdateXINPUT();


private:
	ComPtr<IDirectInputDevice8> m_Devkeyboard = nullptr;
	ComPtr<IDirectInputDevice8> m_MouseDevice = nullptr;
	ComPtr<IDirectInput8> m_pDevice = nullptr;
	bool m_PreviousKeyState[256];
	bool m_CurrentKeyState[256];

private:
	XINPUT_STATE m_State;

	TriggerState m_currentTriggerState;
	TriggerState m_previousTriggerState;

	std::vector<GamePad_VirtualButtons> m_Buttons;
	std::vector < GamePad_Vibration* > m_Vibrations;
	bool m_CurrentButtonState[14];
	bool m_PreviousButtonState[14];
};

