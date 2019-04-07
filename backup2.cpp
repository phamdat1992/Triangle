#include <bits/stdc++.h>
#include <opencv2/opencv.hpp>


using namespace std;
using namespace cv;

const double pi = acos(-1.0);
const double APLHA_GRADIENT = pi/10;
const double ALPHA_J = 1;
const int ALPHA_COLOR = 50;
const int LSIDE = 1;
const int RSIDE = 10;

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

        double minGradientL, maxGradientL;
        double minGradientR, maxGradientR;

        

    public:
        LinkedListNode *next;
        Point upLeft, upRight;

        LinkedListNode() {
            this->l = -1;
            this->r = -1;
            this->gradientL = -100.0;
            this->gradientR = -100.0;
            this->next = NULL;
            this->upLeft = this->upRight = Point(-1, -1);
            this->color = Vec3b(0,0,0);
            this->ind = -1;
            this->minGradientL = this->minGradientR = 1000.0;
            this->maxGradientL = this->maxGradientR = -1000.0;
        }
        LinkedListNode(int _l, int _r, Vec3b _color, LinkedListNode *link) {
            this->l = _l;
            this->r = _r;
            this->color = _color;
            this->next = link;
            this->upLeft = this->upRight = Point(-1, -1);
            this->gradientL = -100.0;
            this->gradientR = -100.0;
            this->ind = -1;
            this->minGradientL = this->minGradientR = 1000.0;
            this->maxGradientL = this->maxGradientR = -1000.0;
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

        void setMaxGradientRight(double maxGradientR) {
            this->maxGradientR = maxGradientR;
        }

        double getMaxGradientRight() {
            return this->maxGradientR;
        }

        void setMinGradientRight(double minGradientR) {
            this->minGradientR = minGradientR;
        }

        double getMinGradientRight() {
            return this->minGradientR;
        }

        void setMaxGradientLeft(double maxGradientL) {
            this->maxGradientL = maxGradientL;
        }

        double getMaxGradientLeft() {
            return this->maxGradientL;
        }

        void setMinGradientLeft(double minGradientL) {
            this->minGradientL = minGradientL;
        }

        double getMinGradientLeft() {
            return this->minGradientL;
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
Mat image, imgwrite;
int m, n;
int cnt = 0;
Pixels a[1000][1000];
vector<Point> vPointL[100000], vPointR[100000];

Vec3b fix(Vec3b color) {
    int mx = max(color.val[0], max(color.val[1], color.val[2]));

    if (color.val[0] == mx) {
        color.val[1] = color.val[2] = 0;
    }
    else if (color.val[1] == mx) {
        color.val[0] = color.val[2] = 0;
    }
    else {
        color.val[0] = color.val[1] = 0;
    }

    return color;
}

void init() {
    image = imread(imageName, CV_LOAD_IMAGE_COLOR);
    //imwrite("out2.png", image);

    //cout<< atan2(3, -2) <<' '<< atan2(4, -2) <<endl;
    if (image.empty()) {
        cout<< "Image not found" <<endl;
        cin.get();
        return;
    }

    m = image.rows;
    n = image.cols;


    imgwrite = Mat::zeros(m, n, CV_8UC3);
    //Mat image_fix = Mat::zeros(m, n, CV_8UC3);
    //Mat mt(m, n, CV_8UC3, Scalar(255, 255, 255));
    //imgwrite = mt;
    
    for(int i = 0; i < m; ++i)
        for(int j = 0; j < n; ++j) {
            //Vec3b color = fix(image.at<Vec3b>(i, j));
            Vec3b color = image.at<Vec3b>(i, j);
            if (color.val[0] != 255 || color.val[1] != 255 || color.val[2] != 255)
                color = Vec3b(0, 0, 0);
            a[i][j] = Pixels(i, j, color);

            //image_fix.at<Vec3b>(i, j) = fix(color);
            //cout<< i <<' '<< j <<' '<< image.at<Vec3b>(i,j) <<endl;
        }
    //imwrite("out2.png", mt);
}

LinkedListNode* insertLinkedListNode(LinkedListNode *head, int l, int r, Vec3b color) {
    LinkedListNode *node = new LinkedListNode(l,r,color, head);
    return node;
}

double gradient(Point2d A, Point2d Origin) {
    double x = A.x - Origin.x, y = A.y - Origin.y;
    //cout<< x <<' '<< y <<' '<< atan2(x, y) <<endl;
    return atan2((double) x, (double) y);
}

double computeJ(int i, double gradient) {
    return (double) i / tan(gradient);
}

bool is_same_color(Vec3b c1, Vec3b c2) {
    //c1 = fix(c1);
    //c2 = fix(c2);
    
    for(int i = 0; i < 3; ++i)
    if (abs(c1.val[i] - c2.val[i]) > ALPHA_COLOR) return false;
    return true;
}

void colored(Point p, Vec3b color) {
    imgwrite.at<Vec3b>(p.x, p.y) = color;
}

void eraseColor(Point p) {
    //cout<< p.x <<' '<< p.y <<' '<< "BUG" <<endl;
    imgwrite.at<Vec3b>(p.x, p.y) = Vec3b(0, 0, 0);
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
                while(l1->getLeft() > l2->getRight() && l1->next != NULL) {
                    l1 = l1->next;
                    /*if (l2->getLeft() == 408 && l2->getRight() == 408){
                        cout<< "CHECK :" <<endl;
                        cout<< l1->getLeft() <<' '<< l1->getRight() <<endl;
                        cout<< l2->getLeft() <<' '<< l2->getRight() <<endl;
                        cout<< l1->getColor() <<' '<< l2->getColor() <<endl;
                    }*/
                }
                // Lấy vị trí các đoạn màu tại List 1 và List 2
                int l1l = l1->getLeft(), l1r = l1->getRight();
                int l2l = l2->getLeft(), l2r = l2->getRight();

                /*
                    Kiểm tra xem 2 đoạn màu có giao nhau không
                */
                if (l1r + l2r - l1l - l2l + 1 >= max(l1r,l2r) - min(l1l,l2l) &&
                    is_same_color(l1->getColor(), l2->getColor())) {
                    /*
                        Kiểm tra góc sẵn có với góc mới, nếu không chênh lệch nhiều
                        thì ta coi như 3 điểm nằm trên 1 cạnh của đa giác
                    */

                    
                    //line(imgwrite, Point(i,l2l), Point(i,l2r), Scalar(0,0,100), 1, 8);
                    //line(imgwrite, Point(i,l2l), Point(i,l2r), Scalar(0,0,0), 1, 8);

                    if (l1->upRight != Point(-1, -1)) {

                        int l0r = l1->upRight.y;
                        int absI = i - l1->upRight.x;
                        Point2d p = Point2d(i - absI, l0r);

                        double minGradientRight = max(  gradient(Point2d(i-0.5, l2r-ALPHA_J), p),
                                                        gradient(Point2d(i+0.5, l2r-ALPHA_J), p));
                        double maxGradientRight = min(  gradient(Point2d(i-0.5, l2r+ALPHA_J), p),
                                                        gradient(Point2d(i+0.5, l2r+ALPHA_J), p));
                        
                        double newMinGradientRight = min(minGradientRight, l1->getMinGradientRight());
                        double newMaxGradientRight = max(maxGradientRight, l1->getMaxGradientRight());

                        double jmin = l0r + computeJ(absI, newMinGradientRight);
                        double jmax = l0r + computeJ(absI, newMaxGradientRight);
                        //if (LSIDE <= i && i <= RSIDE) 
                            //cout<< jmin <<' '<< jmax <<' '<< l2r <<' '<< "R" <<endl;

                        if (jmin <= jmax) {
                            l2->setMinGradientRight(newMinGradientRight);
                            l2->setMaxGradientRight(newMaxGradientRight);

                            eraseColor(Point(i-1, l1r));
                            l2->upRight = l1->upRight;

                        }
                        else {
                            Point2d p = Point2d(i-1, l1r);
                            minGradientRight = max( gradient(Point2d(i-0.5, l2r-ALPHA_J), p),
                                                    gradient(Point2d(i+0.5, l2r-ALPHA_J), p));
                            maxGradientRight = min( gradient(Point2d(i-0.5, l2r+ALPHA_J), p),
                                                    gradient(Point2d(i+0.5, l2r+ALPHA_J), p));
                            l2->setMinGradientRight(minGradientRight);
                            l2->setMaxGradientRight(maxGradientRight);


                            l2->upRight = Point(i-1, l1r);
                        }
                    }
                    else {
                        Point2d p = Point2d(i-1, l1r);
                        double minGradientRight = max(      gradient(Point2d(i-0.5, l2r-ALPHA_J), p),
                                                            gradient(Point2d(i+0.5, l2r-ALPHA_J), p));
                        double maxGradientRight = min(      gradient(Point2d(i-0.5, l2r+ALPHA_J), p),
                                                            gradient(Point2d(i+0.5, l2r+ALPHA_J), p));
                        l2->setMinGradientRight(minGradientRight);
                        l2->setMaxGradientRight(maxGradientRight);

                        l2->upRight = Point(i-1, l1r);
                    }

                    if(l2l == l2r) break;

                    if (l1->upLeft != Point(-1, -1)) {
                        int l0l = l1->upLeft.y;
                        int absI = i - l1->upLeft.x;
                        Point2d p = Point2d(i - absI, l0l);

                        double minGradientLeft = max(   gradient(Point2d(i-0.5, l2l-ALPHA_J), p),
                                                        gradient(Point2d(i+0.5, l2l-ALPHA_J), p));
                        double maxGradientLeft = min(   gradient(Point2d(i-0.5, l2l+ALPHA_J), p),
                                                        gradient(Point2d(i+0.5, l2l+ALPHA_J), p));

                        double newMinGradientLeft = min(minGradientLeft, l1->getMinGradientLeft());
                        double newMaxGradientLeft = max(maxGradientLeft, l1->getMaxGradientLeft());

                        double jmin = l0l + computeJ(absI, newMinGradientLeft);
                        double jmax = l0l + computeJ(absI, newMaxGradientLeft);

                        if (jmin <= jmax ) {
                            l2->setMinGradientLeft(newMinGradientLeft);
                            l2->setMaxGradientLeft(newMaxGradientLeft);

                            eraseColor(Point(i-1, l1l));
                            l2->upLeft = l1->upLeft;
                        }
                        else {
                            p = Point2d(i-1, l1l);
                            minGradientLeft = max(  gradient(Point2d(i-0.5, l2l-ALPHA_J), p),
                                                    gradient(Point2d(i+0.5, l2l-ALPHA_J), p));
                            maxGradientLeft = min(  gradient(Point2d(i-0.5, l2l+ALPHA_J), p),
                                                    gradient(Point2d(i+0.5, l2l+ALPHA_J), p));
                            l2->setMinGradientLeft(minGradientLeft);
                            l2->setMaxGradientLeft(maxGradientLeft);

                            l2->upLeft = Point(i-1, l1l);
                        }

                    }
                    else {
                        Point2d p = Point2d(i-1, l1l);
                        double minGradientLeft = max(   gradient(Point2d(i-0.5, l2l-ALPHA_J), p),
                                                        gradient(Point2d(i+0.5, l2l-ALPHA_J), p));
                        double maxGradientLeft = min(   gradient(Point2d(i-0.5, l2l+ALPHA_J), p),
                                                        gradient(Point2d(i+0.5, l2l+ALPHA_J), p));
                        l2->setMinGradientLeft(minGradientLeft);
                        l2->setMaxGradientLeft(maxGradientLeft);

                        l2->upLeft = Point(i-1, l1l);
                    }

                    break;
                }
                else if (l1->getRight() < l2->getLeft()) {
                    break;
                }
                else if (l1->getLeft() <= l2->getLeft() && l2->getRight() <= l1->getRight()) {
                    break;
                }
                l1 = l1->next;
            }

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
        //int ind = cur->getInd();
        int l = cur->getLeft(), r = cur->getRight();
        Vec3b color = cur->getColor();
        colored(Point(i, l), color);
        colored(Point(i, r), color);

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


    cout<< cnt <<endl;
    cout<< m <<' '<< n <<endl;
    cout<< a[79][72].getColor() <<' '<< a[80][72].getColor() <<endl;
    cout<< is_same_color(a[79][72].getColor(), a[80][72].getColor()) <<endl;
    imwrite("out.jpg", imgwrite);
    imwrite("out1.png", imgwrite);
}   


int main(int argc, char* argv[]) {

    imageName = argv[1];
    freopen("out.txt", "w", stdout);

    init();
    process();

    return 0;
}