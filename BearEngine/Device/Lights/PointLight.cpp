#include "PointLight.h"

#include "LightManager.h"
#include "imgui/imgui.h"

PointLight::PointLight(const SimpleMath::Vector3& position, const SimpleMath::Color& color, float distance, float decay)
	:_position(position), _color(color), _distance(distance), _decay(decay)
{
}

void PointLight::UpdatePointLight(const SimpleMath::Vector3& position, const SimpleMath::Color& color, float distance,
	float decay)
{
	_position = position;
	_color = color;
	_distance = distance;
	_decay = decay;

	_isUpdate = true;
}

const PointLight::ConstPointLightDatas PointLight::CreateLightData()
{
	return ConstPointLightDatas{ _position,0,_color,_distance,_decay };
}

bool PointLight::DebugRender()
{
	bool isUpdate = false;

	float pos_[3] = { _position.x,_position.y,_position.z };

	if (ImGui::DragFloat3("LightPosition", pos_, 0.01f, -30.0f, 30.0f))
	{
		_position = SimpleMath::Vector3(pos_);

		isUpdate = true;
	}

	return isUpdate;
}

