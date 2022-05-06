//**********************************************************************************************************************
// FILE: filter.hpp
//
// DESCRIPTION
// Contains functions for applying filter to image
//
// AUTHOR
// Sherly Hartono
//**********************************************************************************************************************
#ifndef FILTER_H
#define FILTER_H

#include <opencv2/opencv.hpp>

/*
* Implement own greyscale filter function 
* src is CV_8UC3
* dst is CV_8UC3
*/
void greyscale(cv::Mat &src, cv::Mat &dst);


/*
* Implement a 5x5 Gaussian filter as separable 1x5 filters ([1 2 4 2 1] vertical and horizontal)
* using the following function prototype. Implement the function by accessing pixels,
* not using openCV filter functions.
* You can assume the input is a color image and the output should also be a color image.
* src is CV_8UC3
* dst is CV_8UC3
*/
void blur5x5(cv::Mat &src, cv::Mat &dst);

/**
 * @brief A 3x3 Sobel X and 3x3 Sobel Y filter - as separable 1x3 filters
 * Each should implement a 3x3 Sobel filter, either horizontal (X) or vertical (Y).
 * The X filter should be positive right and the Y filter should be positive up.
 * Both the input and output images should be color images,
 * but the output needs to be of type 16S (signed short)
 * because the values can be in the range [-255, 255]
 * 
 * @param src is CV_8UC3
 * @param dst is CV_16SC3
 */
void sobelX3x3( cv::Mat &src, cv::Mat &dst );
void sobelY3x3( cv::Mat &src, cv::Mat &dst );


/**
 * @brief Generates a gradient magnitude image from the X and Y Sobel images
 * In filter.cpp, implement a function that generates a gradient magnitude image using Euclidean distance
 * for magnitude: I = sqrt( sx*sx + sy*sy ). This should still be a color image.
 * The two input images will be 3-channel signed short images,
 * but the output should be a uchar color image suitable for display.
 * @param sx is CV_16SC3
 * @param sy is CV_16SC3
 * @param dst is CV_8UC3
 * @return int 
 */
void magnitude( cv::Mat &sx, cv::Mat &sy, cv::Mat &dst);

/**
 * @brief Takes in a color image, blurs the image, and then quantizes
 * the image into a fixed number of levels as specified by a parameter. For example,
 * if the level parameter is 10, then each color channel should be quantized into 10 values.
 * You can do this by:
 * 1. Figuring out the size of a bucket using b = 255/levels.
 * 2. Take a color channel value x, then xt = x / b,
 * 3. xf = xt * b.
 * After executing this for each pixel and each color channel,
 * The image will have only levels**3 possible color values.
 * @param src is CV_8UC3
 * @param dst is CV_8UC3
 */
void blurQuantize( cv::Mat &src, cv::Mat &dst, int levels );


/**
 * @brief Cartoonizes a color image.
 * Do this by first:
 * 1. Calculate gradient magnitude.
 * 2. blurQuantize the srcImage.
 * 3. Set to black any pixels with a gradient magnitude larger than a threshold.
 * @param src is CV_8UC3
 * @param dst is CV_8UC3
 */
void cartoon( cv::Mat &src, cv::Mat&dst, int levels, int magThreshold );



/**
 * @brief Draw lines in black and white
 * @param src is CV_8UC3
 * @param dst is CV_8UC3
 */
void lineDraw(cv::Mat &src, cv::Mat &dst);

/**
 * @brief Write text and draw three circles
 * @param src is CV_8UC3
 * @param dst is CV_8UC3
 */
void writeText(cv::Mat &src, cv::Mat &dst);
#endif