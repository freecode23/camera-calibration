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


/**
 * @brief This is a util function that loads up a list of 2D and 3D world points for camera calibration
 * in the beginning of the program if there is any.
 * 
 * @param src_csv the file that contains 2D and 3D points
 * @param chessboardSize the size of the chessboard
 * @param listImagePoints the list of 2D points
 * @param listWorldPoints the list of 3D points
 * @param imageNames the names of the image
 * @param echo_file setting if we want to set the input to read only
 * @return int 0 if not successful
 */
int read2d3DVectorsFromCSV(char *src_csv, cv::Size chessboardSize,
                           vector<vector<cv::Point2f>> &listImagePoints,
                           vector<vector<cv::Point3f>> &listWorldPoints,
                           std::vector<char *> &imageNames, int echo_file);


/*
 * Task 1: Given an image source, find a chessboard pattern and draw points on the
 * chessboard. Save this corner points as a vector in imagePoints vector
 * @param src the source image
 * @param dst the destination frame to display
 * @param the imagepoints output
 * @chessboardSize the width and height cell of the chessboard
 */
void drawOnChessboard(cv::Mat &src, cv::Mat &dst,
                      std::vector<cv::Point2f> &imagePoints,
                      cv::Size chessboardSize);
/**
 * @brief Task 2: Will save an image as png to the res folder 
 * 
 * @param frame the image frame to be saved as png
 * @param imgPrefix the name of the image
 * @return string 
 */
string saveImage(cv::Mat frame, string imgPrefix);



/**
 * @brief Task 2: For the purpose of calibration, this function 
 * will save the image 2D points of the chessboard and its projection in world 3D points
 * to a csv file and to its respective "list" of vectors. 
 * We store the image names so that we don't need to re-calibrate each time we start the program
 * We can just load the world and image points when calibrating.
 * 
 * @param chessboardSize the row col of the chessboard
 * @param imagePoints the 2D points of each corner of the chessboard
 * @param worldPoints the 3D world points of each corner of the chessboard
 * @param listImagePoints the list to push our new 2D points to
 * @param listWorldPoints the list to push our new 3D points to
 * @param imgName the name of the image to be saved to gether in the csv file
 * @param imageNames the list of names of the image to store our images.
 */
void savePointsCsvVector(cv::Size chessboardSize,
                         vector<cv::Point2f> &imagePoints,
                         vector<cv::Point3f> &worldPoints,
                         vector<vector<cv::Point2f>> &listImagePoints,
                         vector<vector<cv::Point3f>> &listWorldPoints,
                         char *imgName, std::vector<char *> &imageNames);

/**
 * @brief Task 3. Given a list of world and image points this function 
 * will save the intrinsic matrices: distortion cofficient and camera matrix to a csv file.
 * It will also save the extrinsic matrices for every images used as calibration. 
 * It will use openCV's calibrateCamera method to perform the calibration
 * 
 * @param srcFrame the image to calibrate the camera
 * @param listWorldPoints the list of 3D points of the chessboard
 * @param listImagePoints 
 * @param imageNames 
 */
void calibrating(cv::Mat srcFrame, vector<vector<cv::Point3f>> &listWorldPoints,
                 vector<vector<cv::Point2f>> &listImagePoints,
                 std::vector<char *> &imageNames);


/**
 * @brief Task 4. Given position of chessboard in 2D and 3D, 
 * this function will print rotation and translation vectors.
 * If the given calibration matrix and distortion is empty, it will load it from a csv file
 * as this is also need in using openCV solvePnP method.
 * 
 * @param chessboardSize the size of the chessboard
 * @param worldPoints the 3D points of the board
 * @param imagePoints the 2D points of projection of the board to image
 * @param calibMatrix the calibration matrix
 * @param distortCoeff the distortion coefficient
 * @return true if imagePoints size is not 0
 * @return false if imagePoints size is 0
 */
bool getCameraPosition(cv::Size chessboardSize,
                       vector<cv::Point3f> &worldPoints,
                       vector<cv::Point2f> &imagePoints,
                       cv::Mat &calibMatrix,
                       cv::Mat &distortCoeff,
                       cv::Mat &rotVec,
                       cv::Mat &transVec);


void draw3DAxesOnChessboard(cv::Mat &srcFrame,
                            cv::Mat &calibMatrix, cv::Mat &distortCoeff,
                            cv::Mat &rotVec, cv::Mat &transVec);



void drawVirttualObjectOnChessboard(cv::Mat &srcFrame, cv::Mat &calibMatrix,
                            cv::Mat &distortCoeff, cv::Mat &rotVec,
                            cv::Mat &transVec);
#endif