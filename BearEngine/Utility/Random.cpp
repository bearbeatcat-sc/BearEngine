#include "Random.h"

int Random::GetRandom(int min, int max)
{
	std::random_device rd;
	std::mt19937 mt(rd());

	std::uniform_int_distribution<int> temp(min, max);

	return temp(mt);
}

float Random::GetRandom(float min, float max)
{
	std::random_device rd;
	std::mt19937 mt(rd());

	std::uniform_real_distribution<float> temp(min, max);

	return temp(mt);
}
