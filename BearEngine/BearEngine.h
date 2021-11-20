#ifndef _BEAR_ENGINE_H_
#define _BEAR_ENGINE_H_


class BearEngine
{
public:
	BearEngine();
	~BearEngine();
	void InitEngine();
	void EngineUpdate();
	void BeginRender();
	void EndRender();

private:
	void RenderApplication();

	BearEngine(const BearEngine& b);
	BearEngine& operator= (const BearEngine& b);

};

#endif