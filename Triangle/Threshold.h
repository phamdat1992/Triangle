#pragma once

using namespace std;

class Threshold
{
public:
	float gmax, gmin;

	void reset()
	{
		this->gmax = numeric_limits<float>::max();
		this->gmin = numeric_limits<float>::min();
	}
};
