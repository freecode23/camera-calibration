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
#include <sys/stat.h>
using namespace std;

/*
 * Implement a 5x5 Gaussian filter as separable 1x5 filters ([1 2 4 2 1]
 * vertical and horizontal) using the following function prototype. Implement
 * the function by accessing pixels, not using openCV filter functions. You can
 * assume the input is a color image and the output should also be a color
 * image. src is CV_8UC3 dst is CV_8UC3
 */
void blur5x5(cv::Mat &src, cv::Mat &dst);

/*
 * Given the path and image name, append a number to the image name so that it
 * doesn't overwrite the old one.
 * @param pathName the pathName to write the image to
 * @param imgName the imgName
 * @return the full path to save the image to
 */
string getNewFileName(string pathName, string imgName);

/*
 * Given an image source, find a chessboard pattern and draw points on the
 * chessboard. Save this corner points as a vector in imagePoints vector
 * @param src the source image
 * @param dst the destination frame to display
 * @param the imagepoints output
 * @chessboardSize the width and height cell of the chessboard
 */
void drawOnChessboard(cv::Mat &src, cv::Mat &dst,
                      std::vector<cv::Point2f> &imagePoints,
                      cv::Size chessboardSize);

void saveImage(cv::Mat frame, string imgPrefix); 

void savePoints(cv::Size chessboardSize,
                vector<cv::Point2f> &imagePoints,
                vector<cv::Point3f> &worldPoints,
                vector<vector<cv::Point2f>> &listImagePoints,
                vector<vector<cv::Point3f>> &listWorldPoints);
#endif