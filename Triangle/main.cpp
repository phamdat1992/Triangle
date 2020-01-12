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
};

class ColorNode
{
public:
	int l, r;
	int color;
	Point2D pl, pr;

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

class Edge
{
public:
	Point2D p1, p2;
	double hsg;
	bool isMin;

	Edge()
	{
		this->isMin = false;
		this->hsg = 0.0;
	}
};

class EdgeNode
{
public:
	list<Edge> edges;
	int l, r;
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

	ColorNode* f1 = new ColorNode[inImg.cols];
	ColorNode* f2 = new ColorNode[inImg.cols];
	
	ColorNode* l1 = f1;
	ColorNode* l2 = f2;

	f1->pl.x = f1->pl.y = -1;
	f1->pr.x = f1->pr.y = -1;
	f1->color = -1;

	for (int row = 0; row < inImg.rows; ++row)
	{
		log << "row " << row << endl;
		if (row == 935)
		{
			cout << "test";
			//return 0;
		}
		const uchar* ptr = inImg.ptr(row);

		l2 = f2;
		l2->l = l2->r = 0;
		l2->color = *ptr;
		l2->pl.x = l2->pr.x = 0;
		l2->pl.y = l2->pr.y = row;
		l2->pl.gl = numeric_limits<double>::lowest();
		l2->pl.gr = numeric_limits<double>::max();
		l2->pr.gl = numeric_limits<double>::lowest();
		l2->pr.gr = numeric_limits<double>::max();
		l2->pl.isVertice = l2->pr.isVertice = true;

		for (int col = 0; col < inImg.cols; ++col)
		{
			int cc = *ptr;
			if (cc != l2->color)
			{
				++l2;
				l2->l = col;
				l2->pl.x = col;
				l2->pl.y = row;
				l2->color = cc;
				
				l2->pl.gl = numeric_limits<double>::lowest();
				l2->pl.gr = numeric_limits<double>::max();
				l2->pr.gl = numeric_limits<double>::lowest();
				l2->pr.gr = numeric_limits<double>::max();
				l2->pl.isVertice = l2->pr.isVertice = true;
			}

			l2->r = col;
			l2->pr.x = col;
			l2->pr.y = row;

			ptr += 3;
		}

		int len1 = l1 - f1;
		int len2 = l2 - f2;

		log << "l1" << endl;
		for (int i1 = 0; i1 <= len1; ++i1)
		{
			log << f1[i1].l << " " << f1[i1].r << endl;
			log << f1[i1].pl.x << " " << f1[i1].pl.y << " " << f1[i1].pl.gl << " " << f1[i1].pl.gr << endl;
			log << f1[i1].pr.x << " " << f1[i1].pr.y << " " << f1[i1].pr.gl << " " << f1[i1].pr.gr << endl;
		}

		log << "l2" << endl;
		for (int i2 = 0; i2 <= len2; ++i2)
		{
			log << f2[i2].l << " " << f2[i2].r << endl;
			log << f2[i2].pl.x << " " << f2[i2].pl.y << " " << f2[i2].pl.gl << " " << f2[i2].pl.gr << endl;
			log << f2[i2].pr.x << " " << f2[i2].pr.y << " " << f2[i2].pr.gl << " " << f2[i2].pr.gr << endl;
		}

		for (int i1 = 0, i2 = 0; i1 <= len1 && i2 <= len2; ++i2)
		{
			for (; i1 <= len1 && f1[i1].r < f2[i2].l; ++i1);
			for (; i1 <= len1 && f1[i1].color != f2[i2].color && f1[i1].r <= f2[i2].r; ++i1);

			if (i1 <= len1 && i2 <= len2 && f1[i1].l <= f2[i2].r && f2[i2].l <= f1[i1].r)
			{
				if (f1[i1].color == f2[i2].color && f1[i1].pl.x != -1 && f1[i1].pl.y != -1)
				{
					f2[i2].pl.x = f1[i1].pl.x;
					f2[i2].pl.y = f1[i1].pl.y;

					if (f1[i1].pl.x != f1[i1].l || f1[i1].pl.y != row - 1)
					{
						f1[i1].pl.isVertice = false;
					}

					double dx = (double)f2[i2].l - (double)f2[i2].pl.x;
					double dy = (double)row - (double)f2[i2].pl.y;
					double da = dx / dy;

					if (f1[i1].pl.gl <= da && da <= f1[i1].pl.gr)
					{
						double dm = (dy - ALPHA_J >= 0) ? (dy - ALPHA_J) : 0;

						double ss1 = min((dx - ALPHA_J) / (dm + EPS), (dx - ALPHA_J) / (dy + ALPHA_J + EPS));
						double ss2 = min((dx + ALPHA_J) / (dm + EPS), (dx + ALPHA_J) / (dy + ALPHA_J + EPS));

						double fmin = min(ss1, ss2);

						ss1 = max((dx - ALPHA_J) / (dm + EPS), (dx - ALPHA_J) / (dy + ALPHA_J + EPS));
						ss2 = max((dx + ALPHA_J) / (dm + EPS), (dx + ALPHA_J) / (dy + ALPHA_J + EPS));

						double fmax = max(ss1, ss2);


						f2[i2].pl.gl = max(f1[i1].pl.gl, fmin);
						f2[i2].pl.gr = min(f1[i1].pl.gr, fmax);
					}
					else 
					{
						f2[i2].pl.gl = 1.0;
						f2[i2].pl.gr = -1.0;
					}

					if (f2[i2].pl.gl > f2[i2].pl.gr)
					{
						f2[i2].pl.x = f1[i1].l;
						f2[i2].pl.y = row - 1;

						f1[i1].pl.isVertice = true;

						dx = (double)f2[i2].l - (double)f2[i2].pl.x;
						dy = (double)row - (double)f2[i2].pl.y;

						double dm = (dy - ALPHA_J >= 0) ? (dy - ALPHA_J) : 0;

						double ss1 = min((dx - ALPHA_J) / (dm + EPS), (dx - ALPHA_J) / (dy + ALPHA_J + EPS));
						double ss2 = min((dx + ALPHA_J) / (dm + EPS), (dx + ALPHA_J) / (dy + ALPHA_J + EPS));

						double fmin = min(ss1, ss2);

						ss1 = max((dx - ALPHA_J) / (dm + EPS), (dx - ALPHA_J) / (dy + ALPHA_J + EPS));
						ss2 = max((dx + ALPHA_J) / (dm + EPS), (dx + ALPHA_J) / (dy + ALPHA_J + EPS));

						double fmax = max(ss1, ss2);
						f2[i2].pl.gl = fmin;
						f2[i2].pl.gr = fmax;
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
				if (f1[i1].color == f2[i2].color && f1[i1].pr.x != -1 && f1[i1].pr.y != -1)
				{
					f2[i2].pr.x = f1[i1].pr.x;
					f2[i2].pr.y = f1[i1].pr.y;

					double dx = (double)f2[i2].r - (double)f2[i2].pr.x;
					double dy = (double)row - (double)f2[i2].pr.y;
					double da = dx / dy;

					if (f1[i1].pr.x != f1[i1].r || f1[i1].pr.y != row - 1)
					{
						f1[i1].pr.isVertice = false;
					}

					if (f1[i1].pr.gl <= da && da <= f1[i1].pr.gr)
					{
						double dm = (dy - ALPHA_J >= 0) ? (dy - ALPHA_J) : 0;

						double ss1 = min((dx - ALPHA_J) / (dm + EPS), (dx - ALPHA_J) / (dy + ALPHA_J + EPS));
						double ss2 = min((dx + ALPHA_J) / (dm + EPS), (dx + ALPHA_J) / (dy + ALPHA_J + EPS));

						double fmin = min(ss1, ss2);

						ss1 = max((dx - ALPHA_J) / (dm + EPS), (dx - ALPHA_J) / (dy + ALPHA_J + EPS));
						ss2 = max((dx + ALPHA_J) / (dm + EPS), (dx + ALPHA_J) / (dy + ALPHA_J + EPS));

						double fmax = max(ss1, ss2);

						f2[i2].pr.gl = max(f1[i1].pr.gl, fmin);
						f2[i2].pr.gr = min(f1[i1].pr.gr, fmax);
					}
					else
					{
						f2[i2].pr.gl = 1.0;
						f2[i2].pr.gr = -1.0;
					}

					if (f2[i2].pr.gl > f2[i2].pr.gr)
					{
						f2[i2].pr.x = f1[i1].r;
						f2[i2].pr.y = row - 1;

						f1[i1].pr.isVertice = true;

						dx = (double)f2[i2].r - (double)f2[i2].pr.x;
						dy = (double)row - (double)f2[i2].pr.y;

						double dm = (dy - ALPHA_J >= 0) ? (dy - ALPHA_J) : 0;

						double ss1 = min((dx - ALPHA_J) / (dm + EPS), (dx - ALPHA_J) / (dy + ALPHA_J + EPS));
						double ss2 = min((dx + ALPHA_J) / (dm + EPS), (dx + ALPHA_J) / (dy + ALPHA_J + EPS));

						double fmin = min(ss1, ss2);

						ss1 = max((dx - ALPHA_J) / (dm + EPS), (dx - ALPHA_J) / (dy + ALPHA_J + EPS));
						ss2 = max((dx + ALPHA_J) / (dm + EPS), (dx + ALPHA_J) / (dy + ALPHA_J + EPS));

						double fmax = max(ss1, ss2);
						f2[i2].pr.gl = fmin;
						f2[i2].pr.gr = fmax;
					}

					--i1;
				}
			}
		}

		for (int i = 0; i <= len1; ++i)
		{
			if (f1[i].color != 61)
			{
				continue;
			}

			if (f1[i].pl.isVertice)
			{
				cout << f1[i].l << " " << row - 1 << endl;

				Point centerCircle(f1[i].l, row - 1);
				int radius = 5;
				Scalar colorCircle(0, 0, 255);
				circle(outImg, centerCircle, radius, colorCircle, FILLED);
			}

			if (f1[i].pr.isVertice)
			{
				cout << f1[i].r << " " << row - 1 << endl;
				Point centerCircle(f1[i].r, row - 1);
				int radius = 5;
				Scalar colorCircle(0, 0, 255);
				circle(outImg, centerCircle, radius, colorCircle, FILLED);
			}
		}

		swap(f1, f2);
		swap(l1, l2);
	}

	log.close();
	imwrite("output.bmp", outImg);

	return 0;
}
