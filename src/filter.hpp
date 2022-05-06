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
#include <vector>

/*
 * Implement own greyscale filter function
 * src is CV_8UC3
 * dst is CV_8UC3
 */
void greyscale(cv::Mat &src, cv::Mat &dst);

/*
 * Implement a 5x5 Gaussian filter as separable 1x5 filters ([1 2 4 2 1]
 * vertical and horizontal) using the following function prototype. Implement
 * the function by accessing pixels, not using openCV filter functions. You can
 * assume the input is a color image and the output should also be a color
 * image. src is CV_8UC3 dst is CV_8UC3
 */
void blur5x5(cv::Mat &src, cv::Mat &dst);

void drawOnChessboard(cv::Mat &src, cv::Mat &dst, std::vector<cv::Point2f> &imagePoints, cv::Size chessboardSize);

#endif