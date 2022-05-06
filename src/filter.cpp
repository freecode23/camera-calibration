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

// X = positive right
void sobelX3x3(cv::Mat &src, cv::Mat &dst)
{
    // 1. Create intermediate frame for storing h filter result
    cv::Mat inter;
    inter.create(src.size(), CV_16SC3); // make sure its in signed 16 bits

    // 2. X derivative:
    uint8_t *srcPtr = (uint8_t *)src.data;
    int16_t *interPtr = (int16_t *)inter.data;

    // loop over all rows
    for (int i = 1; i < src.rows - 1; i++)
    {
        // loop over columns -1 (j = 1)
        for (int j = 1; j < src.cols - 1; j++)
        {
            cv::Vec3s res16bit = {0, 0, 0}; // sign short type
            // loop over color channel
            for (int ch = 0; ch < 3; ch++)
            {
                // apply filter
                res16bit[ch] = srcPtr[(i * src.cols * 3) + ((j - 1) * 3) + ch] * -1 + srcPtr[(i * src.cols * 3) + ((j)*3) + ch] * 0 + srcPtr[(i * src.cols * 3) + ((j + 1) * 3) + ch] * 1;

                res16bit /= 1; // normalise (do nothing)
                interPtr[(i * inter.cols * 3) + (j * 3) + ch] = res16bit[ch];
            }
        }
    }

    inter.copyTo(dst);
    int16_t *dstPtr = (int16_t *)dst.data;
    // 3. Gaussian filter:
    // Loop pixels and apply vertical filter to the resulting derivative
    // loop over rows - 1
    for (int i = 1; i < inter.rows - 1; i++)
    {
        // loop over all columns
        for (int j = 1; j < inter.cols - 1; j++)
        {
            cv::Vec3s res16bit = {0, 0, 0}; // signed short type

            // loop over color channel
            for (int ch = 0; ch < 3; ch++)
            {
                // apply filter
                res16bit[ch] = interPtr[((i - 1) * inter.cols * 3) + (j * 3) + ch] * 1 + interPtr[((i)*inter.cols * 3) + (j * 3) + ch] * 2 + interPtr[((i + 1) * inter.cols * 3) + (j * 3) + ch] * 1;

                res16bit /= 4;                                            // normalise
                dstPtr[(i * dst.cols * 3) + (j * 3) + ch] = res16bit[ch]; // assign to dst
            }
        }
    }
}

// Y = positive up
void sobelY3x3(cv::Mat &src, cv::Mat &dst)
{
    // 1. Create intermediate frame for storing h filter result
    cv::Mat inter;
    // Initialize to signed 16 bits  shorts
    // Don't need to initialize dst since we will just copy interDst to dst
    inter.create(src.size(), CV_16SC3);

    // 2. Gaussian filter:
    uint8_t *srcPtr = (uint8_t *)src.data;
    int16_t *interPtr = (int16_t *)inter.data;

    // loop over all rows
    for (int i = 1; i < src.rows - 1; i++)
    {
        // loop over columns -1 (j = 1)
        for (int j = 1; j < src.cols - 1; j++)
        {
            cv::Vec3s res16bit = {0, 0, 0}; // sign short type
            // loop over color channel
            for (int ch = 0; ch < 3; ch++)
            {
                // apply filter
                res16bit[ch] = srcPtr[(i * src.cols * 3) + ((j - 1) * 3) + ch] * 1 + srcPtr[(i * src.cols * 3) + ((j)*3) + ch] * 2 + srcPtr[(i * src.cols * 3) + ((j + 1) * 3) + ch] * 1;

                res16bit /= 4;                                            // normalise (do nothing)
                interPtr[i * inter.cols * 3 + j * 3 + ch] = res16bit[ch]; // assign reseult to intermediate result
            }
        }
    }

    inter.copyTo(dst);
    int16_t *dstPtr = (int16_t *)dst.data;
    // 3. Y derivative:
    // Loop pixels and apply vertical filter to the resulting derivative
    // loop over rows - 1
    for (int i = 1; i < inter.rows - 1; i++)
    {
        // loop over all columns
        for (int j = 1; j < inter.cols - 1; j++)
        {
            cv::Vec3s res16bit = {0, 0, 0}; // signed short type

            // loop over color channel
            for (int ch = 0; ch < 3; ch++)
            {
                // apply filter
                res16bit[ch] = interPtr[((i - 1) * inter.cols * 3) + ((j)*3) + ch] * 1 + interPtr[((i)*inter.cols * 3) + ((j)*3) + ch] * 0 + interPtr[((i + 1) * inter.cols * 3) + ((j)*3) + ch] * -1;

                res16bit /= 1; // normalise
                dstPtr[i * inter.cols * 3 + j * 3 + ch] = res16bit[ch];
            }
        }
    }
}

