#ifndef _MESHCOMPONENT_H_
#define _MESHCOMPONENT_H_

#include "Component.h"
#include "../Device/DirectX/Core/Model/MeshManager.h"
#include "../Device/DirectX/Core/Model/MeshDrawer.h"
#include "../Device/DirectX/Core/Model/Mesh.h"
#include <memory>
#include "../Utility/Color.h"
#include "SimpleMath.h"
#include <memory>

class Light;
class Camera;

class MeshComponent
	:public Component
{
public:

	MeshComponent(Actor* user, const std::string& modelName, std::shared_ptr<Camera> camera, const std::string& effectName, int updateOrder = 100);
	MeshComponent(Actor* user, std::shared_ptr<Camera> camera,const std::string& effectName,int updateOrder = 100);
	MeshComponent(Actor* user, std::shared_ptr<Camera> camera,const SimpleMath::Vector3* points, const std::string& effectName,int updateOrder = 100);
	MeshComponent(Actor* user, std::shared_ptr<Camera> camera,size_t tessellation, const std::string& effectName,int updateOrder = 100);
	~MeshComponent();

	virtual void Update() override;
	//void SetPosition(Vector3 pos);
	//void SetScale(Vector3 scale);
	//void SetRotation(Matrix4 mat);
	void SetMatrix(const SimpleMath::Matrix& mat);
	void SetPosition(const SimpleMath::Vector3& pos);
	void SetScale(const DirectX::SimpleMath::Vector3& scale);
	void SetRotation(const DirectX::SimpleMath::Quaternion& axis);
	void SetDrawType(MeshDrawer::DrawType drawType);

	void SetColor(const SimpleMath::Color& color);
	void SetEffect(const std::string& effectName);
	void SetVisible(bool flag);

private:
	std::shared_ptr<Mesh> m_Mesh;
	std::shared_ptr<Camera> m_Camera;

	SimpleMath::Vector3 m_Position;
	DirectX::SimpleMath::Vector3 m_Scale;
	DirectX::SimpleMath::Quaternion m_Rotation;

};

#endif