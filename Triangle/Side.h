#pragma once

using namespace std;

class Side
{
public:
	Vec2i point;
	int idx;
	Gradient grad;
	bool isConnected;

	Side()
	{
		this->isConnected = false;
		this->idx = 0;
		this->grad.resetGradient();
	}

	void setPoint(Vec2i p)
	{
		this->point[0] = p[0];
		this->point[1] = p[1];
	}

	void setPoint(int x, int y)
	{
		this->point[0] = x;
		this->point[1] = y;
	}
};
