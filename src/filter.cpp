//**********************************************************************************************************************
// FILE: filter.cpp
//
// DESCRIPTION
// Contains implementation for applying filter to image
//
// AUTHOR
// Sherly Hartono
//**********************************************************************************************************************

#include "filter.hpp"

#include <iostream>
using namespace std;

void greyscale(cv::Mat &src, cv::Mat &dst) {
    // allocate destination immge use size and type of the source image.
    dst.create(src.size(), src.type());

    // for each ith row
    for (int i = 0; i < src.rows; i++) {
        int greenValue;

        // at each jth column (pixel)
        for (int j = 0; j < src.cols; j++) {
            // takes channel green's value at this pixel
            greenValue = src.at<cv::Vec3b>(i, j)[1];

            // assign green values to channel red, green and blue
            dst.at<cv::Vec3b>(i, j)[0] = greenValue;
            dst.at<cv::Vec3b>(i, j)[1] = greenValue;
            dst.at<cv::Vec3b>(i, j)[2] = greenValue;
        }
    }
}

void blur5x5(cv::Mat &src, cv::Mat &dst) {
    // 1. Create intermediate frame for storing h filter result
    cv::Mat inter;
    src.copyTo(inter);

    // 2. H filter:
    // Loop pixels and apply horizontal filter
    // loop over all rows
    uint8_t *srcPtr = (uint8_t *)src.data;
    uint8_t *interPtr = (uint8_t *)inter.data;
    for (int i = 0; i < src.rows; i++) {
        // loop over columns -2 (j =2)
        for (int j = 2; j < src.cols - 2; j++) {
            cv::Vec3i res16bit = {0, 0, 0};

            // loop over color channel
            for (int ch = 0; ch < 3; ch++) {
                // apply filter
                res16bit[ch] =
                    srcPtr[(i * src.cols * 3) + ((j - 2) * 3) + ch] * 1 +
                    srcPtr[(i * src.cols * 3) + ((j - 1) * 3) + ch] * 2 +
                    srcPtr[(i * src.cols * 3) + ((j)*3) + ch] * 4 +
                    srcPtr[(i * src.cols * 3) + ((j + 1) * 3) + ch] * 2 +
                    srcPtr[(i * src.cols * 3) + ((j + 2) * 3) + ch] * 1;

                res16bit /= 10;  // normalise
                // convert to 8 bit and assign to intermediate result
                interPtr[i * inter.cols * 3 + j * 3 + ch] =
                    (unsigned char)res16bit[ch];
            }
        }
    }
    inter.copyTo(dst);
    uint8_t *dstPtr = (uint8_t *)dst.data;

    // 4. V filter:
    // Loop pixels and apply vertical filter to the resulting horizonal filter
    // loop over all rows -2
    for (int i = 2; i < inter.rows - 2; i++) {
        // loop over all columns
        for (int j = 0; j < inter.cols; j++) {
            cv::Vec3i res16bit = {0, 0, 0};
            cv::Vec3b res8bit;  // result at this i,j pixel

            // loop over color channel
            for (int ch = 0; ch < 3; ch++) {
                // apply filter
                res16bit[ch] =
                    interPtr[((i - 2) * inter.cols * 3) + (j * 3) + ch] * 1 +
                    interPtr[((i - 1) * inter.cols * 3) + (j * 3) + ch] * 2 +
                    interPtr[((i)*inter.cols * 3) + (j * 3) + ch] * 4 +
                    interPtr[((i + 1) * inter.cols * 3) + (j * 3) + ch] * 2 +
                    interPtr[((i + 2) * inter.cols * 3) + (j * 3) + ch] * 1;

                res16bit /= 10;                             // normalise
                res8bit[ch] = (unsigned char)res16bit[ch];  // convert to 8 bit
                dstPtr[i * dst.cols * 3 + j * 3 + ch] = res8bit[ch];  // assign
            }
            // out of for loop. we finish calculating the pixel per color
            // channel
        }
    }
}

void drawOnChessboard(cv::Mat &src, cv::Mat &dst, vector<cv::Point2f> & outputImagePoints, cv::Size chessboardSize) {

    // 1. make grey frame
    cv::Mat srcGray;
    cv::cvtColor(src, srcGray, cv::COLOR_BGR2GRAY);

    // 2. find chessboardimagePoints
    bool found = findChessboardCorners(src, chessboardSize, outputImagePoints,
                        cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_FILTER_QUADS);

    // cout << "corners: " <<imagePoints << "size: " << chessboardSize;

    // 3. parameters for corner subpix
    cv::Size wnSize = cv::Size(5, 5);
    cv::Size zeroZone = cv::Size(-1, -1);
    cv::TermCriteria criteria = cv::TermCriteria(
        cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 40, 0.001);
    ;

    // 4. if it finds something use cornersSubpix on the grey image to get more accurate location
    if (found) {
        cout << "found" << endl; 
        cv::Size winSize = cv::Size(5, 5);
        cv::Size zeroZone = cv::Size(-1, -1);
        cv::cornerSubPix(srcGray, outputImagePoints, winSize, zeroZone,
                         criteria);
        cv::drawChessboardCorners(src, chessboardSize, outputImagePoints, found);                         
    } else {
        cout << "not found" << endl;
    }

    // 5. display
    src.copyTo(dst);
}