#include <bits/stdc++.h>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

const int ALPHA_J = 1;

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
            bool isConnected;

            Side() {
                this->isConnected = false;
                this->idx = 0;
                this->grad.resetGradient();
            }

            void setPoint(Vec2i p) {
                this->point[0] = p[0];
                this->point[1] = p[1];
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
            // valuate the left point
            this->valuatePoints(iRow, this->cl1->leftSide, this->cl2->leftSide);            

            // valuate the right point
            this->valuatePoints(iRow, this->cl1->rightSide, this->cl2->rightSide);

            // update new color
            if (this->cl2->pNext == NULL) {
                this->cl2->pNext = new ColorNode();
            }

            this->cl2 = this->cl2->pNext;
            this->cl2->idRow = this->l2->idRow;
            this->cl2->leftSide->idx = this->cl2->rightSide->idx = jCol;
            this->cl2->setColor(this->color);
        }

        void calcGradient(int iRow, Side *s1, Side* s2) {
            if (s1->point[1] > s2->idx) {
                s2->grad.gmin = max(s1->grad.gmin, ((s2->idx - this->delta) - s1->point[1])/((iRow - this->delta) - s1->point[0]));
                s2->grad.gmax = min(s1->grad.gmax, ((s2->idx + this->delta) - s1->point[1])/((iRow + this->delta) - s1->point[0]));
            } else if (s1->point[1] < s2->idx) {
                s2->grad.gmin = max(s1->grad.gmin, ((s2->idx - this->delta) - s1->point[1])/((iRow + this->delta) - s1->point[0]));
                s2->grad.gmax = min(s1->grad.gmax, ((s2->idx + this->delta) - s1->point[1])/((iRow - this->delta) - s1->point[0]));
            } else {
                s2->grad.gmin = max(s1->grad.gmin, ((s2->idx - this->delta) - s1->point[1])/((iRow - this->delta) - s1->point[0]));
                s2->grad.gmax = min(s1->grad.gmax, ((s2->idx + this->delta) - s1->point[1])/((iRow - this->delta) - s1->point[0]));
            }
        }

        bool updatePoint(int iRow, Side *l1Side, Side *l2Side) {
            this->calcGradient(iRow, l1Side, l2Side);
            if (l2Side->grad.gmin <= l2Side->grad.gmax) {
                l1Side->isConnected = true;
                l2Side->setPoint(l1Side->point);
                return true;
            }

            l2Side->setPoint(iRow, l2Side->idx);
            l2Side->grad.resetGradient();

            return false;
        }

        void valuatePoints(int iRow, Side *l1Side, Side *l2Side) {
            while (this->cl1 != NULL && this->cl1->idRow == this->l1->idRow && l1Side->idx - ALPHA_J <= l2Side->idx && !isSameColor(this->cl1->color, this->cl2->color)) {
                this->cl1 = this->cl1->pNext;
            }

            if (this->cl1 != NULL && this->cl1->idRow == this->l1->idRow && isSameColor(this->cl1->color, this->cl2->color)) {
                this->updatePoint(iRow, l1Side, l2Side);
            } else {
                l2Side->setPoint(iRow, l2Side->idx);
                l2Side->grad.resetGradient();
            }
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
                        this->updateNewColor(iRow, jCol);        
                    }
                }

                // update vertices for the last color node in l2
                this->updateNewColor(iRow, jCol);
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
