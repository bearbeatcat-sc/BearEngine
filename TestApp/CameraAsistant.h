#ifndef _CAMERA_ASISTANT_H_
#define _CAMERA_ASISTANT_H_

#include <memory>

class Camera;

class CameraAsistant
{
public:
	CameraAsistant();
	~CameraAsistant();
	void Update();

private:
	std::shared_ptr<Camera> m_Camera;
};

#endif