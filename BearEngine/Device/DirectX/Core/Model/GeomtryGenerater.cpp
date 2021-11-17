#include "GeomtryGenerater.h"

void GeometryGenerator::GenerateCubeMesh(MeshData::ModelData& modelData,DirectX::SimpleMath::Vector3 size)
{
	//Mesh* mesh = new Mesh();
	std::vector<MeshData::Vertex> vertices;
	std::vector<UINT> indices;
	std::unordered_map<std::string, MaterialData> m_Mats;
	std::string curretMatName = "Mat01";
	std::vector<XMFLOAT2> _uvs;

	Material mat;
	mat.ambient = XMFLOAT3(1.0f, 1.0f, 1.0f);
	mat.diffuse = XMFLOAT3(1.0f, 1.0f, 1.0f);


	MaterialData mat01;
	mat01.material = mat;

	vertices = {
		// ó†
		{ {-1.0f,-1.0f,-1.0f}, { 0.0f, 0.0f, -1.0f },{0.0f,0.0f} },
		{ {-1.0f, 1.0f,-1.0f}, { 0.0f, 0.0f, -1.0f }, {0.0f,0.0f} },
		{ { 1.0f, 1.0f,-1.0f}, { 0.0f, 0.0f, -1.0f },  {0.0f,0.0f}},
		{ { 1.0f,-1.0f,-1.0f}, { 0.0f, 0.0f, -1.0f },  {0.0f,0.0f}},
		// âE
		{ { 1.0f,-1.0f,-1.0f}, { 1.0f, 0.0f, 0.0f },  {0.0f,0.0f}},
		{ { 1.0f, 1.0f,-1.0f}, { 1.0f, 0.0f, 0.0f },{0.0f,0.0f} },
		{ { 1.0f, 1.0f, 1.0f}, { 1.0f, 0.0f, 0.0f }, {0.0f,0.0f}},
		{ { 1.0f,-1.0f, 1.0f}, { 1.0f, 0.0f, 0.0f }, {0.0f,0.0f}},
		// ç∂
		{ {-1.0f,-1.0f, 1.0f}, { -1.0f, 0.0f, 0.0f }, {0.0f,0.0f}},
		{ {-1.0f, 1.0f, 1.0f}, { -1.0f, 0.0f, 0.0f }, {0.0f,0.0f}},
		{ {-1.0f, 1.0f,-1.0f}, { -1.0f, 0.0f, 0.0f }, {0.0f,0.0f}},
		{ {-1.0f,-1.0f,-1.0f}, { -1.0f, 0.0f, 0.0f }, {0.0f,0.0f}},
		// ê≥ñ 
		{ { 1.0f,-1.0f, 1.0f}, { 0.0f, 0.0f, 1.0f}, {0.0f,0.0f}},
		{ { 1.0f, 1.0f, 1.0f}, { 0.0f, 0.0f, 1.0f}, {0.0f,0.0f}},
		{ {-1.0f, 1.0f, 1.0f}, { 0.0f, 0.0f, 1.0f}, {0.0f,0.0f}},
		{ {-1.0f,-1.0f, 1.0f}, { 0.0f, 0.0f, 1.0f}, {0.0f,0.0f}},
		// è„
		{ {-1.0f, 1.0f,-1.0f}, { 0.0f, 1.0f, 0.0f}, {0.0f,0.0f}},
		{ {-1.0f, 1.0f, 1.0f}, { 0.0f, 1.0f, 0.0f}, {0.0f,0.0f} },
		{ { 1.0f, 1.0f, 1.0f}, { 0.0f, 1.0f, 0.0f}, {0.0f,0.0f} },
		{ { 1.0f, 1.0f,-1.0f}, { 0.0f, 1.0f, 0.0f}, {0.0f,0.0f}},
		// íÍ
		{ {-1.0f,-1.0f, 1.0f}, { 0.0f, -1.0f, 0.0f}, {0.0f,0.0f}},
		{ {-1.0f,-1.0f,-1.0f}, { 0.0f, -1.0f, 0.0f}, {0.0f,0.0f}},
		{ { 1.0f,-1.0f,-1.0f}, { 0.0f, -1.0f, 0.0f}, {0.0f,0.0f}},
		{ { 1.0f,-1.0f, 1.0f}, { 0.0f, -1.0f, 0.0f}, {0.0f,0.0f}},
	};

	indices = {
	0, 1, 2, 2, 3,0,
	4, 5, 6, 6, 7,4,
	8, 9, 10, 10, 11, 8,
	12,13,14, 14,15,12,
	16,17,18, 18,19,16,
	20,21,22, 22,23,20,
	};

	
	//constexpr  int faceCount = 6;

	//XMVECTORF32 faceNoramls[faceCount] =
	//{
	//	{ { {  0,  0,  1, 0 } } },
	//	{ { {  0,  0, -1, 0 } } },
	//	{ { {  1,  0,  0, 0 } } },
	//	{ { { -1,  0,  0, 0 } } },
	//	{ { {  0,  1,  0, 0 } } },
	//	{ { {  0, -1,  0, 0 } } },
	//};

	//XMVECTORF32 uvs[4] =
	//{
	//	{ { { 1, 0, 0, 0 } } },
	//	{ { { 1, 1, 0, 0 } } },
	//	{ { { 0, 1, 0, 0 } } },
	//	{ { { 0, 0, 0, 0 } } },
	//};

	//XMVECTOR tSize = size;
	//tSize = XMVectorDivide(tSize, g_XMTwo);

	//for (int i = 0; i < faceCount; i++)
	//{
	//	auto normal = faceNoramls[i];

	//	XMVECTOR basis = (i >= 4) ? g_XMIdentityR2 : g_XMIdentityR1;

	//	XMVECTOR side1 = XMVector3Cross(normal, basis);
	//	XMVECTOR side2 = XMVector3Cross(normal, side1);


	//	size_t vbase = vertices.size();
	//	indices.push_back(vbase + 0);
	//	indices.push_back(vbase + 1);
	//	indices.push_back(vbase + 2);

	//	indices.push_back(vbase + 0);
	//	indices.push_back(vbase + 2);
	//	indices.push_back(vbase + 3);

	//	MeshData::Vertex vert;
	//	SimpleMath::Vector3 pos = XMVectorMultiply(XMVectorSubtract(XMVectorSubtract(normal, side1), side2), tSize);
	//	SimpleMath::Vector3 _noraml = SimpleMath::Vector3(normal);
	//	SimpleMath::Vector3 _uv = SimpleMath::Vector3(uvs[0]);
	//	vert.pos = pos;
	//	vert.normal = _noraml;
	//	vert.uv = XMFLOAT2(_uv.x, _uv.y);
	//	vertices.push_back(vert);


	//	pos = XMVectorMultiply(XMVectorAdd(XMVectorSubtract(normal, side1), side2), tSize);
	//	_noraml = SimpleMath::Vector3(normal);
	//	_uv = SimpleMath::Vector3(uvs[1]);
	//	vert.pos = pos;
	//	vert.normal = _noraml;
	//	vert.uv = XMFLOAT2(_uv.x, _uv.y);
	//	vertices.push_back(vert);


	//	pos = XMVectorMultiply(XMVectorAdd(normal, XMVectorAdd(side1, side2)), tSize);
	//	_noraml = SimpleMath::Vector3(normal);
	//	_uv = SimpleMath::Vector3(uvs[2]);
	//	vert.pos = pos;
	//	vert.normal = _noraml;
	//	vert.uv = XMFLOAT2(_uv.x, _uv.y);
	//	vertices.push_back(vert);


	//	pos = XMVectorMultiply(XMVectorSubtract(XMVectorAdd(normal, side1), side2), tSize);
	//	_noraml = SimpleMath::Vector3(normal);
	//	_uv = SimpleMath::Vector3(uvs[3]);
	//	vert.pos = pos;
	//	vert.normal = _noraml;
	//	vert.uv = XMFLOAT2(_uv.x, _uv.y);
	//	vertices.push_back(vert);
	//}

	//ReverseWinding(indices, vertices);
	//mesh->generateMesh(vertices, indices, test);
	mat01.m_StartIndex = 0;
	mat01.indexCount = indices.size();
	m_Mats.emplace(curretMatName, mat01);

	modelData.m_Indices = indices;
	modelData.vertices = vertices;
	modelData.m_MaterialDatas = m_Mats;
	


	//for (int i = 0; i < _countof(uvs); i++)
	//{
	//	SimpleMath::Vector3 _uv = SimpleMath::Vector3(uvs[i]);
	//	XMFLOAT2 temp = XMFLOAT2(_uv.x, _uv.z);
	//	_uvs.push_back(temp);
	//}

	modelData.texCords = _uvs;



	return;
}

