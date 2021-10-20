#pragma once

#include <d3d12.h>

#include "Device/DirectX/Core/Buffer.h"

struct DXRInstance
{
	D3D12_RAYTRACING_GEOMETRY_DESC _geometryDesc;
	std::shared_ptr<Buffer> _vertexBuffer;
	std::shared_ptr<Buffer> _indexBuffer;

	// 現状はマテリアル情報なし
	
};
