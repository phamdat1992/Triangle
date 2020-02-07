#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <list>
#include <limits>
#include <algorithm>
#include <fstream>

using namespace std;
using namespace cv;

const double ALPHA_J = 10.0;
const double EPS = 0.001;

class Point2D
{
public:
	bool isVertice;
	int x, y;
	double gl, gr;

	Point2D()
	{
		this->isVertice = true;
		this->gl = numeric_limits<double>::min();
		this->gr = numeric_limits<double>::max();
		this->x = -1;
		this->y = -1;
	}

	void assign(Point2D* p)
	{
		this->x = p->x;
		this->y = p->y;
	}

	void assign(int x, int y)
	{
		this->x = x;
		this->y = y;
	}
};

class ColorNode
{
public:
	int l, r;
	int color;
	int linkLeftUp, linkLeftDown;
	int linkRightUp, LinkRightDown;

	Point2D pL, pR;

	ColorNode(int il, int ir, int ic)
	{
		this->l = il;
		this->r = ir;
		this->color = ic;
	}

	ColorNode()
	{
	}
};

class Triangle
{
public:
	Point2D p1, p2, p3;

	Triangle(Point2D x, Point2D y, Point2D z)
	{
		this->p1.x = x.x;
		this->p1.y = x.y;
		
		this->p2.x = y.x;
		this->p2.y = y.y;

		this->p3.x = z.x;
		this->p3.y = z.y;
	}
};

class Edge
{
public:
	Point2D p1, p2;
	double hsg;
	bool isMin;
	int pNext;
	int flag;

	Edge()
	{
		this->flag = -1;
		this->pNext = -1;
		this->isMin = false;
		this->hsg = 0.0;
	}

	void setEdge(int currentImageId, int pNext, double hsg, bool isMin, Point2D* p1, Point2D* p2)
	{
		this->flag = currentImageId;
		this->pNext = pNext;

		this->hsg = hsg;
		this->isMin = isMin;

		this->p1.assign(p1);
		this->p2.assign(p2);
	}

	void setEdge(int currentImageId, int pNext, double hsg, bool isMin, int x1, int y1, int x2, int y2)
	{
		this->flag = currentImageId;
		this->pNext = pNext;

		this->hsg = hsg;
		this->isMin = isMin;

		this->p1.assign(x1, y1);
		this->p2.assign(x2, y2);
	}
};

class EdgeNode
{
public:
	int edgeID;
	int l, r;
	int color;
	int linkLeftDown, LinkRightDown;

	EdgeNode()
	{
		this->edgeID = -1;
		this->l = this->r = this->color = -1;
	}

	void setColorNode(ColorNode* node)
	{
		this->l = node->l;
		this->r = node->r;
		this->linkLeftDown = node->linkLeftDown;
		this->LinkRightDown = node->LinkRightDown;
		this->color = this->color;
	}
};

class EdgeStructure
{
public:
	int headStack;
	Edge* stackEdge;

	void init(int width)
	{
		this->headStack = 0;
		this->stackEdge = new Edge[width];
	}

	~EdgeStructure()
	{
		delete[] stackEdge;
	}

	int getEdgeFromStack(int currentImageId)
	{
		int edgeId = headStack;

		if (stackEdge[headStack].flag != currentImageId)
		{
			++headStack;
		}
		else
		{
			headStack = stackEdge[headStack].pNext;
		}

		stackEdge[edgeId].flag = currentImageId;
		stackEdge[edgeId].pNext = -1;

		return edgeId;
	}

	Edge* getEdge(int edgeId)
	{
		return &this->stackEdge[edgeId];
	}

	void addEdge(int edgeId, int currentImageId)
	{
		stackEdge[edgeId].flag = currentImageId;
		stackEdge[edgeId].pNext = headStack;
		headStack = edgeId;
	}
};

void readImage(Mat& image, const string& path)
{
	image = imread(path, IMREAD_COLOR);

	if (image.empty())
	{
		cout << "Image not found" << endl;
	}
}

