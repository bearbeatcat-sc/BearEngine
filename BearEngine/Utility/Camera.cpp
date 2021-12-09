#include "Camera.h"

#include "../Device/DirectX/DirectXInput.h"
#include "../Device/WindowApp.h"
#include "Math/Plane.h"
#include "Math/MathUtility.h"

#include <iostream>

Camera::Camera()
	:m_Pos(SimpleMath::Vector3(0,0,0)), m_Target(SimpleMath::Vector3(0,0,1)), m_Up(SimpleMath::Vector3(0,1,0))
{
	m_near = 0.1f;
	m_far = 300.0f;

	m_Fov = XMConvertToRadians(60.0f);
	auto windowSize = WindowApp::GetInstance().GetWindowSize();
	m_Aspect = (float)windowSize.window_Width / (float)windowSize.window_Height;
}

Camera::Camera(const SimpleMath::Vector3& pos, const SimpleMath::Vector3& target, const SimpleMath::Vector3& up)
	:m_Pos(pos),m_Target(target),m_Up(up)
{
	m_near = 0.1f;
	m_far = 300.0f;

	m_Fov = XMConvertToRadians(60.0f);
	auto windowSize = WindowApp::GetInstance().GetWindowSize();
	m_Aspect = (float)windowSize.window_Width / (float)windowSize.window_Height;
}

Camera::~Camera()
{
}

void Camera::SetPosition(XMFLOAT3 pos)
{
	m_Pos = pos;
}

void Camera::SetTarget(XMFLOAT3 target)
{
	m_Target = target;
}

void Camera::SetUp(XMFLOAT3 up)
{
	m_Up = up;
}

void Camera::SetPosition(const SimpleMath::Vector3& pos)
{
	m_Pos = XMFLOAT3(pos.x, pos.y, pos.z);
}

void Camera::SetTarget(const SimpleMath::Vector3& target)
{
	m_Target = XMFLOAT3(target.x, target.y, target.z);
}

void Camera::SetUp(const SimpleMath::Vector3& up)
{
	m_Up = XMFLOAT3(up.x, up.y, up.z);
}

void Camera::SetNear(float Near)
{
	m_near = Near;
}

void Camera::SetFar(float Far)
{
	m_far = Far;
}

void Camera::SetFov(float fov)
{
	m_Fov = fov;
}

float Camera::GetNear()
{
	return m_near;
}

float Camera::GetFar()
{
	return m_far;
}

float Camera::GetFov()
{
	return m_Fov;
}

float Camera::GetAspect()
{
	return m_Aspect;
}

const SimpleMath::Matrix Camera::GetBillBoardMat()
{
	XMVECTOR eyePosition = XMLoadFloat3(&m_Pos);
	XMVECTOR targetPosition = XMLoadFloat3(&m_Target);
	XMVECTOR upVector = XMLoadFloat3(&m_Up);

	// カメラのZ軸
	XMVECTOR cameraAxisZ = XMVectorSubtract(targetPosition, eyePosition);
	cameraAxisZ = XMVector3Normalize(cameraAxisZ); // 正規化

	// カメラのX軸
	XMVECTOR cameraAxisX;
	cameraAxisX = XMVector3Cross(upVector, cameraAxisZ);
	cameraAxisX = XMVector3Normalize(cameraAxisX);

	// カメラのY軸
	XMVECTOR cameraAxisY;
	cameraAxisY = XMVector3Cross(cameraAxisZ, cameraAxisX);
	cameraAxisY = XMVector3Normalize(cameraAxisY);

	// カメラ回転行列
	XMMATRIX matCameraRot;
	matCameraRot.r[0] = cameraAxisX;
	matCameraRot.r[1] = cameraAxisY;
	matCameraRot.r[2] = cameraAxisZ;
	matCameraRot.r[3] = XMVectorSet(0, 0, 0, 1);

	// 逆行列を計算
	XMMATRIX matView = XMMatrixTranspose(matCameraRot);

	XMVECTOR reverseEyePosition = XMVectorNegate(eyePosition);

	// カメラの位置からワールド原点へのベクトル
	XMVECTOR tX = XMVector3Dot(cameraAxisX, reverseEyePosition);
	XMVECTOR tY = XMVector3Dot(cameraAxisY, reverseEyePosition);
	XMVECTOR tZ = XMVector3Dot(cameraAxisZ, reverseEyePosition);

	XMVECTOR translation = XMVectorSet(tX.m128_f32[0], tY.m128_f32[1], tZ.m128_f32[2], 1.0f);

	matView.r[3] = translation;

	XMMATRIX mat;
	mat = XMMatrixIdentity();
	mat.r[0] = cameraAxisX;
	mat.r[1] = cameraAxisY;
	mat.r[2] = cameraAxisZ;
	mat.r[3] = XMVectorSet(0, 0, 0, 1);

	return mat;
}

