#pragma once
#include "MeshDatas.h"

struct Material
{
	XMFLOAT3 ambient;
	float pad1;
	XMFLOAT3 diffuse;
	float pad2;
	XMFLOAT3 specular;

	Material()
	{
		ambient = { 0.3f,0.3f,0.3f };
		diffuse = { 0.0f,0.0f,0.0f };
		specular = { 1.0f,1.0f,1.0f };
	}
};

struct MaterialData
{
	Material material;
	std::string texFilePath;
	unsigned short indexCount;
	unsigned short m_StartIndex;

	bool operator<(const MaterialData& right) const
	{
		return m_StartIndex < right.m_StartIndex;
	}
};
