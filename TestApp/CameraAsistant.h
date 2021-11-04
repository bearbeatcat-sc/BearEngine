#ifndef _CAMERA_ASISTANT_H_
#define _CAMERA_ASISTANT_H_

#include <memory>
#include <SimpleMath.h>

class Camera;

class CameraAsistant
{
public:
	CameraAsistant();
	~CameraAsistant();
	void Update();

private:
	void ViewMove(const DirectX::SimpleMath::Vector3& moveVec);
	void PositionMove(const DirectX::SimpleMath::Vector3& moveVec);

private:
	std::shared_ptr<Camera> m_Camera;

	float _Pitch;
	float _Yaw;
};

#endif