void GeometryGenerator::GenerateCubeMeshDatas(MeshData::ModelData& modelData,DirectX::SimpleMath::Vector3 size)
{
	std::vector<MeshData::Vertex> vertices;
	std::vector<UINT> indices;
	std::unordered_map<std::string, MaterialData> m_Mats;
	std::string curretMatName = "Mat01";
	std::vector<XMFLOAT2> _uvs;

	Material mat;
	mat.ambient = XMFLOAT3(1.0f, 1.0f, 1.0f);
	mat.diffuse = XMFLOAT3(1.0f, 1.0f, 1.0f);


	MaterialData mat01;
	mat01.material = mat;


	constexpr  int faceCount = 6;

	XMVECTORF32 faceNoramls[faceCount] =
	{
		{ { {  0,  0,  1, 0 } } },
		{ { {  0,  0, -1, 0 } } },
		{ { {  1,  0,  0, 0 } } },
		{ { { -1,  0,  0, 0 } } },
		{ { {  0,  1,  0, 0 } } },
		{ { {  0, -1,  0, 0 } } },
	};

	XMVECTORF32 uvs[4] =
	{
		{ { { 1, 0, 0, 0 } } },
		{ { { 1, 1, 0, 0 } } },
		{ { { 0, 1, 0, 0 } } },
		{ { { 0, 0, 0, 0 } } },
	};

	XMVECTOR tSize = size;
	tSize = XMVectorDivide(tSize, g_XMTwo);

	for (int i = 0; i < faceCount; i++)
	{
		auto normal = faceNoramls[i];

		XMVECTOR basis = (i >= 4) ? g_XMIdentityR2 : g_XMIdentityR1;

		XMVECTOR side1 = XMVector3Cross(normal, basis);
		XMVECTOR side2 = XMVector3Cross(normal, side1);


		size_t vbase = vertices.size();
		indices.push_back(vbase + 0);
		indices.push_back(vbase + 1);
		indices.push_back(vbase + 2);

		indices.push_back(vbase + 0);
		indices.push_back(vbase + 2);
		indices.push_back(vbase + 3);

		MeshData::Vertex vert;
		SimpleMath::Vector3 pos = XMVectorMultiply(XMVectorSubtract(XMVectorSubtract(normal, side1), side2), tSize);
		SimpleMath::Vector3 _noraml = SimpleMath::Vector3(normal);
		SimpleMath::Vector3 _uv = SimpleMath::Vector3(uvs[0]);
		vert.pos = pos;
		vert.normal = _noraml;
		vert.uv = XMFLOAT2(_uv.x, _uv.y);
		vertices.push_back(vert);


		pos = XMVectorMultiply(XMVectorAdd(XMVectorSubtract(normal, side1), side2), tSize);
		_noraml = SimpleMath::Vector3(normal);
		_uv = SimpleMath::Vector3(uvs[1]);
		vert.pos = pos;
		vert.normal = _noraml;
		vert.uv = XMFLOAT2(_uv.x, _uv.y);
		vertices.push_back(vert);


		pos = XMVectorMultiply(XMVectorAdd(normal, XMVectorAdd(side1, side2)), tSize);
		_noraml = SimpleMath::Vector3(normal);
		_uv = SimpleMath::Vector3(uvs[2]);
		vert.pos = pos;
		vert.normal = _noraml;
		vert.uv = XMFLOAT2(_uv.x, _uv.y);
		vertices.push_back(vert);


		pos = XMVectorMultiply(XMVectorSubtract(XMVectorAdd(normal, side1), side2), tSize);
		_noraml = SimpleMath::Vector3(normal);
		_uv = SimpleMath::Vector3(uvs[3]);
		vert.pos = pos;
		vert.normal = _noraml;
		vert.uv = XMFLOAT2(_uv.x, _uv.y);
		vertices.push_back(vert);
	}

	//ReverseWinding(indices, vertices);
	//mesh->generateMesh(vertices, indices, test);
	mat01.m_StartIndex = 0;
	mat01.indexCount = indices.size();
	m_Mats.emplace(curretMatName, mat01);

	modelData.m_Indices = indices;
	modelData.vertices = vertices;
	modelData.m_MaterialDatas = m_Mats;



	for (int i = 0; i < _countof(uvs); i++)
	{
		SimpleMath::Vector3 _uv = SimpleMath::Vector3(uvs[i]);
		XMFLOAT2 temp = XMFLOAT2(_uv.x, _uv.z);
		_uvs.push_back(temp);
	}

	modelData.texCords = _uvs;
}

