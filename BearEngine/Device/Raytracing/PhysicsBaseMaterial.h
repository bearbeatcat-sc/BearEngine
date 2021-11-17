#pragma once

#include "SimpleMath.h"
using namespace DirectX;

struct PhysicsBaseMaterial
{
public:
	SimpleMath::Vector4 _albedo;
	SimpleMath::Vector4 _metallic;
	float _roughness;
	float _transmission;
	float _refract;
	float _pad;

	PhysicsBaseMaterial(const SimpleMath::Vector4& albedo, const SimpleMath::Vector4& metallic, float roughness, float transmission = 1.0f, float refract = 1.0f)
		:_albedo(albedo), _roughness(roughness), _metallic(metallic), _transmission(transmission), _refract(refract)
	{

	}

	PhysicsBaseMaterial()
	{
		_albedo = SimpleMath::Vector4(1, 1, 1, 1);
		_metallic = SimpleMath::Vector4(1, 1, 1, 1);
		_roughness = 0.5f;
		_transmission = 1.0f;
		_refract = 0.0f;
	}
};