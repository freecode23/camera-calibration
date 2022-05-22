#include <sys/stat.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <opencv2/aruco.hpp>
#include <vector>

#include "filter.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/xfeatures2d.hpp"
using namespace std;

enum filter {
    none,
    gaussBlur,
    opDrawOnChessboard,
    opSaveImageWorldPoints,
    opCalibrate,
    opSaveImage,
    opCameraPosition,
    op3DAxes,
    opPolygon,
    opHarris,
    opDetectAruco,
    opFast,
    opVirtualObj
};

int videoMode() {
    cv::VideoCapture *capdev;
    cv::VideoCapture *capMovie;
    bool record = false;

    // 2. create movie and real time video capture
    capdev = new cv::VideoCapture(0);
    // capMovie = new cv::VideoCapture("res/sky.mp4");

    if (!capdev->isOpened()) {
        printf("Unable to open video device\n");
        return (-1);
    }

    // if (!capMovie->isOpened()) {
    //     cout << "Error opening video stream or file" << endl;
    //     return -1;
    // }

    capdev->set(cv::CAP_PROP_FRAME_WIDTH,
                600);  // Setting the width of the video
    capdev->set(cv::CAP_PROP_FRAME_HEIGHT,
                400);  // Setting the height of the video//

    // 2. Get video resolution and create a size object
    cv::Size refS((int)capdev->get(cv::CAP_PROP_FRAME_WIDTH),
                  (int)capdev->get(cv::CAP_PROP_FRAME_HEIGHT));
    printf("Expected size: %d %d\n", refS.width, refS.height);

    // 3. Create video writer object filename, format, size
    cv::VideoWriter output("myout.avi", cv::CAP_OPENCV_MJPEG,
                           cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 30,
                           refS, true);

    cv::namedWindow("Video", 1);  // 4. identifies a window
    cv::Mat srcFrame;             // 5. create srcFrame to display
    cv::Mat dstFrame;
    cv::Mat movieFrame;
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
    vector<cv::Point3f> vertices;
    std::vector<std::vector<int>> faces;
    string movFile;

        float x_shift;
    float y_shift;
    // readObjFile("res/shuttle.obj", vertices, faces);
    char intInput;
    for (;;) {
        // 1. get a new frame from the camera, treat as a stream
        *capdev >> srcFrame;

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
            cout << "corners found: " << imagePoints.size() << endl;
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
                cout << "No camera with chessboard detected. Press \'T\' again "
                        "once you put your chessboard in front of camera"
                     << endl;
                op = none;
            }

        } else if (op == op3DAxes) {
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
                cout << "No camera with chessboard detected. Press \'X\' again "
                        "once you put your chessboard in front of camera"
                     << endl;
                op = none;
            }

        } else if (op == opPolygon) {
            // 1. get image points
            drawOnChessboard(srcFrame, dstFrame, imagePoints, chessboardSize);

            // 2. get rVec and tVec
            cv::Mat rotVec(3, 1, cv::DataType<double>::type);
            cv::Mat transVec(3, 1, cv::DataType<double>::type);

            if (getCameraPosition(chessboardSize, worldPoints, imagePoints,
                                  calibMatrix, distortCoeff, rotVec,
                                  transVec)) {
                drawPolygonOnChessboard(srcFrame, calibMatrix, distortCoeff,
                                        rotVec, transVec);
                srcFrame.copyTo(dstFrame);
            } else {
                cout << "No camera with chessboard detected. Press \'L\' again "
                        "once you put your chessboard in front of camera"
                     << endl;
                op = none;
            }
        } else if (op == opHarris) {
            cout << "opHarris" << endl;
            int blockSize = 2;
            int apertureSize = 3;
            double k = 0.04;
            cv::Mat srcGray;
            int thresh = 120;
            int MAX_THRESH = 255;

            // 1. convert src to gray
            cv::cvtColor(srcFrame, srcGray, cv::COLOR_BGR2GRAY);

            // 2. get harris corner
            cv::Mat corners = cv::Mat::zeros(srcFrame.size(), CV_32FC1);

            cv::cornerHarris(srcGray, corners, blockSize, apertureSize, k);

            cv::Mat corners_norm, corners_norm_scaled;
            cv::normalize(corners, corners_norm, 0, 255, cv::NORM_MINMAX,
                          CV_32FC1, cv::Mat());
            cv::convertScaleAbs(corners_norm, corners_norm_scaled);

            srcFrame.copyTo(dstFrame);

            for (int i = 0; i < corners_norm.rows; i++) {
                for (int j = 0; j < corners_norm.cols; j++) {
                    if ((int)corners_norm.at<float>(i, j) > thresh) {
                        // draw circle
                        cv::circle(dstFrame, cv::Point(j, i), 5,
                                   cv::Scalar(0, 0, 255), 2, 8, 0);
                    }
                }
            }
            // cout << "finishcorner harris" << endl;

        } else if (op == opFast) {
            cv::Mat srcGray;
            // convert image to grey scale
            cv::cvtColor(srcFrame, srcGray, cv::COLOR_BGR2GRAY);
            std::vector<cv::KeyPoint> keypoints;

            // 1. SIFT
            // create sift Ptr and keypoints to
            // cv::Ptr<cv::SIFT> siftPtr = cv::SIFT::create(0, 3, 0.04,
            // 12, 1.6); siftPtr->detect(srcGray, keypoints);

            // 2. FAST
            cv::Ptr<cv::FastFeatureDetector> fastPtr =
                cv::FastFeatureDetector::create(
                    30, true, cv::FastFeatureDetector::TYPE_9_16);
            fastPtr->setNonmaxSuppression(false);
            fastPtr->detect(srcGray, keypoints);

            // 3. ORB
            // const int MAX_FEATURES = 500;
            // cv::Mat descriptors;
            // cv::Ptr<cv::Feature2D> orbPtr = cv::ORB::create();
            // orbPtr->detectAndCompute(srcGray, cv::Mat(), keypoints,
            // descriptors);

            cv::drawKeypoints(srcGray, keypoints, dstFrame, cv::Scalar::all(-1),
                              cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

        } else if (op == opDetectAruco) {
            // >> given a movie frame and frame with aruco corners, map movie to
            // the original srcFrame
            *capMovie >> movieFrame;
            if (movieFrame.empty()) {
                capMovie = new cv::VideoCapture("res/dog.mp4");
                *capMovie >> movieFrame;
            }
            createMovieOnAruco(srcFrame, movieFrame, dstFrame);

        } else if (op == opVirtualObj) {
        
            // 1. get image points
            drawOnChessboard(srcFrame, dstFrame, imagePoints, chessboardSize);

            // 2. get rVec and tVec
            cv::Mat rotVec(3, 1, cv::DataType<double>::type);
            cv::Mat transVec(3, 1, cv::DataType<double>::type);

            if (getCameraPosition(chessboardSize, worldPoints, imagePoints,
                                  calibMatrix, distortCoeff, rotVec,
                                  transVec)) {
                // >> given a movie frame and frame with aruco corners, map
                // movie to
                // the original srcFrame

                *capMovie >> movieFrame;
                if (movieFrame.empty()) {
                    cout << "filling mov file" << endl;
                    cout << movFile << endl;
                    capMovie = new cv::VideoCapture(movFile);
                    *capMovie >> movieFrame;
                }
                // *capMovie >> movieFrame;
                projectMovieOnChessboard(srcFrame, rotVec, transVec, calibMatrix, 
                distortCoeff, movieFrame, dstFrame);

                drawVirtualObjectOnChessboard(srcFrame, rotVec, transVec,
                                              calibMatrix, distortCoeff,
                                              vertices, faces, dstFrame);

                // srcFrame.copyTo(dstFrame);
            } else {
                cout << "No camera with chessboard detected. Press \'1, 2, or 3\' "
                        "again "
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

        } else if (key == 'x') {
            cout << "\n>>>>>>>>> project 3D axes..." << endl;
            op = op3DAxes;
        } else if (key == 'l') {
            cout << "\n>>>>>>>>> draw polygon object..." << endl;
            op = opPolygon;
        } else if (key == 'h') {
            cout << "\n>>>>>>>>> harris corner dection..." << endl;
            op = opHarris;
        } else if (key == 'a') {
            cout << "\n>>>>>>>>> aruco.." << endl;
            op = opDetectAruco;

        } else if (key == 'f') {
            cout << "fast corner detection.." << endl;
            op = opFast;

        } else if (key == '1') {
            cout << "\n>>>>>>>>> draw shuttle from obj file..." << endl;
            op = opVirtualObj;
   
            vertices.clear();
            faces.clear();
            readObjFile("res/shuttle.obj", vertices, faces);
            if (movieFrame.empty() || movFile != "res/space.mp4") {
                movFile = "res/space.mp4";
                capMovie = new cv::VideoCapture(movFile);
            }
        } else if (key == '2') {
            cout << "\n>>>>>>>>> draw cow from obj file..." << endl;
            op = opVirtualObj;

            vertices.clear();
            faces.clear();
            readObjFile("res/cow.obj", vertices, faces);
            if (movieFrame.empty() || movFile != "res/grass.mp4") {
                movFile = "res/grass.mp4";
                capMovie = new cv::VideoCapture(movFile);
            }

        } else if (key == '3') {
            cout << "\n>>>>>>>>> draw plane from obj file..." << endl;
            op = opVirtualObj;

            vertices.clear();
            faces.clear();
            readObjFile("res/plane.obj", vertices, faces);
            if (movieFrame.empty() || movFile != "res/sky.mp4") {
                movFile = "res/sky.mp4";
                capMovie = new cv::VideoCapture(movFile);
            }
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
    srcImage = cv::imread("res/sample.png", 1);
    filter op = none;

    while (1) {
        // 1. Execute operations depending on key pressed
        if (op == opDrawOnChessboard) {
            drawOnChessboard(srcImage, dstImage, imagePoints, chessboardSize);

        } else if (op == opDetectAruco) {
            // opencv method
            std::vector<int> markerIds;
            std::vector<std::vector<cv::Point2f>> markerCorners,
                rejectedCandidates;
            cv::Ptr<cv::aruco::DetectorParameters> parameters =
                cv::aruco::DetectorParameters::create();
            cv::Ptr<cv::aruco::Dictionary> dictionary =
                cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);

            // detect
            cv::aruco::detectMarkers(srcImage, dictionary, markerCorners,
                                     markerIds, parameters, rejectedCandidates);

            cout << "markerIds size=" << markerIds.size() << endl;
            srcImage.copyTo(dstImage);
            cv::aruco::drawDetectedMarkers(dstImage, markerCorners, markerIds);

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

        } else if (key == 'a') {
            cout << "arucos.." << endl;
            op = opDetectAruco;

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
