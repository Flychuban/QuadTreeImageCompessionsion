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
    Scalar average_color = mean(image); // Scalar - BGR channels
    // Vec3b - 3-channel vector of 8-bit unsigned integers
    return Vec3b((uchar)average_color[0], (uchar)average_color[1], (uchar)average_color[2]); // 8-bit unsigned integer - BGR channels
}

// Function to calculate the weighted average of a histogram -> calculate the detail
double weighted_average(const Mat &hist)
{
    int total = 0;
    double value = 0;
    double error = 0;

    for (int i = 0; i < hist.rows; i++)
    {
        total += hist.at<int>(i); // using at() function to access the histogram values from Mat object -> OpenCV
        value += i * hist.at<int>(i);
    }

    if (total > 0)
    {
        value /= total;
        for (int i = 0; i < hist.rows; i++)
        {
            error += hist.at<int>(i) * (i - value) * (i - value);
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

    // Check if the image was correctly split
    if (bgr_planes.size() != 3)
    {
        cerr << "Error: Image was not correctly split into 3 channels" << endl;
        return -1;
    }

    // Calculate the histogram for each channel
    calcHist(&bgr_planes[0], 1, 0, Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate);
    calcHist(&bgr_planes[1], 1, 0, Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate);
    calcHist(&bgr_planes[2], 1, 0, Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate);

    // Check if histograms were correctly calculated
    if (b_hist.empty() || g_hist.empty() || r_hist.empty())
    {
        cerr << "Error: Histogram calculation failed" << endl;
        return -1;
    }

    // Calculate the detail for each channel
    double red_detail = weighted_average(r_hist);
    double green_detail = weighted_average(g_hist);
    double blue_detail = weighted_average(b_hist);

    // Combine the details of the 3 channels
    return red_detail * 0.2989 + green_detail * 0.5870 + blue_detail * 0.1140; // Luminance of the image -> grayscale to rgb
}

class Quadrant{
public:
    Rect bbox;
    int depth;
    bool is_leaf;
    vector<Quadrant*> children;
    Vec3b color;
    double detail;

    Quadrant(const Mat &image, Rect bbox, int depth)
        : bbox(bbox), depth(depth), is_leaf(false), children(4, nullptr)
    {
        // Log the bounding box dimensions and position
        cout << "Creating Quadrant at depth " << depth << " with bounding box: " << bbox << endl;

        // Ensure the bounding box is within the image boundaries
        if (bbox.x >= 0 && bbox.y >= 0 &&
            bbox.x + bbox.width <= image.cols &&
            bbox.y + bbox.height <= image.rows)
        {
            Mat region = image(bbox);
            this->detail = get_detail(region);
            this->color = average_color(region);
        }
        else
        {
            cerr << "Error: Bounding box is out of image boundaries" << endl;
            this->detail = 0;
            this->color = Vec3b(0, 0, 0);
        }
    }

    void split_region(const Mat& image){
        cout << "BBOX width: " << bbox.width << " BBOX height: " << bbox.height << endl;
        if (bbox.width <= 0 || bbox.height <= 0)
        {
            cerr << "Error: Zero or negative dimensions in bounding box" << endl;
            return;
        }

        int middle_x = bbox.x + bbox.width / 2;
        int middle_y = bbox.y + bbox.height / 2;

        cout << "Splitting Quadrant at depth " << depth << " into four children." << endl;

        this->children[0] = new Quadrant(image, Rect(bbox.x, bbox.y, bbox.width / 2, bbox.height / 2), this->depth + 1); // Top-left
        this->children[1] = new Quadrant(image, Rect(middle_x, bbox.y, bbox.width / 2, bbox.height / 2), this->depth + 1); // Top-right
        this->children[2] = new Quadrant(image, Rect(bbox.x, middle_y, bbox.width / 2, bbox.height / 2), this->depth + 1); // Bottom-left
        this->children[3] = new Quadrant(image, Rect(middle_x, middle_y, bbox.width / 2, bbox.height / 2), this->depth + 1); // Bottom-right
    }

    ~Quadrant(){
        for(auto& child : this->children){
            delete child;
        }
    }
};

class QuadTree{
public:
    Quadrant* root;
    int max_depth;

    // Initialize with zero depth and the root quadrant
    QuadTree(const Mat& image){
        this->max_depth = 0;
        this->root = new Quadrant(image, Rect(0, 0, image.cols, image.rows), 0);
    }

    // Recursively build the quadtree by splitting the regions
    void build(Quadrant* quad, const Mat& image){
        if (quad->depth >= MAX_DEPTH || quad->detail < DETAIL_THRESHOLD){ // Stop splitting the region
            if (quad->depth > this->max_depth){
                this->max_depth = quad->depth;
            }
            quad->is_leaf = true;
            return; 
        }
        else{
            quad->split_region(image);
            for(auto& child : quad->children){
                build(child, image);
            }
        }
    }

    void draw_quadrants(Mat& image, Quadrant* quad, int depth, bool show_lines){
        if (quad->depth == depth || quad->is_leaf){
            // rectangle() function draws a rectangle on the image -> OpenCV
            rectangle(image, quad->bbox, Scalar(quad->color[0], quad->color[1], quad->color[2]), FILLED); // Fill the rectangle with the average color of the region
            if (show_lines){
                rectangle(image, quad->bbox, Scalar(0, 0, 0), 1); // Draw the lines of the rectangle
            }            
        }
        else{
            for(auto& child : quad->children){
                if (child)
                {
                    draw_quadrants(image, child, depth, show_lines);
                }
            }
        }
    }

    Mat create_image(int depth, bool show_lines){
        // CV_8UC3 - 8-bit unsigned integer matrix with 3 channels
        Mat image = Mat::zeros(this->root->bbox.height, root->bbox.width, CV_8UC3); // Create a black imagec
        draw_quadrants(image, this->root, depth, show_lines);
        return image;
    }

    ~QuadTree(){
        for(auto& child : this->root->children){
            delete child;
        }
    }
};

int main(){
    cout << "QuadTree Image Compression" << endl;
    // Load image
    const char *image_path = "/Users/flychuban/Documents/img_compression_test1.jpeg";
    Mat image = imread(image_path, IMREAD_COLOR);
    if (image.empty())
    {
        cout << "Could not open or find the image." << endl;
        return -1;
    }

    // Resize image if needed
    resize(image, image, Size(image.cols * SIZE_MULT, image.rows * SIZE_MULT));

    // Create quadtree
    QuadTree quadtree(image);
    quadtree.build(quadtree.root, image);

    // Iterate through the quadtree and print the depth of each quadrant
    cout << "Max depth: " << quadtree.max_depth << endl;

    // Create image with custom depth
    int depth = 7;
    Mat result_image = quadtree.create_image(depth, false);
    imwrite("/Users/flychuban/Documents/img_compression_test1_result.jpeg", result_image);

    return 0;
}