#include <iostream>
#include "filter.hpp"
#define PI 3.14159265
using namespace std;

enum filter
{
    none,
    greyCVT,
    greyAlt,
    gaussBlur,
    sobelX,
    sobelY,
    mag,
    blurQ,
    cartoonize,
    drawOp,
    writeTxt
};

void orient(cv::Mat &sx, cv::Mat &sy, cv::Mat &dst)
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
            cv::Vec3s interOrient; // short

            // loop over channel
            for (int ch = 0; ch < 3; ch++)
            {
                short sxVal = sxPtr[i * sx.cols * 3 + j * 3 + ch];
                short syVal = syPtr[i * sx.cols * 3 + j * 3 + ch];
                interOrient[ch] = (signed short) atan2(syVal, sxVal)* 180 / PI;

                dstPtr[i * dst.cols * 3 + j * 3 + ch] = (unsigned char)interOrient[ch];
            }
        }
    }
}

int videoMode()
{
    cv::VideoCapture *capdev;
    bool record = false;

    // 1. Open the video device
    capdev = new cv::VideoCapture(0);
    if (!capdev->isOpened())
    {
        printf("Unable to open video device\n");
        return (-1);
    }

    // 2. Get video resolution and create a size object
    cv::Size refS((int)capdev->get(cv::CAP_PROP_FRAME_WIDTH),
                  (int)capdev->get(cv::CAP_PROP_FRAME_HEIGHT));
    printf("Expected size: %d %d\n", refS.width, refS.height);

    // 3. Create video writer object filename, format, size
    cv::VideoWriter output("myout.avi", cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 5, refS);

    cv::namedWindow("Video", 1); // 4. identifies a window
    cv::Mat srcFrame;            // 5. create srcFrame to display
    cv::Mat interFrame;
    cv::Mat dstFrame;
    cv::Mat sXFrame ;
    cv::Mat sYFrame;
    filter op = none;

    for (;;)
    {
        *capdev >> srcFrame; // 6. get a new frame from the camera, treat as a stream

        if (srcFrame.empty())
        {
            printf("srcFrame is empty\n");
            break;
        }

        // Record
        if(record == 1)
        {
            output.write(dstFrame);
        }
       
        // 7. Apply filters depending on key pressed
        if (op == greyCVT)
        {
            cv::cvtColor(srcFrame, dstFrame, cv::COLOR_BGR2GRAY);
        }
        else if (op == greyAlt)
        {
            greyscale(srcFrame, dstFrame);
        }
        else if (op == gaussBlur)
        {
            blur5x5(srcFrame, dstFrame);
        }
        else if (op == sobelX)
        {
            sobelX3x3(srcFrame, interFrame);

            // convert image back to unsign char and save to destination
            cv::convertScaleAbs(interFrame, dstFrame);
        }
        else if (op == sobelY)
        {
            sobelY3x3(srcFrame, interFrame);
            cv::convertScaleAbs(interFrame, dstFrame);
        }
        else if (op == mag)
        {
            // apply filter and save image as sobelX
            sobelX3x3(srcFrame, sXFrame);

            // apply filter and save frame as sobelY
            sobelY3x3(srcFrame, sYFrame);

            // magnitude
            orient(sXFrame, sYFrame, dstFrame);
        }
        else if (op == blurQ)
        {
            blurQuantize(srcFrame, dstFrame, 15);
        }
        else if(op == cartoonize)
        {
            cartoon(srcFrame, dstFrame, 20, 15);
        }
        else if(op == drawOp)
        {
            lineDraw(srcFrame, dstFrame);
        }
        else if(op == writeTxt)
        {
            writeText(srcFrame, dstFrame);
        }
        else
        { // op == none
            srcFrame.copyTo(dstFrame);
        }
        cv::imshow("Video", dstFrame);

        // 9. If key strokes are pressed, set flags
        char key = cv::waitKey(5);
        if (key == 'q')
        {
            cout << "Quit program." << endl;
            break;
        }
        else if (key == 's') // 10. save single image to jpeg
        {
            cout << "saving" << endl;
            cv::imwrite("myimg.jpg", dstFrame);
        }
        else if (key == 'r') // 11. record video to avi
        {
            cout << "Recording starts.. " << endl;
            record = true;
        }
        else if (key == 'g') // 12. task 3: greyscale cvtColor
        {
            cout << "Grey cvtColor..." << endl;
            op = greyCVT;
        }
        else if (key == 'h') // 13. task 4: greyscale alternative method
        {
            cout << "Grey alternative..." << endl;
            op = greyAlt;
        }
        else if (key == 'b') // 14. task 5:
        {
            cout << "Gaussian 5X5 blur.." << endl;
            op = gaussBlur;
        }
        else if (key == 'x') // 15. task 6
        {
            cout << "Sobel X filter.." << endl;
            op = sobelX;
        }
        else if (key == 'y')
        {
            cout << "Sobel Y filter.." << endl;
            op = sobelY;
        }
        else if (key == 'm')
        {
            cout << "mag sobel" << endl;
            op = mag;
        }
        else if (key == 'l')
        {
            cout << "blur quantize" << endl;
            op = blurQ;
        }
        else if (key == 'c')
        {
            cout << "cartoonize" << endl;
            op = cartoonize;
        }
        else if( key == 'o')
        {
            cout << "line draw" << endl;
            op = drawOp;
        }
        else if( key == 'w')
        {
            cout << "write text" << endl;
            op = writeTxt;
        }
        else if (key == 32)
        {
            cout << "Reset color..." << endl;
            op = none;
        }
        else if (key == -1)
        {
            continue;
        }
        else
        {
            cout << key << endl;
        }
    }
    // release video writer
    output.release();
    delete capdev;
    return (0);
}

void imageMode() {
    cv::Mat srcImage;
    cv::Mat dstImage;
    srcImage = cv::imread("res/checkerboard.png", 1);
    filter op = none;

    while (1)
    {
        // 1. Apply filters depending on key pressed
        if (op == gaussBlur)
        {
            blur5x5(srcImage, dstImage);
        }
        else
        { // op == none
            srcImage.copyTo(dstImage);
        }
        cv::imshow("image", dstImage);

        int k = cv::waitKey(0);

        // 8. check keys
        if (k == 'q') // 1. quit
        {
            break;
        }
        else if (k == 'b') // 2.blur
        {
            op = gaussBlur;
        }
        else if (k == 3) // 3. right arrow, rotate right
        {
            cv::rotate(srcImage, srcImage, cv::ROTATE_90_CLOCKWISE);
        }
        else if (k == 2) // 4. left arrow, rotate left
        {
            cv::rotate(srcImage, srcImage, cv::ROTATE_90_COUNTERCLOCKWISE);
        }
        else if (k == 32) // 5. Reset to original img
        {
            cout << "reset" << endl;
            k = -1;
            op = none;
        }
        else if (k == -1)
        {
            continue; // 7. normally -1 returned,so don't print it
        }
        else
        {
            cout << k << endl; // 8. else print its value
        }
    }
}


int main(int argc, char *argv[]) {

    char mode;
    cout << "enter mode: v video, i image" << endl;

    cin >> mode;

    while (mode != 'q') {
        if (mode == 'v') {
            videoMode();  
        } else if (mode == 'i') {
            imageMode();  
        } 
        cout << "enter mode: v video, i image, or q to quit" << endl;
        cin >> mode;
    }
}
