#include <opencv2/opencv.hpp>

int main()
{
    // Load an image
    // cv::Mat image = cv::imread("/Users/flychuban/Desktop/TUES_10_klas/School_10grade/ProizvosdtvenaPraktika/QuadTreeImageCompression/teenovator_final.jpg");
    cv::Mat image = cv::imread("/Users/flychuban/Desktop/Kaloyan_Anastasov/images/front_face.png");
    // Check if image loading is successful
    if (!image.data)
    {
        std::cerr << "Error loading image!" << std::endl;
        return -1;
    }

    // Display the image (optional)
    cv::imshow("Loaded Image", image);
    cv::waitKey(0); // Wait for a key press

    // Release resources
    cv::destroyAllWindows();

    return 0;
}