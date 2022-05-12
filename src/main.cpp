#include <sys/stat.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

#include "filter.hpp"

#define PI 3.14159265
using namespace std;

enum filter {
    none,
    gaussBlur,
    opDrawOnChessboard,
    opSaveImageWorldPoints,
    opCalibrate,
    opSaveImage,
    opCameraPosition,
    opProject
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

    // load the points from previously stored images
    // will stay empty if there is no previous data
    char src_csv[] = "res/imageWorldPoints.csv";
    vector<char *> imageNames;
    read2d3DVectorsFromCSV(src_csv, chessboardSize, listImagePoints,
                           listWorldPoints, imageNames, 0);

    cv::Mat calibMatrix;   // 3X3 matrix
    cv::Mat distortCoeff;  // 1X5 matrix

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
            // - draw last one
            drawOnChessboard(srcFrame, dstFrame, imagePoints, chessboardSize);
            string imgPrefix = "calibration_";

            if (imagePoints.size() > 0) {
                // - save image
                string imgName = saveImage(srcFrame, imgPrefix);

                // - save points in a csv and in 2 vectors
                char imgNameChar[256];
                strcpy(imgNameChar, imgName.c_str());
                savePointsCsvVector(chessboardSize, imagePoints, worldPoints,
                                    listImagePoints, listWorldPoints,
                                    imgNameChar, imageNames);

                cout << worldPoints << endl;

            } else {
                cout << "no chessboard detected " << endl;
                srcFrame.copyTo(dstFrame);
            }

            // just draw chessboard again don't save until user ask
            op = opDrawOnChessboard;

        } else if (op == opCalibrate) {
            // 1. Not enough image
            if (listImagePoints.size() < 5 || listWorldPoints.size() < 5) {
                cout << "you only have " << listImagePoints.size()
                     << " calibration images. Please add more" << endl;

            } else {  // 2. Start calibrating
                calibrating(srcFrame, listWorldPoints, listImagePoints,
                            imageNames);
            }

            srcFrame.copyTo(dstFrame);  // make sure video keep playing
            op = none;

        } else if (op == opCameraPosition) {
            // 1. get image points
            drawOnChessboard(srcFrame, dstFrame, imagePoints, chessboardSize);

            // 2. get rVec and tVec
            cv::Mat rotVec(3, 1, cv::DataType<double>::type);
            cv::Mat transVec(3, 1, cv::DataType<double>::type);
            if (getCameraPosition(chessboardSize, worldPoints, imagePoints,
                                  calibMatrix, distortCoeff, rotVec,
                                  transVec)) {
                // - print
                cv::Ptr<cv::Formatter> formatMat =
                    cv::Formatter::get(cv::Formatter::FMT_DEFAULT);

                formatMat->set64fPrecision(4);
                formatMat->set32fPrecision(4);
                cout << "\nrotation vector: \n"
                     << formatMat->format(rotVec) << endl;
                cout << "translation vector: \n"
                     << formatMat->format(transVec) << endl;

            } else {
                cout << "No camera with chessboard detected. Press \'P\' again "
                        "once you put your chessboard in front of camera"
                     << endl;
                op = none;
            }

        } else if (op == opProject) {
            // 1. get image points
            drawOnChessboard(srcFrame, dstFrame, imagePoints, chessboardSize);

            // 2. get rVec and tVec
            cv::Mat rotVec(3, 1, cv::DataType<double>::type);
            cv::Mat transVec(3, 1, cv::DataType<double>::type);

            if (getCameraPosition(chessboardSize, worldPoints, imagePoints,
                                  calibMatrix, distortCoeff, rotVec,
                                  transVec)) {
                draw3DAxesOnChessboard(srcFrame, calibMatrix, distortCoeff,
                                       rotVec, transVec);
                srcFrame.copyTo(dstFrame);
            } else {
                cout << "No camera with chessboard detected. Press \'P\' again "
                        "once you put your chessboard in front of camera"
                     << endl;
                op = none;
            }

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
            cout << "\n>>>>>>>>> draw on chessboard.." << endl;
            op = opDrawOnChessboard;

        } else if (key == 's') {
            // save imagePoints
            cout << "\n>>>>>>>>> saving chessboard" << endl;
            op = opSaveImageWorldPoints;

        } else if (key == 'c') {
            cout << "\n>>>>>>>>> calibrating" << endl;
            op = opCalibrate;

        } else if (key == 'r') {
            cout << "\n>>>>>>>>> recording starts.. " << endl;
            record = true;

        } else if (key == 'i') {
            saveImage(dstFrame, "realtime_");

        } else if (key == 't') {
            cout << "\n>>>>>>>>> calculate camera position..." << endl;
            op = opCameraPosition;

        } else if (key == 'p') {
            cout << "\n>>>>>>>>> project outside corners..." << endl;
            op = opProject;

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