int main(int argc, char* argv[])
{
	string imageName;
	Mat inImg, outImg;
	ofstream log;
	log.open("log.txt");

	readImage(inImg, "./cluster.bmp");
	outImg = inImg.clone();

	list<Triangle> triangles;
	ColorNode* f1 = new ColorNode[inImg.cols];
	ColorNode* f2 = new ColorNode[inImg.cols];

	EdgeNode* cc1 = new EdgeNode[inImg.cols];
	EdgeNode* cc2 = new EdgeNode[inImg.cols];

	EdgeStructure edgeStack;
	edgeStack.init(inImg.cols);

	int currentImageId = 0;
	int lenc1 = 0, lenc2 = 0;

	ColorNode* l1 = f1;
	ColorNode* l2 = f2;

	f1->pL.x = f1->pL.y = -1;
	f1->pR.x = f1->pR.y = -1;
	f1->color = -1;

	for (int row = 0; row < inImg.rows; ++row)
	{
		log << "row " << row << endl;
		/* 
		if (row == 935)
		{
			cout << "test";
		}
		*/
		const uchar* ptr = inImg.ptr(row);

		l2 = f2;
		l2->l = l2->r = 0;
		l2->color = *ptr;
		l2->pL.x = l2->pR.x = 0;
		l2->pL.y = l2->pR.y = row;
		l2->pL.gl = numeric_limits<double>::lowest();
		l2->pL.gr = numeric_limits<double>::max();
		l2->pR.gl = numeric_limits<double>::lowest();
		l2->pR.gr = numeric_limits<double>::max();
		l2->pL.isVertice = l2->pR.isVertice = true;
		l2->linkLeftUp = l2->linkLeftDown = l2->linkRightUp = l2->LinkRightDown = -1;

		for (int col = 0; col < inImg.cols; ++col)
		{
			int cc = *ptr;
			if (cc != l2->color)
			{
				++l2;
				l2->l = col;
				l2->pL.x = col;
				l2->pL.y = row;
				l2->color = cc;
				
				l2->pL.gl = numeric_limits<double>::lowest();
				l2->pL.gr = numeric_limits<double>::max();
				l2->pR.gl = numeric_limits<double>::lowest();
				l2->pR.gr = numeric_limits<double>::max();
				l2->pL.isVertice = l2->pR.isVertice = true;
				l2->linkLeftUp = l2->linkLeftDown = l2->linkRightUp = l2->LinkRightDown = -1;
			}

			l2->r = col;
			l2->pR.x = col;
			l2->pR.y = row;

			ptr += 3;
		}

		int len1 = l1 - f1;
		int len2 = l2 - f2;

		/*
		log << "l1" << endl;
		for (int i1 = 0; i1 <= len1; ++i1)
		{
			log << f1[i1].l << " " << f1[i1].r << endl;
			log << f1[i1].pL.x << " " << f1[i1].pL.y << " " << f1[i1].pL.gl << " " << f1[i1].pL.gr << endl;
			log << f1[i1].pR.x << " " << f1[i1].pR.y << " " << f1[i1].pR.gl << " " << f1[i1].pR.gr << endl;
		}

		log << "l2" << endl;
		for (int i2 = 0; i2 <= len2; ++i2)
		{
			log << f2[i2].l << " " << f2[i2].r << endl;
			log << f2[i2].pL.x << " " << f2[i2].pL.y << " " << f2[i2].pL.gl << " " << f2[i2].pL.gr << endl;
			log << f2[i2].pR.x << " " << f2[i2].pR.y << " " << f2[i2].pR.gl << " " << f2[i2].pR.gr << endl;
		}
		*/

		for (int i1 = 0, i2 = 0; i1 <= len1 && i2 <= len2; ++i2)
		{
			for (; i1 <= len1 && f1[i1].r < f2[i2].l; ++i1);
			for (; i1 <= len1 && f1[i1].color != f2[i2].color && f1[i1].r <= f2[i2].r; ++i1);

			if (i1 <= len1 && i2 <= len2 && f1[i1].l <= f2[i2].r && f2[i2].l <= f1[i1].r)
			{
				if (f1[i1].color == f2[i2].color && f1[i1].pL.x != -1 && f1[i1].pL.y != -1)
				{
					f2[i2].pL.x = f1[i1].pR.x;
					f2[i2].pL.y = f1[i1].pR.y;

					f2[i2].linkLeftUp = i1;
					f1[i1].linkLeftDown = i2;

					if (f1[i1].pL.x != f1[i1].l || f1[i1].pL.y != row - 1)
					{
						f1[i1].pL.isVertice = false;
					}

					double dx = (double)f2[i2].l - (double)f2[i2].pL.x;
					double dy = (double)row - (double)f2[i2].pL.y;
					double da = dx / dy;

					if (f1[i1].pL.gl <= da && da <= f1[i1].pL.gr)
					{
						double dm = (dy - ALPHA_J >= 0) ? (dy - ALPHA_J) : 0;

						double ss1 = min((dx - ALPHA_J) / (dm + EPS), (dx - ALPHA_J) / (dy + ALPHA_J + EPS));
						double ss2 = min((dx + ALPHA_J) / (dm + EPS), (dx + ALPHA_J) / (dy + ALPHA_J + EPS));

						double fmin = min(ss1, ss2);

						ss1 = max((dx - ALPHA_J) / (dm + EPS), (dx - ALPHA_J) / (dy + ALPHA_J + EPS));
						ss2 = max((dx + ALPHA_J) / (dm + EPS), (dx + ALPHA_J) / (dy + ALPHA_J + EPS));

						double fmax = max(ss1, ss2);


						f2[i2].pL.gl = max(f1[i1].pL.gl, fmin);
						f2[i2].pL.gr = min(f1[i1].pL.gr, fmax);
					}
					else 
					{
						f2[i2].pL.gl = 1.0;
						f2[i2].pL.gr = -1.0;
					}

					if (f2[i2].pL.gl > f2[i2].pL.gr)
					{
						f2[i2].pL.x = f1[i1].l;
						f2[i2].pL.y = row - 1;

						f1[i1].pL.isVertice = true;

						dx = (double)f2[i2].l - (double)f2[i2].pL.x;
						dy = (double)row - (double)f2[i2].pL.y;

						double dm = (dy - ALPHA_J >= 0) ? (dy - ALPHA_J) : 0;

						double ss1 = min((dx - ALPHA_J) / (dm + EPS), (dx - ALPHA_J) / (dy + ALPHA_J + EPS));
						double ss2 = min((dx + ALPHA_J) / (dm + EPS), (dx + ALPHA_J) / (dy + ALPHA_J + EPS));

						double fmin = min(ss1, ss2);

						ss1 = max((dx - ALPHA_J) / (dm + EPS), (dx - ALPHA_J) / (dy + ALPHA_J + EPS));
						ss2 = max((dx + ALPHA_J) / (dm + EPS), (dx + ALPHA_J) / (dy + ALPHA_J + EPS));

						double fmax = max(ss1, ss2);
						f2[i2].pL.gl = fmin;
						f2[i2].pL.gr = fmax;
					}

					++i1;
				}
			}
		}
		
		for (int i1 = len1, i2 = len2; i1 >= 0 && i2 >= 0; --i2)
		{
			for (; i1 >= 0 && f1[i1].l > f2[i2].r; --i1);
			for (; i1 >= 0 && f1[i1].color != f2[i2].color && f1[i1].l >= f2[i2].l; --i1);

			if (i1 >= 0 && i2 >= 0 && f1[i1].r >= f2[i2].l && f2[i2].r >= f1[i1].l)
			{
				if (f1[i1].color == f2[i2].color && f1[i1].pR.x != -1 && f1[i1].pR.y != -1)
				{
					f2[i2].pR.x = f1[i1].pR.x;
					f2[i2].pR.y = f1[i1].pR.y;
					f2[i2].linkRightUp = i1;
					f1[i1].LinkRightDown = i2;

					double dx = (double)f2[i2].r - (double)f2[i2].pR.x;
					double dy = (double)row - (double)f2[i2].pR.y;
					double da = dx / dy;

					if (f1[i1].pR.x != f1[i1].r || f1[i1].pR.y != row - 1)
					{
						f1[i1].pR.isVertice = false;
					}

					if (f1[i1].pR.gl <= da && da <= f1[i1].pR.gr)
					{
						double dm = (dy - ALPHA_J >= 0) ? (dy - ALPHA_J) : 0;

						double ss1 = min((dx - ALPHA_J) / (dm + EPS), (dx - ALPHA_J) / (dy + ALPHA_J + EPS));
						double ss2 = min((dx + ALPHA_J) / (dm + EPS), (dx + ALPHA_J) / (dy + ALPHA_J + EPS));

						double fmin = min(ss1, ss2);

						ss1 = max((dx - ALPHA_J) / (dm + EPS), (dx - ALPHA_J) / (dy + ALPHA_J + EPS));
						ss2 = max((dx + ALPHA_J) / (dm + EPS), (dx + ALPHA_J) / (dy + ALPHA_J + EPS));

						double fmax = max(ss1, ss2);

						f2[i2].pR.gl = max(f1[i1].pR.gl, fmin);
						f2[i2].pR.gr = min(f1[i1].pR.gr, fmax);
					}
					else
					{
						f2[i2].pR.gl = 1.0;
						f2[i2].pR.gr = -1.0;
					}

					if (f2[i2].pR.gl > f2[i2].pR.gr)
					{
						f2[i2].pR.x = f1[i1].r;
						f2[i2].pR.y = row - 1;

						f1[i1].pR.isVertice = true;

						dx = (double)f2[i2].r - (double)f2[i2].pR.x;
						dy = (double)row - (double)f2[i2].pR.y;

						double dm = (dy - ALPHA_J >= 0) ? (dy - ALPHA_J) : 0;

						double ss1 = min((dx - ALPHA_J) / (dm + EPS), (dx - ALPHA_J) / (dy + ALPHA_J + EPS));
						double ss2 = min((dx + ALPHA_J) / (dm + EPS), (dx + ALPHA_J) / (dy + ALPHA_J + EPS));

						double fmin = min(ss1, ss2);

						ss1 = max((dx - ALPHA_J) / (dm + EPS), (dx - ALPHA_J) / (dy + ALPHA_J + EPS));
						ss2 = max((dx + ALPHA_J) / (dm + EPS), (dx + ALPHA_J) / (dy + ALPHA_J + EPS));

						double fmax = max(ss1, ss2);
						f2[i2].pR.gl = fmin;
						f2[i2].pR.gr = fmax;
					}

					--i1;
				}
			}
		}

		// print keypoints
		/*
		for (int i = 0; i <= len1; ++i)
		{
			if (f1[i].pL.isVertice)
			{
				cout << f1[i].l << " " << row - 1 << endl;

				Point centerCircle(f1[i].l, row - 1);
				int radius = 5;
				Scalar colorCircle(0, 0, 255);
				circle(outImg, centerCircle, radius, colorCircle, FILLED);
			}

			if (f1[i].pR.isVertice)
			{
				cout << f1[i].r << " " << row - 1 << endl;
				Point centerCircle(f1[i].r, row - 1);
				int radius = 5;
				Scalar colorCircle(0, 0, 255);
				circle(outImg, centerCircle, radius, colorCircle, FILLED);
			}
		}
		*/

		int lenc2 = len1;
		for (int idLeftDown = 0, idLeftUp, idRightUp, idRightDown; idLeftDown <= len1; ++idLeftDown)
		{
			idLeftUp = f1[idLeftDown].linkLeftUp;
			cc2[idLeftDown].setColorNode(&f1[idLeftDown]);

			if (idLeftUp == -1)
			{
				// add edge
				cc2[idLeftDown].edgeID = edgeStack.getEdgeFromStack(currentImageId);
				edgeStack.getEdge(cc2[idLeftDown].edgeID)->setEdge(
					currentImageId,
					-1,
					((double)f1[idLeftDown].pR.x - f1[idLeftDown].pL.x) / ((double)EPS),
					false,
					&f1[idLeftDown].pL,
					&f1[idLeftDown].pR
				);
			} 
			else
			{
				idRightUp = f1[idLeftDown].linkRightUp;
				if (f1[idLeftDown].linkRightUp == -1)
				{
					idRightDown = cc1[idLeftUp].LinkRightDown;
				}
				else
				{
					idRightDown = idLeftDown;
				}

				for (int j = idLeftUp + 1, indexCur = cc1[idLeftUp].edgeID; j <= idRightUp; ++j)
				{
					if (cc1[j].color == f1[idLeftDown].color)
					{
						while (edgeStack.getEdge(indexCur)->pNext != -1)
						{
							indexCur = edgeStack.getEdge(indexCur)->pNext;
						}

						int newEdgeId = edgeStack.getEdgeFromStack(currentImageId);
						edgeStack.getEdge(newEdgeId)->setEdge(
							currentImageId,
							cc1[j].edgeID,
							((double)cc1[j].l - cc1[j - 1].r) / ((double)EPS),
							false,
							cc1[j - 1].r,
							row -2,
							cc1[j].l,
							row - 2
						);

						edgeStack.getEdge(indexCur)->pNext = newEdgeId;
					}

				}
				
				for (int j = idLeftDown; j <= idRightDown; ++j)
				{
					if (cc1[idLeftUp].color != f1[j].color)
					{
						// add edge
						cc2[j].edgeID = edgeStack.getEdgeFromStack(currentImageId);
						edgeStack.getEdge(cc2[j].edgeID)->setEdge(
							currentImageId,
							-1,
							((double)f1[j].pR.x - f1[j].pL.x) / ((double)EPS),
							false,
							&f1[j].pL,
							&f1[j].pR
						);
					}
					else
					{
						if (f1[j].pL.isVertice)
						{
							for (; cc1[idLeftUp].edgeID != -1; )
							{
								Edge* curEdge = edgeStack.getEdge(cc1[idLeftUp].edgeID);
								double hsg = ((double)f1[j].pL.x - curEdge->p1.x) / ((double)f1[j].pL.y - curEdge->p1.y);
								if ((curEdge->isMin && hsg >= curEdge->hsg) || (!curEdge->isMin && hsg <= curEdge->hsg))
								{
									triangles.push_back(
										Triangle(
											curEdge->p1,
											curEdge->p2,
											f1[j].pL
										)
									);

									int curEdgeId = cc1[idLeftUp].edgeID;
									cc1[idLeftUp].edgeID = curEdge->pNext;
									edgeStack.addEdge(curEdgeId, currentImageId);
								}
								else
								{
									int newEdgeId = edgeStack.getEdgeFromStack(currentImageId);

									edgeStack.getEdge(newEdgeId)->setEdge(
										currentImageId,
										cc1[idLeftUp].edgeID,
										((double)f1[j].pL.x - curEdge->p1.x) / ((double)f1[j].pL.y - curEdge->p1.y + EPS),
										true,
										&f1[j].pL,
										&curEdge->p1
									);
									cc1[idLeftUp].edgeID = newEdgeId;

									break;
								}
							}
						}

						if (f1[j].pR.isVertice)
						{
							for (; cc1[idLeftUp].edgeID != -1; )
							{
								Edge* curEdge = edgeStack.getEdge(cc1[idLeftUp].edgeID);
								double hsg = ((double)f1[j].pR.x - curEdge->p1.x) / ((double)f1[j].pR.y - curEdge->p1.y);
								if ((curEdge->isMin && hsg >= curEdge->hsg) || (!curEdge->isMin && hsg <= curEdge->hsg))
								{
									triangles.push_back(
										Triangle(
											curEdge->p1,
											curEdge->p2,
											f1[j].pR
										)
									);

									int curEdgeId = cc1[idLeftUp].edgeID;
									cc1[idLeftUp].edgeID = curEdge->pNext;
									edgeStack.addEdge(curEdgeId, currentImageId);
								}
								else
								{
									{
										int newEdgeId = edgeStack.getEdgeFromStack(currentImageId);
										edgeStack.getEdge(newEdgeId)->setEdge(
											currentImageId,
											-1,
											((double)f1[j].pR.x - curEdge->p1.x) / ((double)f1[j].pR.y - curEdge->p1.y + EPS),
											true,
											&f1[j].pL,
											&curEdge->p1
										);
										cc2[j].edgeID = newEdgeId;
									}

									{
										int newEdgeId = edgeStack.getEdgeFromStack(currentImageId);
										edgeStack.getEdge(newEdgeId)->setEdge(
											currentImageId,
											cc1[idLeftUp].edgeID,
											((double)f1[j].pR.x - curEdge->p1.x) / ((double)f1[j].pR.y - curEdge->p1.y + EPS),
											true,
											&f1[j].pL,
											&curEdge->p1
										);
										cc1[idLeftUp].edgeID = newEdgeId;
									}

									break;
								}
							}
						}
						else
						{
							cc2[j].edgeID = cc1[idLeftUp].edgeID;
							cc1[idLeftUp].edgeID = -1;
						}
					}
				}

				idLeftDown = idRightDown;
			}
		}

		// free unused edge
		for (int i = 0; i <= lenc1; ++i)
		{
			while (cc1[i].edgeID != -1)
			{
				int cur = edgeStack.getEdge(cc1[i].edgeID)->pNext;
				edgeStack.addEdge(cc1[i].edgeID, currentImageId);
				cc1[i].edgeID = cur;
			}
		}

		swap(f1, f2);
		swap(l1, l2);

		swap(cc1, cc2);
		swap(lenc1, lenc2);
	}

	log.close();
	imwrite("output.bmp", outImg);

	delete[] f1;
	delete[] f2;

	delete[] cc1;
	delete[] cc2;

	return 0;
}
