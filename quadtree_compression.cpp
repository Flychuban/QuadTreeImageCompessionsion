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

// Function to calculate the detail of a region based on the histogram of the image
double get_detail(const Mat& image){
    int histSize = 256;
    float range[] = {0, 256};
    const float* histRange = {range};
    bool uniform = true; // all bins in the histogram will have the same size
    bool accumulate = false; // compute the histogram from scratch ignoring previously computed histogram

    Mat r_hist, g_hist, b_hist; // Histograms for the 3 channels

    // Calculate the histogram for each channel
    vector<Mat> bgr_planes; // Planes for the 3 channels

    split(image, bgr_planes); // Split the image into the 3 channels

    // Calculate the histogram for each channel
    calcHist(&bgr_planes[0], 1, 0, Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate);
    calcHist(&bgr_planes[1], 1, 0, Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate);
    calcHist(&bgr_planes[2], 1, 0, Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate);

    // Calculate the detail for each channel
    double red_detail = weighted_average((int *)r_hist.data, histSize);
    double green_detail = weighted_average((int *)g_hist.data, histSize);
    double blue_detail = weighted_average((int *)b_hist.data, histSize);

    // Combine the details of the 3 channels
    return red_detail * 0.2989 + green_detail * 0.5870 + blue_detail * 0.1140; // Luminance of the image -> grayscale to rgb
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

    void split_region(const Mat&){
        int middle_x = bbox.x + bbox.width / 2;
        int middle_y = bbox.y + bbox.height / 2;

        children[0] = Quadrant(image, Rect(bbox.x, bbox.y, bbox.width / 2, bbox.height / 2), depth + 1); // Top-left
        children[1] = Quadrant(image, Rect(middle_x, bbox.y, bbox.width / 2, bbox.height / 2), depth + 1); // Top-right
        children[2] = Quadrant(image, Rect(bbox.x, middle_y, bbox.width / 2, bbox.height / 2), depth + 1); // Bottom-left
        children[3] = Quadrant(image, Rect(middle_x, middle_y, bbox.width / 2, bbox.height / 2), depth + 1); // Bottom-right
    }
};

class QuadTree{
public:
    Quadrant* root;
    int max_depth;

    QuadTree(const Mat& image){
        max_depth = 0;
        root = new Quadrant(image, Rect(0, 0, image.cols, image.rows), 0);
    }

    // Recursively build the quadtree by splitting the regions
    void build(Quadrant* quad, const Mat& iamge){
        if (quad->depth >= MAX_DEPTH || quad->detail < DETAIL_THRESHOLD){ // Stop splitting the region
            if (quad->depth > max_depth){
                max_depth = quad->depth;
            }
            quad->is_leaf = true;
            return; 
        }
        else{
            quad->split_region(image);
            for (int i = 0; i < 4; i++){
                build(&quad->children[i], image);
            }
        }
    }
};