const SimpleMath::Matrix Camera::GetYAxisBillBoardMat()
{
	XMVECTOR eyePosition = XMLoadFloat3(&m_Pos);
	XMVECTOR targetPosition = XMLoadFloat3(&m_Target);
	XMVECTOR upVector = XMLoadFloat3(&m_Up);

	// カメラのZ軸
	XMVECTOR cameraAxisZ = XMVectorSubtract(targetPosition, eyePosition);
	cameraAxisZ = XMVector3Normalize(cameraAxisZ); // 正規化

	// カメラのX軸
	XMVECTOR cameraAxisX;
	cameraAxisX = XMVector3Cross(upVector, cameraAxisZ);
	cameraAxisX = XMVector3Normalize(cameraAxisX);

	// カメラのY軸
	XMVECTOR cameraAxisY;
	cameraAxisY = XMVector3Cross(cameraAxisZ, cameraAxisX);
	cameraAxisY = XMVector3Normalize(cameraAxisY);

	// カメラ回転行列
	XMMATRIX matCameraRot;
	matCameraRot.r[0] = cameraAxisX;
	matCameraRot.r[1] = cameraAxisY;
	matCameraRot.r[2] = cameraAxisZ;
	matCameraRot.r[3] = XMVectorSet(0, 0, 0, 1);

	// 逆行列を計算
	XMMATRIX matView = XMMatrixTranspose(matCameraRot);

	XMVECTOR reverseEyePosition = XMVectorNegate(eyePosition);

	// カメラの位置からワールド原点へのベクトル
	XMVECTOR tX = XMVector3Dot(cameraAxisX, reverseEyePosition);
	XMVECTOR tY = XMVector3Dot(cameraAxisY, reverseEyePosition);
	XMVECTOR tZ = XMVector3Dot(cameraAxisZ, reverseEyePosition);

	XMVECTOR translation = XMVectorSet(tX.m128_f32[0], tY.m128_f32[1], tZ.m128_f32[2], 1.0f);

	matView.r[3] = translation;

	XMVECTOR yBillCameraAxisX, yBillCameraAxisY, yBillCameraAxisZ;

	yBillCameraAxisX = cameraAxisX;
	yBillCameraAxisY = XMVector3Normalize(upVector);
	yBillCameraAxisZ = XMVector3Cross(yBillCameraAxisX, yBillCameraAxisY);

	XMMATRIX mat;
	mat = XMMatrixIdentity();
	mat.r[0] = yBillCameraAxisX;
	mat.r[1] = yBillCameraAxisY;
	mat.r[2] = yBillCameraAxisZ;
	mat.r[3] = XMVectorSet(0, 0, 0, 1);

	
	return mat;
}

const SimpleMath::Matrix  Camera::GetProjectMat()
{

	//return SimpleMath::Matrix::CreatePerspectiveFieldOfView(
	//	m_Fov,
	//	m_Aspect,
	//	m_near,
	//	m_far);
	
	return XMMatrixPerspectiveFovLH(m_Fov,
		m_Aspect,
		m_near,
		m_far
	);

	//return SimpleMath::Matrix::CreatePerspectiveFieldOfView(		
	//	m_Fov,
	//	m_Aspect,
	//	m_near,
	//	m_far
	//);
}

