#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>

#define MAX_DEPTH 8
#define DETAIL_THRESHOLD 13
#define SIZE_MULT 1

using namespace std;
using namespace cv;

// Calculating average color of a region
Vec3b average_color(const Mat& image){
    // mean() function calculates the average of all the pixels in the image -> OpenCV
    Scalar avg_color = mean(image); // Scalar - BGR channels
    // Vec3b - 3-channel vector of 8-bit unsigned integers
    return Vec3b((uchar)average_color[0], (uchar)average_color[1], (uchar)average_color[2]); // 8-bit unsigned integer - BGR channels
}

// Function to calculate the weighted average of a histogram -> calculate the detail
double weighted_average(int* hist, int size){
    int total = 0;
    double value = 0;
    double error = 0;
    
    // Standard deviation
    for (int i = 0; i < size; i++){
        total += hist[i];
        value += i * hist[i];
    }

    if (total > 0){
        value /= total;
        for (int i = 0; i < size; i++){
            error += hist[i] * (i - value) * (i - value);
        }
        error = sqrt(error / total);
    }

    return error; // Difference between the average and the actual values -> calculate the detail
}

class Quadrant{
public:
    Rect bbox;
    int depth;
    bool is_leaf;
    vector<Quadrant> children;
    Vec3b color;
    double detail;


    Quadrant(const Mat& image, Rect bbox, int depth)
    : bbox(bbox), depth(depth), is_leaf(false), children(4, nullptr)
    {
        Mat region = image(bbox);
        detail = get_detail(region);
        color = average_color(region);
    }

}