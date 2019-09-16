#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

const int ALPHA_J = 1;

void readImage(Mat& image, const string& path)
{
	image = imread(path, CV_LOAD_IMAGE_COLOR);

	if (image.empty())
	{
		cout << "Image not found" << endl;
	}
}

int main(int argc, char* argv[])
{
	TriangleImage triangleImg;
	string imageName;
	Mat inImg, outImg;

	readImage(inImg, argv[1]);
	triangleImg.process(inImg, outImg);
	imwrite(argv[2], outImg);

	return 0;
}
