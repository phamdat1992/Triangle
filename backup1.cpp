#include <bits/stdc++.h>
#include <opencv2/highgui.hpp>
#include <opencv2/core/core.hpp>

using namespace std;
using namespace cv;

const double pi = acos(-1.0);
const double APLHA_GRADIENT = pi/10;
const int ALPHA_COLOR = 50;

class LinkedListNode {
    private:
        /*
            Mỗi nút lưu lại màu, kiểm soát cột l -> r, 
            có 2 hệ số góc tương ứng Left, Right và
            số thứ tự đa giác chứa nút này
        */
        Vec3b color;
        int l, r;
        double gradientL, gradientR;
        int ind;

    public:
        LinkedListNode *next;
        LinkedListNode() {
            this->l = -1;
            this->r = -1;
            this->gradientL = -100.0;
            this->gradientR = -100.0;
            this->next = NULL;
            this->color = Vec3b(0,0,0);
            this->ind = -1;
        }
        LinkedListNode(int _l, int _r, Vec3b _color, LinkedListNode *link) {
            this->l = _l;
            this->r = _r;
            this->color = _color;
            this->next = link;
            this->gradientL = -100.0;
            this->gradientR = -100.0;
            this->ind = -1;
        }

        void printLinkedList() {
            for(LinkedListNode *p = this; p != NULL; p = p->next)
                cout<< p->getLeft() <<' '<< p->getRight() <<' '<< p->getInd() <<endl;
            cout<< "END" <<endl;
        }   

        /*
            Getter && Setter
        */
        void setLeft(int _l) {
            this->l = _l;
        }

        int getLeft() {
            return this->l;
        }

        void setRight(int _r) {
            this->r = _r;
        }
        
        int getRight() {
            return this->r;
        }

        void setGradientL(double gradientL) {
            this->gradientL = gradientL;
        }

        double getGradientL() {
            return this->gradientL;
        }

        void setGradientR(double gradientR) {
            this->gradientR = gradientR;
        }

        double getGradientR() {
            return this->gradientR;
        }

        void setInd(int ind) {
            this->ind = ind;
        }

        int getInd() {
            return this->ind;
        }

        void setColor(Vec3b color) {
            this->color = color;
        } 

        Vec3b getColor() {
            return this->color;
        }


} *head = NULL, *current = NULL;

class Pixels {
    private:
        int x, y;
        Vec3b color;
    public: 
        Pixels() {
            this->x = 0;
            this->y = 0;
            this->color = Vec3b(0,0,0);
        }
        Pixels(int _posx, int _posy, Vec3b _color) {
            this->x = _posx;
            this->y = _posy;
            this->color = _color;
        }

        Vec3b getColor() {
            return this->color;
        }

        void setColor(Vec3b color) {
            this->color = color;
        }
};


string imageName;
Mat image;
int m, n;
int cnt = 0;
Pixels a[1000][1000];
vector<Point> vPointL[100000], vPointR[100000];


void init() {
    image = imread(imageName, 1);
    imwrite("out2.png", image);

    m = image.rows;
    n = image.cols;
    
    for(int i = 0; i < m; ++i)
        for(int j = 0; j < n; ++j) {
            a[i][j] = Pixels(i, j, image.at<Vec3b>(i,j));
        }
}

LinkedListNode* insertLinkedListNode(LinkedListNode *head, int l, int r, Vec3b color) {
    LinkedListNode *node = new LinkedListNode(l,r,color, head);
    return node;
}

double gradient(Point A, Point Origin) {
    int x = A.x - Origin.x, y = A.y - Origin.y;
    return atan2((double) x, (double) y);
}

bool is_same_color(Vec3b c1, Vec3b c2) {
    for(int i = 0; i < 3; ++i)
    if (abs(c1.val[i] - c2.val[i]) > ALPHA_COLOR) return false;
    return true;
}