const SimpleMath::Matrix Camera::GetViewMat()
{
	//XMVECTOR eyePosition = XMLoadFloat3(&m_Pos);
	//XMVECTOR targetPosition = XMLoadFloat3(&m_Target);
	//XMVECTOR upVector = XMLoadFloat3(&m_Up);

	//// カメラのZ軸
	//XMVECTOR cameraAxisZ = XMVectorSubtract(targetPosition, eyePosition);
	//cameraAxisZ = XMVector3Normalize(cameraAxisZ); // 正規化

	//// カメラのX軸
	//XMVECTOR cameraAxisX;
	//cameraAxisX = XMVector3Cross(upVector, cameraAxisZ);
	//cameraAxisX = XMVector3Normalize(cameraAxisX);

	//// カメラのY軸
	//XMVECTOR cameraAxisY;
	//cameraAxisY = XMVector3Cross(cameraAxisZ, cameraAxisX);
	//cameraAxisY = XMVector3Normalize(cameraAxisY);

	//// カメラ回転行列
	//XMMATRIX matCameraRot;
	//matCameraRot.r[0] = cameraAxisX;
	//matCameraRot.r[1] = cameraAxisY;
	//matCameraRot.r[2] = cameraAxisZ;
	//matCameraRot.r[3] = XMVectorSet(0, 0, 0, 1);

	//// 逆行列を計算
	//XMMATRIX matView = XMMatrixTranspose(matCameraRot);

	//XMVECTOR reverseEyePosition = XMVectorNegate(eyePosition);

	//// カメラの位置からワールド原点へのベクトル
	//XMVECTOR tX = XMVector3Dot(cameraAxisX, reverseEyePosition);
	//XMVECTOR tY = XMVector3Dot(cameraAxisY, reverseEyePosition);
	//XMVECTOR tZ = XMVector3Dot(cameraAxisZ, reverseEyePosition);

	//XMVECTOR translation = XMVectorSet(tX.m128_f32[0], tY.m128_f32[1], tZ.m128_f32[2], 1.0f);

	//matView.r[3] = translation;

	return XMMatrixLookAtLH(XMLoadFloat3(&m_Pos), XMLoadFloat3(&m_Target), XMLoadFloat3(&m_Up));

	//return matView;
}

const 	SimpleMath::Vector3& Camera::GetPosition()
{
	return m_Pos;
}

const 	SimpleMath::Vector3& Camera::GetTarget()
{
	return m_Target;
}

const 	SimpleMath::Vector3& Camera::GetUp()
{
	return m_Up;
}

void Camera::Update()
{
}

std::vector<std::shared_ptr<SimpleMath::Plane>> Camera::GetFrustum()
{
	return CreateFrustum();
}

void Camera::SetAspect(float aspect)
{
	m_Aspect = aspect;
}

std::vector< std::shared_ptr<SimpleMath::Plane>> Camera::CreateFrustum()
{
	// 視錐台を構成するプレーン
	std::vector<std::shared_ptr<SimpleMath::Plane>> planes;
	planes.resize(6);

	planes[0] = std::make_shared<SimpleMath::Plane>();
	planes[1] = std::make_shared<SimpleMath::Plane>();
	planes[2] = std::make_shared<SimpleMath::Plane>();
	planes[3] = std::make_shared<SimpleMath::Plane>();
	planes[4] = std::make_shared<SimpleMath::Plane>();
	planes[5] = std::make_shared<SimpleMath::Plane>();

	float zMinimum, r;
	SimpleMath::Matrix matrix;

	auto projectMat = SimpleMath::Matrix(GetProjectMat());
	auto viewMat = SimpleMath::Matrix(GetViewMat());

	// 錐台の最小Z nearを計算
	zMinimum = -projectMat._43 / projectMat._33;
	r = m_far / (m_far - zMinimum);

	// 更新後のProjectMatで錘台用のMatrixを作成
	matrix = viewMat * projectMat;

	// 前面
	planes[0]->x = matrix._14 + matrix._13;
	planes[0]->y = matrix._24 + matrix._23;
	planes[0]->z = matrix._34 + matrix._33;
	planes[0]->w = matrix._44 + matrix._43;
	planes[0]->Normalize();

	// 奥
	planes[1]->x = matrix._14 - matrix._13;
	planes[1]->y = matrix._24 - matrix._23;
	planes[1]->z = matrix._34 - matrix._33;
	planes[1]->w = matrix._44 - matrix._43;
	planes[1]->Normalize();

	// 左
	planes[2]->x = matrix._14 + matrix._11;
	planes[2]->y = matrix._24 + matrix._21;
	planes[2]->z = matrix._34 + matrix._31;
	planes[2]->w = matrix._44 + matrix._41;
	planes[2]->Normalize();

	//右
	planes[3]->x = matrix._14 - matrix._11;
	planes[3]->y = matrix._24 - matrix._21;
	planes[3]->z = matrix._34 - matrix._31;
	planes[3]->w = matrix._44 - matrix._41;
	planes[3]->Normalize();

	// 上
	planes[4]->x = matrix._14 - matrix._12;
	planes[4]->y = matrix._24 - matrix._22;
	planes[4]->z = matrix._34 - matrix._32;
	planes[4]->w = matrix._44 - matrix._42;
	planes[4]->Normalize();

	// 下
	planes[5]->x = matrix._14 + matrix._12;
	planes[5]->y = matrix._24 + matrix._22;
	planes[5]->z = matrix._34 + matrix._32;
	planes[5]->w = matrix._44 + matrix._42;
	planes[5]->Normalize();


	return planes;
}
