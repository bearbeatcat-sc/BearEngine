#pragma once

#include <DirectXMath.h>
#include <SimpleMath.h>
#include <vector>
#include <memory>

using namespace DirectX;

class Plane;

class Camera
{
public:
	Camera();
	Camera(const SimpleMath::Vector3& pos, const SimpleMath::Vector3& target, const SimpleMath::Vector3& up = SimpleMath::Vector3(0, 1, 0));
	~Camera();
	void SetPosition(XMFLOAT3 pos);
	void SetTarget(XMFLOAT3 target);
	void SetUp(XMFLOAT3 up);
	void SetPosition(const SimpleMath::Vector3& pos);
	void SetTarget(const SimpleMath::Vector3& target);
	void SetUp(const SimpleMath::Vector3& up);
	void SetNear(float Near);
	void SetFar(float Far);
	void SetFov(float fov);
	void SetSpriteScale(float scale);
	float GetNear();
	float GetFar();
	float GetFov();
	float GetAspect();
	float GetSpriteScale();
	const SimpleMath::Matrix GetBillBoardMat();
	const SimpleMath::Matrix GetYAxisBillBoardMat();
	const SimpleMath::Matrix GetProjectMat();
	const SimpleMath::Matrix GetViewMat();
	const SimpleMath::Vector3& GetPosition();
	const SimpleMath::Vector3& GetTarget();
	const SimpleMath::Vector3& GetUp();
	void Update();
	std::vector<std::shared_ptr<SimpleMath::Plane>> GetFrustum();
	void SetAspect(float aspect);

private:
	std::vector<std::shared_ptr<SimpleMath::Plane>> CreateFrustum();

private:
	SimpleMath::Vector3 m_Pos;
	SimpleMath::Vector3 m_Target;
	SimpleMath::Vector3 m_Up;
	SimpleMath::Matrix matBillBoard;
	SimpleMath::Matrix matYAxisBillBoard;
	float m_near;
	float m_far;
	float m_Aspect;
	float m_Fov;

};

