#pragma once

#include "Node.h"

using namespace std;

class ImageStructure
{
public:
	Node* head;
	int idRow;

	ImageStructure()
	{
		this->head = new Node();
		this->idRow = 0;
	}

	~ImageStructure()
	{
		Node* cur = head;
		while (this->head != NULL)
		{
			cur = head->pNext;
			delete head;
			head = cur;
		}
	}
};
