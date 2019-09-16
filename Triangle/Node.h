#pragma once

#include <opencv2/opencv.hpp>

using namespace std;

class Node
{
public:
	Vec3b color;
	ColorNode* pNext;
	Side* leftSide, * rightSide;
	int idRow;

	Node()
	{
		// reset to first column
		this->leftSide = new Side();
		this->rightSide = new Side();
		this->pNext = NULL;
		this->color = Vec3b(0, 0, 0);
		this->idRow = 0;
	}

	void setColor(uchar a, uchar b, uchar c)
	{
		this->color[0] = a;
		this->color[1] = b;
		this->color[2] = c;
	}

	void setColor(Vec3b& cc)
	{
		this->color[0] = cc[0];
		this->color[1] = cc[1];
		this->color[2] = cc[2];
	}
};
