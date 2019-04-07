#include "opencv2/highgui/highgui.hpp"
#include <bits/stdc++.h>

using namespace std;
using namespace cv;

int main(int argc, const char**argv) {
    
      vector<int> a;
      a.push_back(5);

      vector<int> *b;
      b = &a;
      cout<< (*b)[0] <<endl;
      return 0;
}