void magnitude(cv::Mat &sx, cv::Mat &sy, cv::Mat &dst)
{
    dst.create(sx.size(), CV_8UC3);

    // create pointer to the first bit of sx and sy
    // signed 16 bits (short)
    int16_t *sxPtr = (int16_t *)sx.data;
    int16_t *syPtr = (int16_t *)sy.data;

    uint8_t *dstPtr = (uint8_t *)dst.data;
    for (int i = 0; i < sx.rows; i++)
    {
        for (int j = 0; j < sx.cols; j++)
        {
            cv::Vec3s interMag; // short

            // loop over channel
            for (int ch = 0; ch < 3; ch++)
            {
                short sx2 = sxPtr[i * sx.cols * 3 + j * 3 + ch] *
                            sxPtr[i * sx.cols * 3 + j * 3 + ch];
                short sy2 = sxPtr[i * sx.cols * 3 + j * 3 + ch] *
                            syPtr[i * sx.cols * 3 + j * 3 + ch];
                interMag[ch] = (signed short)sqrt(sx2 + sy2);

                dstPtr[i * dst.cols * 3 + j * 3 + ch] = (unsigned char)interMag[ch];
            }
        }
    }
}

void blurQuantize(cv::Mat &src, cv::Mat &dst, int levels)
{
    cv::Mat inter;
    int bucket = 255 / levels;

    // set dst data type explicitly since we are not copying inter to dst later
    dst.create(src.size(), CV_8UC3);

    // do blur5X5
    blur5x5(src, inter);

    // now inter is blur. quantize it and save to dst
    // for loop row
    uint8_t *interPtr = (uint8_t *)inter.data;
    uint8_t *dstPtr = (uint8_t *)dst.data;
    for (int i = 0; i < inter.rows; i++)
    {
        // loop over columns -2 (j =2)
        for (int j = 2; j < inter.cols - 2; j++)
        {
            cv::Vec3b xt; // intermediate result
            cv::Vec3b xf; // final result
            // loop over color channel
            for (int ch = 0; ch < 3; ch++)
            {
                // apply filter
                // 3 is the number of channel
                xt[ch] = interPtr[(i * inter.cols * 3) + (j * 3) + ch] / bucket; // divide
                xf[ch] = xt[ch] * bucket;                                        // multiply
                dstPtr[i * inter.cols * 3 + j * 3 + ch] = xf[ch];
            }
        }
    }
}

void cartoon(cv::Mat &src, cv::Mat &dst, int levels, int magThreshold)
{
    // 1. Create gradient magnitude frame
    // 1.1. Declare magnitude image (don't need to initialize, it will be initialized inside magnitude() function)
    cv::Mat mag;

    // 1.2 Create sx, sy image
    cv::Mat sx;
    cv::Mat sy;

    // 1.3 create sobels and their magnitude
    sobelX3x3(src, sx);
    sobelY3x3(src, sy);
    magnitude(sx, sy, mag);

    // 2. blur quantize the source image
    cv::Mat blurQ;
    blurQuantize(src, blurQ, levels);

    // 3. Modify blur and quantized image pixels to black for values larger than threshold
    // create pointers for blurQ and magnitude image
    uint8_t *blurQPtr = (uint8_t *)blurQ.data;
    uint8_t *magPtr = (uint8_t *)mag.data;

    // loop over all rows
    for (int i = 0; i < blurQ.rows; i++)
    {
        // loop over all columns
        for (int j = 0; j < blurQ.cols; j++)
        {
            // loop over color channel
            for (int ch = 0; ch < 3; ch++)
            {
                // if pixel at magImage > threshold
                if ((magPtr[(i * mag.cols * 3) + (j * 3) + ch]) > magThreshold)
                {
                    // set blurQuantised image to black at 1 channel
                    blurQPtr[i * blurQ.cols * 3 + j * 3 + ch] = 0;
                }
            }
        }
    }
    blurQ.copyTo(dst);
}

void lineDraw(cv::Mat &src, cv::Mat &dst)
{
    src.copyTo(dst);
    //1. Line Draw
    cv::Mat imgBlur;
    cv::Mat imgCanny;
    cv::Mat imgDil;
    cv::Mat imgErode;
    cv::Mat invert;

    blur5x5(src, imgBlur);
    cv::Canny(imgBlur, imgCanny, 25, 75);
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::dilate(imgCanny, imgDil, kernel);

    cv::bitwise_not(imgCanny, dst);
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