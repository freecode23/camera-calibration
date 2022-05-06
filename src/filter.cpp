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

void greyscale(cv::Mat &src, cv::Mat &dst)
{
    // allocate destination immge use size and type of the source image.
    dst.create(src.size(), src.type());

    // for each ith row
    for (int i = 0; i < src.rows; i++)
    {
        int greenValue;

        // at each jth column (pixel)
        for (int j = 0; j < src.cols; j++)
        {
            // takes channel green's value at this pixel
            greenValue = src.at<cv::Vec3b>(i, j)[1];

            // assign green values to channel red, green and blue
            dst.at<cv::Vec3b>(i, j)[0] = greenValue;
            dst.at<cv::Vec3b>(i, j)[1] = greenValue;
            dst.at<cv::Vec3b>(i, j)[2] = greenValue;
        }
    }
}

void blur5x5(cv::Mat &src, cv::Mat &dst)
{
    // 1. Create intermediate frame for storing h filter result
    cv::Mat inter;
    src.copyTo(inter);

    // 2. H filter:
    // Loop pixels and apply horizontal filter
    // loop over all rows
    uint8_t *srcPtr = (uint8_t *)src.data;
    uint8_t *interPtr = (uint8_t *)inter.data;
    for (int i = 0; i < src.rows; i++)
    {
        // loop over columns -2 (j =2)
        for (int j = 2; j < src.cols - 2; j++)
        {
            cv::Vec3i res16bit = {0, 0, 0};

            // loop over color channel
            for (int ch = 0; ch < 3; ch++)
            {
                // apply filter
                res16bit[ch] = srcPtr[(i * src.cols * 3) + ((j - 2) * 3) + ch] * 1 + srcPtr[(i * src.cols * 3) + ((j - 1) * 3) + ch] * 2 + srcPtr[(i * src.cols * 3) + ((j)*3) + ch] * 4 + srcPtr[(i * src.cols * 3) + ((j + 1) * 3) + ch] * 2 + srcPtr[(i * src.cols * 3) + ((j + 2) * 3) + ch] * 1;

                res16bit /= 10; // normalise
                // convert to 8 bit and assign to intermediate result
                interPtr[i * inter.cols * 3 + j * 3 + ch] = (unsigned char)res16bit[ch];
            }
        }
    }
    inter.copyTo(dst);
    uint8_t *dstPtr = (uint8_t *)dst.data;

    // 4. V filter:
    // Loop pixels and apply vertical filter to the resulting horizonal filter
    // loop over all rows -2
    for (int i = 2; i < inter.rows - 2; i++)
    {
        // loop over all columns
        for (int j = 0; j < inter.cols; j++)
        {
            cv::Vec3i res16bit = {0, 0, 0};
            cv::Vec3b res8bit; // result at this i,j pixel

            // loop over color channel
            for (int ch = 0; ch < 3; ch++)
            {
                // apply filter
                res16bit[ch] = interPtr[((i - 2) * inter.cols * 3) + (j * 3) + ch] * 1 + interPtr[((i - 1) * inter.cols * 3) + (j * 3) + ch] * 2 + interPtr[((i)*inter.cols * 3) + (j * 3) + ch] * 4 + interPtr[((i + 1) * inter.cols * 3) + (j * 3) + ch] * 2 + interPtr[((i + 2) * inter.cols * 3) + (j * 3) + ch] * 1;

                res16bit /= 10;                                      // normalise
                res8bit[ch] = (unsigned char)res16bit[ch];           // convert to 8 bit
                dstPtr[i * dst.cols * 3 + j * 3 + ch] = res8bit[ch]; // assign
            }
            // out of for loop. we finish calculating the pixel per color channel
        }
    }
}

void writeText(cv::Mat &src, cv::Mat &dst)
{
    src.copyTo(dst);
    cv::ellipse(dst,
                cv::Point(500, 500),
                cv::Size(140, 140),
                45, //angle
                0,
                360,
                cv::Scalar(255, 0, 0), // blue
                2,
                8);

    cv::ellipse(dst,
                cv::Point(700, 500),
                cv::Size(140, 140),
                45, //angle
                0,
                360,
                cv::Scalar(0, 255, 0), // green
                2,
                8);

    cv::ellipse(dst,
                cv::Point(900, 500),
                cv::Size(140, 140),
                45, //angle
                0,
                360,
                cv::Scalar(0, 0, 255), // red
                2,
                8);

    // Put text
    cv::putText(dst,
                "Hello there!",
                cv::Point(400, 500),            // Coordinates (Bottom-left corner of the text string in the image)
                cv::FONT_HERSHEY_COMPLEX_SMALL, // Font
                4.0,                            // Scale. 2.0 = 2x bigger
                cv::Scalar(255, 255, 255),      // BGR Color
                2,                              // Line Thickness 
                cv::LINE_4);                    
}