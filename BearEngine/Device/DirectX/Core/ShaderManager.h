#ifndef _SHADERMANAGER_H_
#define _SHADERMANAGER_H_
#include "../../Singleton.h"
#include <d3d12.h>
#include <wrl/client.h>
#include <string>
#include <map>

using Microsoft::WRL::ComPtr;


class ShaderManager
	:public Singleton<ShaderManager>
{
public:
	friend class Singleton<ShaderManager>;
	bool LoadShader(const wchar_t* fileName, const char* target, const std::string& shaderName, const char* entryPoint = "main");
	ID3DBlob* GetShader(const std::string& shaderName);

protected:
	ShaderManager();
	~ShaderManager();

private:
	bool LoadShader(const wchar_t* fileName, const char* target,ID3DBlob** blob,const char* entryPoint);

private:
	std::map<std::string, ComPtr<ID3DBlob>> m_Shaders;
};

#endif 