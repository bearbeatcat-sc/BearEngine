#ifndef _SCENE_H_
#define _SCENE_H_

class Scene
{
public:
	virtual ~Scene() {}
	virtual void Update() = 0;
	virtual void ShutDown() = 0;
	virtual void Init() = 0;

private:
};

#endif