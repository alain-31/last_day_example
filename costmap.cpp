#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>
#include <thread>
#include <chrono>


// Function to extract a subregion from a costmap and return as a cv::Mat
cv::Mat cropCostmap(
    const unsigned char* costmap, int map_width, int map_height,
    int min_x, int min_y, int max_x, int max_y) {
    
    // Ensure bounds are within limits
    if (min_x < 0 || min_y < 0 || max_x >= map_width || max_y >= map_height || min_x > max_x || min_y > max_y) {
        std::cerr << "Error: Invalid crop region!\n";
        return cv::Mat(); // Return empty image on failure
    }

    int cropped_width = max_x - min_x + 1;
    int cropped_height = max_y - min_y + 1;

    cv::Mat cropped_image(cropped_height, cropped_width, CV_8UC1);

    // Extract the required region
    for (int row = min_y; row <= max_y; ++row) {
        for (int col = min_x; col <= max_x; ++col) {
            int original_index = row * map_width + col;
            int cropped_row = row - min_y;
            int cropped_col = col - min_x;
            cropped_image.at<unsigned char>(cropped_row, cropped_col) = costmap[original_index];
        }
    }

    return cropped_image;
}

/**
*  g++ costmap.cpp -o costmap `pkg-config --cflags --libs opencv4`
 */

int main() {
    // Dimensions of the costmap
    const int width = 1000;
    const int height = 1000;

    // Create a blank costmap with all pixels initialized to 0 (background)
    unsigned char costmap[width * height] = {0};

    // Define the center and radius of the disk
    const int centerX = width / 2;
    const int centerY = height / 2;
    const int radius = 300; // Disk radius

    // Fill in the disk with value 250
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int dx = x - centerX;
            int dy = y - centerY;
            // Check if the pixel is within the disk
            if (dx * dx + dy * dy <= radius * radius) {
                costmap[y * width + x] = 250;  // Set the pixel value to 250 inside the disk
            }
        }
    }

    // Convert the unsigned char array to a cv::Mat object
    cv::Mat costmapMat(height, width, CV_8UC1, costmap);

    // Display the costmap using OpenCV
    cv::imshow("Costmap", costmapMat);

    // Crop the region
    int x = 100;
    int y = 700;
    int w=300;
    int h=100;
    cv::Rect roi(x, y, w, h);
    cv::Mat cropped = costmapMat(roi);

    cv::imshow("CostmapCropped", cropped);

    int  min_x = x;
    int  min_y = y;
    int  max_x = min_x + w -1;
    int  max_y = min_y + h -1;
    
/*
    for (int k=0;k<10;k++) {


        // Define top-left and bottom-right points of the rectangle
        cv::Point topLeft(min_x+10*k, min_y+10*k);
        cv::Point bottomRight(max_x+10*k, max_y+10*k);

        // Draw the rectangle (image, top-left, bottom-right, color, thickness)
        cv::rectangle(costmapMat, topLeft, bottomRight, cv::Scalar(100, 100, 100), 2);
        
        // Flip
        cv::Mat flipped_costmapMat;
        cv::flip(costmapMat, flipped_costmapMat, 0);

        // Display the costmap using OpenCV
        cv::imshow("Costmap", flipped_costmapMat);

        // crop region directly from cost map
        cv::Mat cropped2 = cropCostmap(costmap, width, height, min_x+10*k, min_y+10*k, max_x+10*k, max_y+10*k);

        cv::imshow("CostmapCropped2", cropped2);    
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
*/

   // crop region directly from cost map
   cv::Mat cropped2 = cropCostmap(costmap, width, height, min_x, min_y, max_x, max_y);

   // Create an elliptical kernel (size 7x7)
   cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(51, 51));

   // gaussian kernel
   cv::Mat gaussianKernel = cv::getGaussianKernel(51, 10, CV_32F);
   cv::Mat gaussianKernel2D = gaussianKernel * gaussianKernel.t(); // Create 2D Gaussian kernel

    // Create a mask where the pixel value is 255 (white)
    cv::Mat binaryMask = (cropped2 == 250);

    // Dilate the binary mask using the elliptical kernel
    cv::Mat dilatedMask;
    //cv::dilate(binaryMask, dilatedMask, kernel);
    cv::dilate(binaryMask, dilatedMask, gaussianKernel2D);


    // Set pixels in the mask to 100
    cropped2.setTo(100, dilatedMask);

    // Display the costmap using OpenCV
    cv::imshow("Costmap", costmapMat);

    // Iterate over the modified image with two nested loops
    for (int y = 0; y < cropped2.rows; y++) {
        for (int x = 0; x < cropped2.cols; x++) {
            uchar pixelValue = cropped2.at<uchar>(y, x);
            // Convert 2D coordinates to 1D index
            int index = (min_y+y) * width + min_x+x;
            costmap[index] = pixelValue;
        }
    }

    

    cv::Mat costmapMat2(height, width, CV_8UC1, costmap);
    cv::normalize(costmapMat2, costmapMat2, 0, 255, cv::NORM_MINMAX);
    cv::imshow("Costmap updated", costmapMat2);

    // Flip
    cv::Mat flipped_costmapMat;
    cv::flip(costmapMat2, flipped_costmapMat, 0);
 
    // Display the costmap using OpenCV
    cv::imshow("flipped Costmap", flipped_costmapMat);

    cv::waitKey(0);  // Wait for a key press to close the window

    return 0;
}