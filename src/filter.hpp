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
#include <sys/stat.h>

#include <opencv2/opencv.hpp>
#include <vector>
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

string saveImage(cv::Mat frame, string imgPrefix);

void savePointsCsvVector(cv::Size chessboardSize,
                         vector<cv::Point2f> &imagePoints,
                         vector<cv::Point3f> &worldPoints,
                         vector<vector<cv::Point2f>> &listImagePoints,
                         vector<vector<cv::Point3f>> &listWorldPoints,
                         char* imgName,
                         std::vector<char *> &imageNames);

/*
  Given a file src_csv with the format of a string as the first column and
  floating point numbers as the remaining columns, this function
  returns the file as a std::vector of Point2f, and vector of Point3f
  @param chessboardSize is the chess board size which will determine the number
  of points we have that is the vector size.
  @param listImagePoints will contain the 2D points of all the images
  @param listWorldPoints contain the 3d world points of all the images

  If echo_file is true, it prints out the contents of the file as read
  into memory.
  The function returns a non-zero value if something goes wrong.
 */
int read2d3DVectorsFromCSV(char *src_csv, cv::Size chessboardSize,
                           vector<vector<cv::Point2f>> &listImagePoints,
                           vector<vector<cv::Point3f>> &listWorldPoints,
                           std::vector<char *> &imageNames, int echo_file);

void appendRotationTranslationVector(cv::Mat rotationVec,
                                     cv::Mat translVec,
                                     char *&imageName, char *csvfilepath,
                                     int reset_file);

#endif