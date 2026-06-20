#include <iostream>
#include <opencv4/opencv2/opencv.hpp>
using namespace std;
using namespace cv;
int main(int argc, char **argv)
{

    if (argc != 2) {
        fprintf(stderr, "usage: \r\n   cvshow  image_filename\n");
        return 1;
    }

    Mat srcImage = imread(argv[1]);
    imshow("源图像", srcImage);
    waitKey(0);
    return 0;
}
