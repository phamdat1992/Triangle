#include <bits/stdc++.h>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

const double ALPHA_J = 1;
const int ALPHA_COLOR = 50;
const double EPSILON = 1e-6;

class TriangleImage {
    class Gradient {
        public:
            float gmax, gmin;

            void resetGradient() {
                this->gmax = numeric_limits<float>::max();
                this->gmin = numeric_limits<float>::min();
            }
    };
    class Side {
        public:
            Vec2i point;
            int idx;
            Gradient grad;

            Side() {
                this->idx = 0;
                this->grad.resetGradient();
            }

            void setPoint(int x, int y) {
                this->point[0] = x;
                this->point[1] = y;
            }
    };
    class ColorNode {
    public:
        Vec3b color;
        ColorNode *pNext;
        Side *leftSide, *rightSide;
        int idRow;

        ColorNode() {
            // reset to first column
            this->leftSide = new Side();
            this->rightSide = new Side();
            this->pNext = NULL;
            this->color = Vec3b(0,0,0);
            this->idRow = 0;
        }

        void setColor(uchar a, uchar b, uchar c) {
            this->color[0] = a;
            this->color[1] = b;
            this->color[2] = c;
        }

        void setColor(Vec3b &cc) {
            this->color[0] = cc[0];
            this->color[1] = cc[1];
            this->color[2] = cc[2];
        }
};

class ImpressedImg {
    public:
        ColorNode* head;
        int idRow;

        ImpressedImg() {
            this->head = new ColorNode();
            this->idRow = 0;
        }

        ~ImpressedImg() {
            ColorNode* cur = head;
            while (this->head != NULL) {
                cur = head->pNext;
                delete head;
                head = cur;
            }
        }
};

    protected:
        ImpressedImg *l1, *l2; 
        ColorNode *cl1, *cl2; 
        Vec3b color; 
        float delta;
    public:
        inline void setColor(uchar a, uchar b, uchar c) {
            this->color[0] = a;
            this->color[1] = b;
            this->color[2] = c;
        }

        TriangleImage() {
            this->delta = 0.5f;
            this->l1 = new ImpressedImg();
            this->l2 = new ImpressedImg();
        }

        ~TriangleImage() {
            this->l1->~ImpressedImg();
            this->l2->~ImpressedImg();
        }

        inline bool isSameColor(Vec3b c1, Vec3b c2) {
            if (c1[0] == c2[0]) {
                return true;
            }
        }

        void updateNewColor(int iRow, int jCol) {

        }

        bool updatePoint(int iRow, Side *l1Side, Side *l2Side) {
            if (l1Side->point[1] > l2Side->idx) {
                l2Side->grad.gmin = max(l1Side->grad.gmin, ((l2Side->idx - this->delta) - l1Side->point[1])/((iRow - this->delta) - l1Side->point[0]));
                l2Side->grad.gmax = min(l1Side->grad.gmax, ((l2Side->idx + this->delta) - l1Side->point[1])/((iRow + this->delta) - l1Side->point[0]));
            } else if (l1Side->point[1] < l2Side->idx) {
                l2Side->grad.gmin = max(l1Side->grad.gmin, ((l2Side->idx - this->delta) - l1Side->point[1])/((iRow + this->delta) - l1Side->point[0]));
                l2Side->grad.gmax = min(l1Side->grad.gmax, ((l2Side->idx + this->delta) - l1Side->point[1])/((iRow - this->delta) - l1Side->point[0]));
            } else {
                l2Side->grad.gmin = max(l1Side->grad.gmin, ((l2Side->idx - this->delta) - l1Side->point[1])/((iRow - this->delta) - l1Side->point[0]));
                l2Side->grad.gmax = min(l1Side->grad.gmax, ((l2Side->idx + this->delta) - l1Side->point[1])/((iRow - this->delta) - l1Side->point[0]));
            }

            if (l2Side->grad.gmin <= l2Side->grad.gmax) {

                return true;
            }

            l2Side->setPoint(iRow, l2Side->idx);
            l2Side->grad.resetGradient();

            return false;
        }

        void process(Mat &inImg, Mat &outImg) {
            uchar* imgData = (uchar*)(inImg.data);
            this->cl2->setColor(imgData[0], imgData[1], imgData[2]);
            this->l1->idRow = -1;

            for (int iRow = 0, jCol = 0, it = 0; iRow < inImg.rows; ++iRow, jCol = 0, swap(l1, l2)) {
                // reset head node
                this->cl1 = this->l1->head;
                this->cl2 = this->l2->head;

                this->cl2->idRow = this->l2->idRow = iRow;
                this->cl2->setColor(imgData[it], imgData[it + 1], imgData[it + 2]);
                
                for (; jCol < inImg.cols; ++jCol, it += 3) {
                    this->setColor(imgData[it], imgData[it + 1], imgData[it + 2]);

                    if (isSameColor(this->color, this->cl2->color)) {
                        this->cl2->rightSide->idx = jCol;
                    } else { 
                        while (this->cl1 != NULL && this->cl1->idRow == this->l1->idRow && this->cl1->rightSide->idx < this->cl2->leftSide->idx) {
                            this->cl1 = this->cl1->pNext;
                        }
                        while (this->cl1 != NULL && this->cl1->idRow == this->l1->idRow && this->cl1->rightSide->idx < this->cl2->leftSide->idx && !isSameColor(this->cl1->color, this->cl2->color)) {
                            this->cl1 = this->cl1->pNext;
                        }

                        if (this->cl1 != NULL && this->cl1->idRow == this->l1->idRow && isSameColor(this->cl1->color, this->cl2->color)) {
                            // stranfer parent points from l1
                            // calculate new gradient
                            // valuate the left point
                            this->updatePoint(iRow, this->cl1->leftSide, this->cl2->leftSide);

                            // valuate the right point
                            this->updatePoint(iRow, this->cl1->rightSide, this->cl2->rightSide);
                        } else {
                            // add vertices to the result
                            this->cl2->leftSide->setPoint(iRow, this->cl2->leftSide->idx);
                            this->cl2->rightSide->setPoint(iRow, this->cl2->rightSide->idx);
                            this->cl2->leftSide->resetGradient();
                            this->cl2->rightSide->resetGradient();
                        }

                        // update new color
                        if (this->cl2->pNext == NULL) {
                            this->cl2->pNext = new ColorNode();
                        }

                        this->cl2 = this->cl2->pNext;
                        this->cl2->idRow = this->l2->idRow;
                        this->cl2->leftSide->idx = this->cl2->rightSide->idx = jCol;
                        this->cl2->setColor(this->color);
                    }
                }

                // update vertices for the last color node in l2
                this->updateNewColor(
                    iRow,

                );
            }
        }
};

void readImage(Mat &image, const string &path) {
    image = imread(path, CV_LOAD_IMAGE_COLOR);

    if (image.empty()) {
        cout<< "Image not found" <<endl;
    }
}

int main(int argc, char* argv[]) {
    TriangleImage triangleImg;
    string imageName;
    Mat inImg, outImg;

    readImage(inImg, argv[1]);
    triangleImg.process(inImg, outImg);
    imwrite(argv[2], outImg);

    return 0;
}