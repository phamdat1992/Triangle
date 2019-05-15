#include <bits/stdc++.h>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

const double ALPHA_J = 1;
const int ALPHA_COLOR = 50;
const double EPSILON = 1e-6;

class ColorNode {
    public:
        Vec3b color;
        Vec2i pLeft, pRight;
        ColorNode *pNext;
        
        int idRow, idxL, idxR;
        float gradMax, gradMin;

        ColorNode() {
            // reset to first column
            this->idxL = 0;
            this->idxR = 0;

            this->gradMax = numeric_limits<float>::max();
            this->gradMin = numeric_limits<float>::min();
            this->pNext = NULL;
            this->color = Vec3b(0,0,0);
            this->idRow = 0;
        }

        void setColor(uchar a, uchar b, uchar c) {
            this->color[0] = a;
            this->color[1] = b;
            this->color[2] = c;
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

void readImage(Mat &image, const string &path) {
    image = imread(path, CV_LOAD_IMAGE_COLOR);

    if (image.empty()) {
        cout<< "Image not found" <<endl;
    }
}

bool isSameColor(Vec3b c1, Vec3b c2) {
    if (c1[0] == c2[0]) {
        return true;
    }
}

void process(Mat &inImg, Mat &outImg) {
    
    uchar* imgData = (uchar*)(inImg.data);
    Vec3b cc;

    ImpressedImg* l1 = new ImpressedImg();
    ImpressedImg* l2 = new ImpressedImg();
    ImpressedImg* tm;

    l1->idRow = -1;
    l2->idRow = 0;

    ColorNode* cl1 = l1->head;
    ColorNode* cl2 = l2->head;

    cl2->setColor(imgData[0], imgData[1], imgData[2]);

    for (int iRow = 0, it = 0; iRow < inImg.rows; ++iRow) {
        for (int jCol = 0; jCol < inImg.cols; ++jCol) {
            cc[0] = imgData[it++];
            cc[1] = imgData[it++];
            cc[2] = imgData[it++];

            if (isSameColor(cc, cl2->color)) {
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

                } else {
                    // add vertices to the result
                }

                // update new color
                if (cl2->pNext == NULL) {
                    cl2 ->pNext = new ColorNode();
                    cl2->idRow = l2->idRow;
                }

                cl2 = cl2->pNext;
                cl2->idxL = cl2->idxR = jCol;
                cl2->color = cc;
            }
        }

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


int main(int argc, char* argv[]) {

    string imageName;
    Mat inImg, outImg;

    readImage(inImg, argv[1]);
    process(inImg, outImg);

    imwrite(argv[2], outImg);

    return 0;
}