void GeometryGenerator::GenerateSpehereMesh(MeshData::ModelData& modelData,float diametro, size_t tessellation)
{
	//MeshData* mesh = new Mesh();
	std::vector<MeshData::Vertex> vertices;
	std::vector<UINT> indices;
	std::unordered_map<std::string, MaterialData> m_Mats;
	std::string curretMatName = "Mat01";
	std::vector<XMFLOAT2> _uvs;

	Material mat;
	mat.ambient = XMFLOAT3(1.0f, 1.0f, 1.0f);
	mat.diffuse = XMFLOAT3(1.0f, 1.0f, 1.0f);


	MaterialData mat01;
	mat01.material = mat;


	size_t verticalSegments = tessellation;
	size_t horizontalSegments = tessellation * 2;

	float radius = diametro / 2;

	for (size_t i = 0; i <= verticalSegments; i++)
	{
		float v = 1 - float(i) / float(verticalSegments);

		float latitude = (float(i) * XM_PI / float(verticalSegments)) - XM_PIDIV2;
		float dy, dxz;

		XMScalarSinCos(&dy, &dxz, latitude);

		// Create a single ring of vertices at this latitude.
		for (size_t j = 0; j <= horizontalSegments; j++)
		{
			float u = float(j) / float(horizontalSegments);

			float longitude = float(j) * XM_2PI / float(horizontalSegments);
			float dx, dz;

			XMScalarSinCos(&dx, &dz, longitude);

			dx *= dxz;
			dz *= dxz;

			XMVECTOR normal = XMVectorSet(dx, dy, dz, 0);
			XMVECTOR textureCoordinate = XMVectorSet(u, v, 0, 0);

			MeshData::Vertex vert;
			SimpleMath::Vector3 pos = XMVectorScale(normal, radius);
			SimpleMath::Vector3 _noraml = SimpleMath::Vector3(normal);
			SimpleMath::Vector3 _uv = SimpleMath::Vector3(textureCoordinate);

			vert.pos = pos;
			vert.normal = _noraml;
			vert.uv = SimpleMath::Vector2(_uv.x,_uv.y);
			_uvs.push_back(XMFLOAT2(_uv.x, _uv.y));

			vertices.push_back(vert);
		}
	}

	size_t stride = horizontalSegments + 1;

	for (size_t i = 0; i < verticalSegments; i++)
	{
		for (size_t j = 0; j <= horizontalSegments; j++)
		{
			size_t nextI = i + 1;
			size_t nextJ = (j + 1) % stride;

			indices.push_back(i * stride + j);
			indices.push_back(nextI * stride + j);
			indices.push_back(i * stride + nextJ);

			indices.push_back(i * stride + nextJ);
			indices.push_back(nextI * stride + j);
			indices.push_back(nextI * stride + nextJ);
		}
	}

	ReverseWinding(indices, vertices);
	//mesh->generateMesh(vertices, indices, test);
	mat01.m_StartIndex = 0;
	mat01.indexCount = indices.size();
	m_Mats.emplace(curretMatName, mat01);

	modelData.m_Indices = indices;
	modelData.vertices = vertices;
	modelData.m_MaterialDatas = m_Mats;

	//std::vector<XMFLOAT2> _uvs;


	//for (int i = 0; i < _countof(uvs); i++)
	//{
	//	SimpleMath::Vector3 _uv = SimpleMath::Vector3(uvs[0]);
	//	XMFLOAT2 temp = XMFLOAT2(_uv.x, _uv.z);
	//	_uvs.push_back(temp);
	//}

	modelData.texCords = _uvs;

	return ;
}

