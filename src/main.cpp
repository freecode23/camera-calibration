#include <sys/stat.h>

#include <iostream>

#include "filter.hpp"
#define PI 3.14159265
using namespace std;

enum filter {
    none,
    gaussBlur,
    opDrawOnChessboard,
    opSaveImageWorldPoints,
    opCalibrate,
    opSaveImage
};

int videoMode() {
    cv::VideoCapture *capdev;
    bool record = false;

    // 1. Open the video device
    capdev = new cv::VideoCapture(0);
    if (!capdev->isOpened()) {
        printf("Unable to open video device\n");
        return (-1);
    }
    capdev->set(cv::CAP_PROP_FRAME_WIDTH,
                600);  // Setting the width of the video
    capdev->set(cv::CAP_PROP_FRAME_HEIGHT,
                400);  // Setting the height of the video//

    // 2. Get video resolution and create a size object

    cv::Size refS((int)capdev->get(cv::CAP_PROP_FRAME_WIDTH),
                  (int)capdev->get(cv::CAP_PROP_FRAME_HEIGHT));
    printf("Expected size: %d %d\n", refS.width, refS.height);

    // 3. Create video writer object filename, format, size
    cv::VideoWriter output(
        "myout.avi", cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 5, refS);

    cv::namedWindow("Video", 1);  // 4. identifies a window
    cv::Mat srcFrame;             // 5. create srcFrame to display
    cv::Mat dstFrame;
    filter op = none;

    // chessboard Size
    cv::Size chessboardSize(9, 6);

    // last image point
    vector<cv::Point2f> imagePoints;
    vector<cv::Point3f> worldPoints;

    // list of points for N images we picked
    vector<vector<cv::Point2f>> listImagePoints;
    vector<vector<cv::Point3f>> listWorldPoints;

    for (;;) {
        *capdev >>
            srcFrame;  // 6. get a new frame from the camera, treat as a stream

        if (srcFrame.empty()) {
            printf("srcFrame is empty\n");
            break;
        }

        // Record
        if (record == 1) {
            output.write(dstFrame);
        }

        // 7. Execute operation
        if (op == opDrawOnChessboard) {
            drawOnChessboard(srcFrame, dstFrame, imagePoints, chessboardSize);

        } else if (op == opSaveImageWorldPoints) {
            // draw last one
            drawOnChessboard(srcFrame, dstFrame, imagePoints, chessboardSize);
            string imgPrefix = "calibration_";

            if (imagePoints.size() > 0) {
                // save points and image
                savePoints(chessboardSize, imagePoints, worldPoints,
                           listImagePoints, listWorldPoints);

                // save image without the points
                saveImage(srcFrame, imgPrefix);

            } else {
                cout << "no chessboard detected " << endl;
                srcFrame.copyTo(dstFrame);
            }

            // just draw chessboard again don't save until user ask
            op = opDrawOnChessboard;

        } else if (op == opCalibrate) {
            if (listImagePoints.size() < 5 || listWorldPoints.size() < 5) {
            } else {
                // 1. extrinsic parameter output
                vector<cv::Mat> rotationVec;
                vector<cv::Mat> translVec;

                // 2. intrinsic output
                float calibVal[3][3] = {{1, 0, (float)srcFrame.cols / 2},
                                        {0, 1, (float)srcFrame.cols / 2},
                                        {0, 0, 1}};
                cv::Mat calibMatrix = cv::Mat(1, 10, CV_32F, calibVal);
                cv::Mat distortCoeffs;
                cout << calibMatrix << endl;

                // 3. optional flags
                int flag = 0;
                // flag |= CALIB_FIX_ASPECT_RATIO;

                // 4. get the projection matrix and record the error
                double error = cv::calibrateCamera(
                    listWorldPoints, listImagePoints, srcFrame.size(),
                    calibMatrix, distortCoeffs, rotationVec, translVec);
                cout << "projection error: " << error << endl;
            }

            srcFrame.copyTo(dstFrame); // make sure video keep playing
            op = none;
        } else {  // op == none
            srcFrame.copyTo(dstFrame);
        }

        cv::imshow("Video", dstFrame);

        // 9. If key strokes are pressed, set flags
        char key = cv::waitKey(5);
        if (key == 'q') {
            cout << "Quit program." << endl;
            break;

        } else if (key == 'd') {
            cout << ">>>>>>>>> draw on chessboard.." << endl;
            op = opDrawOnChessboard;

        } else if (key == 's') {
            // save imagePoints
            cout << ">>>>>>>>> saving chessboard" << endl;
            op = opSaveImageWorldPoints;

        } else if (key == 'c') {
            cout << ">>>>>>>>> calibrating" << endl;
            op = opCalibrate;

        } else if (key == 'r') {
            cout << ">>>>>>>>> recording starts.. " << endl;
            record = true;

        } else if (key == 'i') {
            saveImage(dstFrame, "realtime_");

        } else if (key == 32) {
            cout << ">>>>>>>>> reset..." << endl;
            op = none;

        } else if (key == -1) {
            continue;

        } else {
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

    // chessboard Size
    cv::Size chessboardSize(9, 6);

    // last image point
    vector<cv::Point2f> imagePoints;
    vector<cv::Point3f> worldPoints;

    // list of points for N images we picked
    vector<vector<cv::Point2f>> listImagePoints;
    vector<vector<cv::Point3f>> listWorldPoints;
    srcImage = cv::imread("res/own2.png", 1);
    filter op = none;

    while (1) {
        // 1. Execute operations depending on key pressed
        if (op == opDrawOnChessboard) {
            drawOnChessboard(srcImage, dstImage, imagePoints, chessboardSize);

        } else {  // op == none
            srcImage.copyTo(dstImage);
        }
        cv::imshow("image", dstImage);

        int key = cv::waitKey(0);

        // 8. check keys
        if (key == 'q') {
            break;
        } else if (key == 'd') {
            cout << "draw on chessboard.." << endl;
            op = opDrawOnChessboard;

        } else if (key == 32) {
            cout << "reset" << endl;
            key = -1;
            op = none;

        } else if (key == -1) {
            continue;
        } else {
            cout << key << endl;
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
