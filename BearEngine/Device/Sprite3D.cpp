#include "Sprite3D.h"
#include "DirectX/Core/Buffer.h"
#include "../Utility/Camera.h"
#include "../Utility/CameraManager.h"
#include <SimpleMath.h>

Sprite3D::Sprite3D(XMFLOAT2 anchorpoint, const std::string& effectName)
	:Sprite(anchorpoint,effectName), m_BillBoardType(BillBoard_Type_None)
{
	m_Camera = CameraManager::GetInstance().GetMainCamera();
}

Sprite3D::Sprite3D(XMFLOAT2 anchorpoint, const std::string& effectName, const std::string& cameraName)
	: Sprite(anchorpoint, effectName), m_BillBoardType(BillBoard_Type_None)
{
	m_Camera = CameraManager::GetInstance().GetCamera(cameraName);
}

Sprite3D::~Sprite3D()
{
}

void Sprite3D::Draw(ID3D12GraphicsCommandList* tempCommand)
{
	auto billMat = DirectX::SimpleMath::Matrix::Identity;

	if (m_BillBoardType == BillBoardType::BillBoard_Type_Normal)
	{
		billMat = m_Camera->GetBillBoardMat();
	}
	else if (m_BillBoardType == BillBoardType::BillBoard_Type_Y)
	{
		billMat = m_Camera->GetYAxisBillBoardMat();
	}

	auto vpMat = m_Camera->GetViewMat() * m_Camera->GetProjectMat();

	XMMATRIX worldMat = DirectX::SimpleMath::Matrix::Identity;
	worldMat *= billMat;
	worldMat *= DirectX::SimpleMath::Matrix::CreateScale(m_Scale);
	worldMat *= DirectX::SimpleMath::Matrix::CreateRotationX(m_Rotate.x) * DirectX::SimpleMath::Matrix::CreateRotationY(m_Rotate.y) * DirectX::SimpleMath::Matrix::CreateRotationZ(m_Rotate.z);
	worldMat *= DirectX::SimpleMath::Matrix::CreateTranslation(m_pos);

	XMMATRIX mat = XMMatrixIdentity();
	mat = SimpleMath::Matrix(worldMat) * vpMat;

	ConstBufferData* constMap = nullptr;
	m_ConstantBuffer->getBuffer()->Map(0, nullptr, (void**)&constMap);
	constMap->color = m_Color;
	constMap->mat = XMMatrixIdentity();
	constMap->mat = mat;
	m_ConstantBuffer->getBuffer()->Unmap(0, nullptr);

	DrawCall(tempCommand);
}

void Sprite3D::SetScale(SimpleMath::Vector3 scale)
{
	m_Scale = scale;
}

void Sprite3D::SetRotate(SimpleMath::Vector3 rotate)
{
	m_Rotate = rotate;
}

void Sprite3D::SetBillBoardType(BillBoardType billBoardType)
{
	m_BillBoardType = billBoardType;
}

