#ifndef _GPUPARTICLE_ACTIONMANAGER_H_
#define _GPUPARTICLE_ACTIONMANAGER_H_

#include <string>
#include <SimpleMath.h>
#include <d3d12.h>
#include <DirectXMath.h>
#include <map>
#include <wrl/client.h>
#include <memory>
#include <SimpleMath.h>

#include "../../Utility/Color.h"
#include "../DirectX/Core/Model/MeshDrawer.h"
#include "../DirectX/Core/Effect.h"
#include "../DirectX/Core/PSOManager.h"
#include "../Texture.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

class Buffer;
class Camera;
class Timer;
class Random;
class FluidParticleAction;
class ParticleSequence;
class ParticleAction;

class ParticleActionManager
{
public:
	


	//float collisionDistance;
//float fluidDensity;
//float domainSphereRadius;

	ParticleActionManager();
	~ParticleActionManager();
	void AddAction(std::shared_ptr<ParticleAction> action, const std::string& actionName);
	void AddSequence(std::shared_ptr<ParticleSequence> sequence);
	void Update();
	void DeleteSequence();
	void DeleteAction(const std::string& actionName);
	ID3D12Resource* GetBuffer(const std::string& actionName);
	std::shared_ptr<ParticleAction> GetAction(const std::string& actionName);

private:
	std::map<std::string,std::shared_ptr<ParticleAction>> m_Actions;
	std::vector<std::shared_ptr<ParticleSequence>> m_Sequences;
};

#endif