void GeometryGenerator::GenerateSquareMesh(MeshData::ModelData& modelData)
{
	std::vector<MeshData::Vertex> vertices;
	std::vector<UINT> indices;
	std::unordered_map<std::string, MaterialData> m_Mats;
	std::string curretMatName = "Mat01";
	std::vector<XMFLOAT2> _uvs;

	Material mat;
	mat.ambient = XMFLOAT3(1.0f, 1.0f, 1.0f);
	mat.diffuse = XMFLOAT3(1.0f, 1.0f, 1.0f);


	MaterialData mat01;
	mat01.material = mat;

	MeshData::Vertex vert0 = { {0,0,1},{-5.0f,-5.0f,0.0f},{0,1} };
	MeshData::Vertex vert1 = { {0,0,1},{-5.0f,+5.0f,0.0f},{0,0} };
	MeshData::Vertex vert2 = { {0,0,1},{+5.0f,-5.0f,0.0f},{1,1} };
	MeshData::Vertex vert3 = { {0,0,1},{+5.0f,+5.0f,0.0f},{1,0} };

	vertices.push_back(vert0);
	vertices.push_back(vert1);
	vertices.push_back(vert2);
	vertices.push_back(vert3);

	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(2);
	indices.push_back(1);
	indices.push_back(3);

	mat01.m_StartIndex = 0;
	mat01.indexCount = indices.size();
	m_Mats.emplace(curretMatName, mat01);

	modelData.m_Indices = indices;
	modelData.vertices = vertices;
	modelData.m_MaterialDatas = m_Mats;

	modelData.texCords = _uvs;

	return;
}

