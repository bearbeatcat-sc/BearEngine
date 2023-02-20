#include "ObjLoader.h"
#include <fstream>
#include <vector>
#include "../MeshManager.h"

ObjLoader::ObjLoader()
{
}

ObjLoader::~ObjLoader()
{

}

void ObjLoader::loadObj(MeshData::ModelData& modelData, const std::string& filePath, const std::string& fileName)
{
	std::string mtlFileName;
	std::string materialName;

	std::ifstream file;

	char buf[256] = { 0 };

	std::vector<XMFLOAT3> positions;
	std::vector<XMFLOAT3> normals;
	//std::vector<XMFLOAT2> texCoords;
	//std::vector<unsigned short> indices;
	std::unordered_map<std::string, std::vector<unsigned short>> index_list;
	std::string curret_matName = "";
	//std::vector<MeshData::Vertex> vertices;
	bool m_IsFindMtl = false;

	int indexArray[6] = { 0,1,2,0,2,3 };


	file.open(filePath + fileName, std::ios::in);

	if (!file.is_open())
	{
		return;
	}

	for (;;)
	{
		file >> buf;

		if (!file)
		{
			break;
		}



		// 頂点情報の読み込み
		if (0 == strcmp(buf, "v"))
		{
			float x, y, z;
			file >> x >> y >> z;

			XMFLOAT3 position;
			position.x = x;
			position.y = y;
			position.z = z;

			positions.push_back(position);
		}

		if (0 == strcmp(buf, "vn"))
		{
			float x, y, z;
			file >> x >> y >> z;

			XMFLOAT3 normal;
			normal.x = x;
			normal.y = y;
			normal.z = z;

			normals.push_back(normal);
		}

		if (0 == strcmp(buf, "vt"))
		{
			float x;
			float y;

			file >> x >> y;

			modelData.texCords.push_back(XMFLOAT2(x, y));
		}

		if (0 == strcmp(buf, "f"))
		{
			// 効率悪いかも
			if (!m_IsFindMtl)
			{
				m_IsFindMtl = true;
				curret_matName = "Mat" + std::to_string((index_list.size() + 1));
				index_list.emplace(curret_matName, std::vector<unsigned short>());
			}

			MeshData::Vertex vertex;

			vertex.uv = XMFLOAT2(0, 0);

			// 面の情報たち
			unsigned int p[4] = { -1 },
				t[4] = { -1 },
				n[4] = { -1 };


			unsigned int pos_id;
			unsigned int tex_id;
			unsigned int normal_id;
			int count = 0;

			for (int i = 0; i < 4; i++)
			{
				count++;

				file >> pos_id;

				if (pos_id == 0) continue;

				// 頂点の情報をIDを使って取得
				vertex.pos = positions[pos_id - 1];
				p[i] = pos_id - 1;


				if ('/' == file.peek())
				{
					file.ignore();

					if ('/' != file.peek())
					{
						file >> tex_id;
						vertex.uv = modelData.texCords[tex_id - 1];
						t[i] = tex_id - 1;
					}

					if ('/' == file.peek())
					{
						file.ignore();

						file >> normal_id;
						vertex.normal = normals[normal_id - 1];
						n[i] = normal_id - 1;
					}
				}

				if (i < 3)
				{
					modelData.vertices.push_back(vertex);
					int index = modelData.vertices.size() - 1;
					modelData.m_Indices.push_back(index);
					index_list.at(curret_matName).push_back(index);
				}



				if ('\n' == file.peek())
				{
					break;
				}
			}

			if (count > 3)
			{
				for (int iFace = 1; iFace < 4; iFace++)
				{
					int j = (iFace + 1) % 4;

					vertex.pos = positions[p[j]];
					vertex.normal = normals[n[j]];
					vertex.uv = modelData.texCords[t[j]];

					modelData.vertices.push_back(vertex);
					int index = modelData.vertices.size() - 1;
					modelData.m_Indices.push_back(index);
					index_list.at(curret_matName).push_back(index);
				}
			}
		}

		if (0 == strcmp(buf, "mtllib"))
		{
			file >> mtlFileName;

		}


		if (0 == strcmp(buf, "usemtl"))
		{
			file >> curret_matName;

			if (curret_matName == "")
			{
				curret_matName = "Mat" + std::to_string((index_list.size() + 1));
			}

			index_list.emplace(curret_matName, std::vector<unsigned short>());
			m_IsFindMtl = true;
		}


	}

	std::unordered_map<std::string,MaterialData> mats = loadMtl(filePath, mtlFileName);

	// インデックスの数をマテリアル情報に紐付け
	for (auto itr = mats.begin(); itr != mats.end(); itr++)
	{
		(*itr).second.indexCount = index_list.at((*itr).first).size();
		(*itr).second.m_StartIndex = index_list.at((*itr).first).front();
	}



	// マテリアル情報がない場合、強制的にマテリアルを設定
	if (mtlFileName == "" || mats.size() == 0)
	{
		MaterialData mat;
		mat.material.ambient = XMFLOAT3(1.0f, 1.0f, 1.0f);
		mat.material.diffuse = XMFLOAT3(1.0f, 1.0f, 1.0f);
		mat.m_StartIndex = index_list.begin()->second.front();
		mat.indexCount = index_list.begin()->second.size();
		mats.emplace(index_list.begin()->first, mat);
	}


	modelData.m_MaterialDatas = mats;

	return;
}

std::unordered_map<std::string,MaterialData> ObjLoader::loadMtl(const std::string& filePath, const std::string& fileName)
{
	std::unordered_map<std::string,MaterialData> matDatas;
	std::string curret_material_name = "";


	std::ifstream file;

	char buf[256] = { 0 };

	file.open(filePath + fileName, std::ios::in);

	if (!file.is_open())
	{
		return matDatas;
	}


	for (;;)
	{
		file >> buf;

		if (!file)
		{
			break;
		}


		if (0 == strcmp(buf, "newmtl"))
		{
			std::string matName;
			file >> matName;
			curret_material_name = matName;

			if (curret_material_name == "")
			{
				curret_material_name = "Mat" + (matDatas.size() + 1);
			}

			matDatas.emplace(curret_material_name, MaterialData());
		}

		if (0 == strcmp(buf, "Ka"))
		{
			float r, g, b;
			r = g = b = 1.0f;
			file >> r >> g >> b;
			matDatas.at(curret_material_name).material.ambient = XMFLOAT3(r, g, b);

		}

		if (0 == strcmp(buf, "Kd"))
		{
			float r, g, b;
			r = g = b = 1.0f;
			file >> r >> g >> b;
			matDatas.at(curret_material_name).material.diffuse = XMFLOAT3(r, g, b);
		}

		if (0 == strcmp(buf, "Ks"))
		{
			float r, g, b;
			r = g = b = 1.0f;
			file >> r >> g >> b;
			matDatas.at(curret_material_name).material.specular = XMFLOAT3(r, g, b);
		}

		if (0 == strcmp(buf, "map_Kd"))
		{
			std::string texturePath;
			file >> texturePath;
			matDatas.at(curret_material_name).texFilePath = filePath + texturePath;
		}
	}


	return matDatas;
}

