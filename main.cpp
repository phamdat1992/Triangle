#include <bits/stdc++.h>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

const double ALPHA_J = 1;
const int ALPHA_COLOR = 50;
const double EPSILON = 1e-6;

class ColorNode {
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

            void setPointLeft(int x, int y) {
                this->point[0] = x;
                this->point[1] = y;
            }
    };

    public:
        Vec3b color;
        ColorNode *pNext;
        Side leftSide, rightSide;
        int idRow;

        ColorNode() {
            // reset to first column
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

class TriangleImage {
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

        void init() {
            l1->idRow = -1;
            l2->idRow = 0;

            ColorNode* cl1 = l1->head;
            ColorNode* cl2 = l2->head;
        }

        inline bool isSameColor(Vec3b c1, Vec3b c2) {
            if (c1[0] == c2[0]) {
                return true;
            }
        }

        void updateNewColor(int iRow, int jCol) {

        }

        bool updatePoint(int iRow) {
            if (this->cl1->pLeft[1] > this->cl2->idxL) {
                this->cl2->gradLeft.gmin = max(this->cl1->gradLeft.gmin, ((this->cl2->idxL - this->delta) - this->cl1->pLeft[1])/((iRow - this->delta) - this->cl1->pLeft[0]));
                this->cl2->gradLeft.gmax = min(this->cl1->gradLeft.gmax, ((this->cl2->idxL + this->delta) - this->cl1->pLeft[1])/((iRow + this->delta) - this->cl1->pLeft[0]));

                if (this->cl2->gradLeft.gmin <= this->cl2->gradLeft.gmax) {

                    return false;
                }
            } else if (this->cl1->pLeft[1] < this->cl2->idxL) {
                this->cl2->gradLeft.gmin = max(this->cl1->gradLeft.gmin, ((this->cl2->idxL - this->delta) - this->cl1->pLeft[1])/((iRow + this->delta) - this->cl1->pLeft[0]));
                this->cl2->gradLeft.gmax = min(this->cl1->gradLeft.gmax, ((this->cl2->idxL + this->delta) - this->cl1->pLeft[1])/((iRow - this->delta) - this->cl1->pLeft[0]));

                if (this->cl2->gradLeft.gmin <= this->cl2->gradLeft.gmax) {
                        
                    return false;
                }
            } else {
                this->cl2->gradLeft.gmin = max(this->cl1->gradLeft.gmin, ((this->cl2->idxL - this->delta) - this->cl1->pLeft[1])/((iRow - this->delta) - this->cl1->pLeft[0]));
                this->cl2->gradLeft.gmax = min(this->cl1->gradLeft.gmax, ((this->cl2->idxL + this->delta) - this->cl1->pLeft[1])/((iRow - this->delta) - this->cl1->pLeft[0]));

                if (this->cl2->gradLeft.gmin <= this->cl2->gradLeft.gmax) {
                        
                    return false;
                }
            }

            this->cl2->setPointLeft(iRow, cl2->idxL);
            this->cl2->gradLeft.resetGradient();

            return true;
        }

        void process(Mat &inImg, Mat &outImg) {
            uchar* imgData = (uchar*)(inImg.data);
            cl2->setColor(imgData[0], imgData[1], imgData[2]);
            this->init();

            for (int iRow = 0, jCol = 0, it = 0; iRow < inImg.rows; ++iRow, jCol = 0) {
                for (; jCol < inImg.cols; ++jCol) {
                    this->setColor(imgData[it++], imgData[it++], imgData[it++]);

                    if (isSameColor(this->color, cl2->color)) {
                        cl2->idxR = jCol;
                    } else { 
                        while (cl1 != NULL && cl1->idRow == l1->idRow && cl1->idxR < cl2->idxL) {
                            cl1 = cl1->pNext;
                        }
                        while (cl1 != NULL && cl1->idRow == l1->idRow && cl1->idxR < cl2->idxR && !isSameColor(cl1->color, cl2->color)) {
                            cl1 = cl1->pNext;
                        }

                        if (cl1 != NULL && cl1->idRow == l1->idRow && isSameColor(cl1->color, cl2->color)) {
                            // stranfer parent points from l1
                            // calculate new gradient
                            // valuate the left point
                            this->updateLeftPoint(iRow);

                            // valuate the right point
                            this->updateRightPoint(iRow);
                        } else {
                            // add vertices to the result
                            cl2->setPointLeft(iRow, cl2->idxL);
                            cl2->setPointRight(iRow, cl2->idxR);
                            cl2->resetGradient();
                        }

                        // update new color
                        if (cl2->pNext == NULL) {
                            cl2 ->pNext = new ColorNode();
                        }

                        cl2 = cl2->pNext;
                        cl2->idRow = l2->idRow;
                        cl2->idxL = cl2->idxR = jCol;
                        cl2->setColor(this->color);
                    }
                }

                // update vertices for the last color node in l2
                this->updateNewColor(
                    iRow,

                );

                // end row
                swap(l1, l2);

                // reset head node
                cl1 = l1->head;
                cl2 = l2->head;
                if (iRow + 1 < inImg.rows) {
                    cl2->idRow = l2->idRow = l1->idRow + 1;
                    cl2->setColor(imgData[it], imgData[it + 1], imgData[it + 2]);
                }
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

    string imageName;
    Mat inImg, outImg;

    readImage(inImg, argv[1]);
    process(inImg, outImg);

    imwrite(argv[2], outImg);

    return 0;
}