void GeometryGenerator::GenerateTriangleMesh(MeshData::ModelData& modelData, const DirectX::SimpleMath::Vector3* points)
{
	std::vector<MeshData::Vertex> vertices;
	std::vector<UINT> indices;
	std::unordered_map<std::string, MaterialData> m_Mats;
	std::string curretMatName = "Mat01";
	std::vector<XMFLOAT2> _uvs;

	Material mat;
	mat.ambient = XMFLOAT3(1.0f, 1.0f, 1.0f);
	mat.diffuse = XMFLOAT3(1.0f, 1.0f, 1.0f);

	for (int i = 0; i < 3; i++)
	{
		MeshData::Vertex vertex;
		vertex.pos = points[i];
		vertices.push_back(vertex);
		indices.push_back(i);
	}

	MaterialData mat01;
	mat01.m_StartIndex = 0;
	mat01.indexCount = indices.size();
	mat01.material = mat;
	m_Mats.emplace(curretMatName, mat01);



	modelData.m_Indices = indices;
	modelData.vertices = vertices;
	modelData.m_MaterialDatas = m_Mats;

	modelData.texCords = _uvs;
}

void GeometryGenerator::GenerateCubeDatas(std::vector<XMFLOAT3>& positions, std::vector<UINT>& indices, DirectX::SimpleMath::Vector3 size)
{
	constexpr  int faceCount = 6;

	XMVECTORF32 faceNoramls[faceCount] =
	{
		{ { {  0,  0,  1, 0 } } },
		{ { {  0,  0, -1, 0 } } },
		{ { {  1,  0,  0, 0 } } },
		{ { { -1,  0,  0, 0 } } },
		{ { {  0,  1,  0, 0 } } },
		{ { {  0, -1,  0, 0 } } },
	};

	XMVECTORF32 uvs[4] =
	{
		{ { { 1, 0, 0, 0 } } },
		{ { { 1, 1, 0, 0 } } },
		{ { { 0, 1, 0, 0 } } },
		{ { { 0, 0, 0, 0 } } },
	};

	XMVECTOR tSize = size;
	tSize = XMVectorDivide(tSize, g_XMTwo);

	for (int i = 0; i < faceCount; i++)
	{
		auto normal = faceNoramls[i];

		XMVECTOR basis = (i >= 4) ? g_XMIdentityR2 : g_XMIdentityR1;

		XMVECTOR side1 = XMVector3Cross(normal, basis);
		XMVECTOR side2 = XMVector3Cross(normal, side1);


		size_t vbase = positions.size();
		indices.push_back(vbase + 0);
		indices.push_back(vbase + 1);
		indices.push_back(vbase + 2);

		indices.push_back(vbase + 1);
		indices.push_back(vbase + 2);
		indices.push_back(vbase + 3);

		MeshData::Vertex vert;
		SimpleMath::Vector3 pos = XMVectorMultiply(XMVectorAdd(XMVectorSubtract(normal, side1), side2), tSize);
		positions.push_back(pos);

		pos = XMVectorMultiply(XMVectorSubtract(XMVectorSubtract(normal, side1), side2), tSize);
		positions.push_back(pos);

		pos = XMVectorMultiply(XMVectorAdd(normal, XMVectorAdd(side1, side2)), tSize);
		positions.push_back(pos);

		pos = XMVectorMultiply(XMVectorSubtract(XMVectorAdd(normal, side1), side2), tSize);
		positions.push_back(pos);

	}

	ReverseWinding(indices);

	return;
}

void GeometryGenerator::ReverseWinding(std::vector<UINT>& indicies, std::vector<MeshData::Vertex>& vertcies)
{
	for (auto it = indicies.begin(); it != indicies.end(); it += 3)
	{
		std::swap(*it, *(it + 2));
	}

	for (auto it = vertcies.begin(); it != vertcies.end(); ++it)
	{
		it->uv.x = (1.0f - it->uv.x);
	}
}

void GeometryGenerator::ReverseWinding(std::vector<UINT>& indicies)
{
	for (auto it = indicies.begin(); it != indicies.end(); it += 3)
	{
		std::swap(*it, *(it + 2));
	}


}