void updateIndPolygon(int i, LinkedListNode *l1Head, LinkedListNode *l2Head) {

    /*
        Do cài đặt bằng LinkedList nên các đoạn màu được duyệt từ 
        phải qua trái   
    */

    LinkedListNode *l1 = l1Head, *l2 = l2Head;
    while(l2 != NULL) {
            while(l1 != NULL) {
                /*
                    Nếu 2 đoạn màu không trùng nhau thì
                    set đoạn màu ở List 2 vào đa giác mới
                */
                if (l1->getRight() < l2->getLeft()) {
                    if (l2->getInd() == -1) l2->setInd(cnt++);
                    break;
                }

                /* 
                    Nếu đoạn màu ở List 1 nằm bên phải đoạn màu ở List 2 
                    và 2 đoạn này không giao nhau thì xét đoạn tiếp theo trong List 1
                */
                while(l1->getLeft() > l2->getRight() && l1->next != NULL) l1 = l1->next;
                
                // Lấy vị trí các đoạn màu tại List 1 và List 2
                int l1l = l1->getLeft(), l1r = l1->getRight();
                int l2l = l2->getLeft(), l2r = l2->getRight();

                /*
                    Kiểm tra xem 2 đoạn màu có giao nhau không
                */
                if (l1r + l2r - l1l - l2l + 1 >= max(l1r,l2r) - min(l1l,l2l) &&
                    is_same_color(l1->getColor(), l2->getColor())) {
                    int ind = l1->getInd();
                    l2->setInd(ind);

                    /*
                        Kiểm tra góc sẵn có với góc mới, nếu không chênh lệch nhiều
                        thì ta coi như 3 điểm nằm trên 1 cạnh của đa giác
                    */


                    double graR = gradient(Point(i, l2r), Point(i-1, l1r));
                    if (abs(graR - l1->getGradientR()) > APLHA_GRADIENT) {

                        l2->setGradientR(graR);
                    }
                    else {
                        l2->setGradientR(l1->getGradientR());
                        if (!vPointR[ind].empty()) vPointR[ind].erase(vPointR[ind].end());
                    }

                    if(l2l == l2r) break;

                    double graL = gradient(Point(i, l2l), Point(i-1, l1l));
                    if (abs(graL - l1->getGradientL()) > APLHA_GRADIENT) {
                        l2->setGradientL(graL);
                    }
                    else {

                        l2->setGradientL(l1->getGradientL());
                        if (!vPointL[ind].empty()) vPointL[ind].erase(vPointL[ind].end());
                    }
                    break;
                }
                else if (l1->getRight() < l2->getLeft()) {
                    // Nếu 2 đoạn không giao nhau thì đoạn màu ở List 2 thuộc đa giác mới
                    l2->setInd(cnt++);
                    break;
                }
                l1 = l1->next;
            }
        // Nếu List 1 rỗng thì set các đoạn màu ở List 2 vào các đa giác mới
        if (l1 == NULL && l2->getInd() == -1) l2->setInd(cnt++);
        l2 = l2->next;
    }   
}

void addColorOfPixelinRow(int i, LinkedListNode *head) {
    /*
        Lưu lại màu vào 2 vector Left Right tương ứng với
        các điểm ở 2 bên của đa giác
    */
    LinkedListNode *cur = head;
    while(cur != NULL) {
        int ind = cur->getInd();
        int l = cur->getLeft(), r = cur->getRight();
        if (l == r) {
            vPointR[ind].push_back(Point(i, r));
        }
        else {
            vPointL[ind].push_back(Point(i, l));
            vPointR[ind].push_back(Point(i, r));
        }

        cur = cur->next;
    }
}

void process() {
    LinkedListNode *l1Head = NULL, *l2Head = NULL;
    

    for(int i = 0; i < m; ++i) {
        l2Head = insertLinkedListNode(l2Head, 0, 0, a[i][0].getColor());
        /*
            Tìm các đoạn màu của hàng i rồi lưu vào List 2
            Nếu i = 0 thì phải khởi tạo vị trí đa giác mà chứa đoạn màu
        */
        if (i == 0) l2Head->setInd(cnt++);
        for(int j = 1; j < n; ++j) {
            if (!is_same_color(a[i][j].getColor(),  a[i][j-1].getColor())) {
                l2Head = insertLinkedListNode(l2Head,j,j, a[i][j].getColor());
                if (i == 0) {
                    l2Head->setInd(cnt++);
                }
            }
            else {
                l2Head->setRight(j);
            }
        }

        if (i > 0) {
            // Cập nhật xem các đoạn màu thuộc đa giác nào
            updateIndPolygon(i, l1Head, l2Head);
        }

        //Đẩy các điểm trên đa giác vào vector 
        addColorOfPixelinRow(i, l2Head);

        /*
            Giải phóng bộ nhớ List 1 và gán List 2 vào List 1
        */
        if (l1Head != NULL)
            while(l1Head->next != NULL) {
                LinkedListNode *l1next = l1Head->next;
                delete l1Head;
                l1Head = l1next;
            }
        l1Head = l2Head;
        l2Head = NULL;

    }

    /*
        ghi màu các đa giác tại các đỉnh ra Matrix rồi 
        xuất ra file ảnh jpg (ou1.jpg) và png (out2.png)
    */
    Mat imgwrite = Mat::zeros(m, n, CV_8UC3);

    for(int i = 0; i < cnt; ++i) {

        for(int j = 0; j < vPointR[i].size(); ++j) {
            int x = vPointR[i][j].x, y = vPointR[i][j].y;
            imgwrite.at<Vec3b>(x, y) = a[x][y].getColor();

        }
        for(int j = vPointL[i].size() - 1; j >= 0; --j) {
            int x = vPointL[i][j].x, y = vPointL[i][j].y;
            imgwrite.at<Vec3b>(x, y) = a[x][y].getColor();
        }

    }

    cout<< cnt <<endl;
    imwrite("out.jpg", imgwrite);
    imwrite("out1.png", imgwrite);
}   


int main(int argc, char* argv[]) {

    imageName = argv[1];

    init();
    process();

    return 0;
}