#include "ShaderManager.h"
#include <d3dcompiler.h>

bool ShaderManager::LoadShader(const wchar_t* fileName, const char* target, const std::string& shaderName, const char* entryPoint)
{
	ComPtr<ID3DBlob> temp;

	if (!LoadShader(fileName, target, &temp,entryPoint))
	{
		return false;
	}

	m_Shaders.emplace(shaderName, temp);

	return true;
}

ID3DBlob* ShaderManager::GetShader(const std::string& shaderName)
{
	if (m_Shaders.find(shaderName) == m_Shaders.end()) return nullptr;
	return m_Shaders.at(shaderName).Get();
}

ShaderManager::ShaderManager()
{
}

ShaderManager::~ShaderManager()
{
	//for (auto itr = m_Shaders.begin(); itr != m_Shaders.end(); itr++)
	//{
	//	(*itr).second->Release();
	//}

	m_Shaders.clear();
}

bool ShaderManager::LoadShader(const wchar_t* fileName, const char* target, ID3DBlob** blob, const char* entryPoint)
{
	ComPtr<ID3DBlob> errorBuffer = nullptr;

	if (FAILED(D3DCompileFromFile(fileName, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, target, D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, blob, &errorBuffer)))
	{
		if (errorBuffer)
		{
			OutputDebugStringA(static_cast<char*>(errorBuffer->GetBufferPointer()));
		}

		return false;
	}

	return true;
}
