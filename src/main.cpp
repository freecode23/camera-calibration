#include <sys/stat.h>

#include <iostream>

#include "filter.hpp"
#define PI 3.14159265
using namespace std;

enum filter { none, gaussBlur, opDrawOnChessboard, opSaveImageWorldPoints };

string getNewFileName(string pathName, string imgName) {
    // create img name
    int fileIdx = 0;
    string imgNameCopy;
    imgNameCopy.append(to_string(fileIdx)).append(".png");

    // create full path
    // string pathName = "res/owntrial/";
    string path_copy = pathName;
    path_copy.append(imgNameCopy);
    struct stat buffer;
    bool isFileExist = (stat(path_copy.c_str(), &buffer) == 0);

    while (isFileExist) {
        fileIdx += 1;
        imgNameCopy = imgName;
        imgNameCopy.append(to_string(fileIdx)).append(".png");

        // pathName = "res/owntrial/";
        string path_copy = pathName;
        path_copy.append(imgNameCopy);
        isFileExist = (stat(path_copy.c_str(), &buffer) == 0);
    }
    // file does not exists retunr this name
    return imgNameCopy;
}

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
            // - save image points
            drawOnChessboard(srcFrame, dstFrame, imagePoints, chessboardSize);

            if (imagePoints.size() > 0) {
                // - save image
                string path_name = "res/";
                string imgName = getNewFileName(path_name, "calibration_");
                path_name.append(imgName);
                cout << "saving image at: " << path_name << endl;
                cv::imwrite(path_name, dstFrame);

                // push image points
                listImagePoints.push_back(vector<cv::Point2f>(imagePoints));

                // - save world points
                if (listWorldPoints.size() == 0) {
                    cout << "create the first world points" << endl;

                    // createWorldPoints(chessboardSize, worldPoints);
                    for (int i = 0; i < chessboardSize.height; i++) {
                        for (int j = 0; j < chessboardSize.width; j++) {
                            worldPoints.push_back(
                                cv::Point3f((float)j, (float)i * -1, 0));
                        }
                    }
                }

                // cout << "saved world points: " << worldPoints << endl;
                // cout << "saved image points: " << imagePoints << endl;
                // push worldPoints
                listWorldPoints.push_back(vector<cv::Point3f>(worldPoints));
            } else {
                cout << "no chessboard detected " << endl;
            }
            srcFrame.copyTo(dstFrame);
            
            // just draw chessboard again don't save until user ask
            op = opDrawOnChessboard;
            

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
            cout << "draw on chessboard.." << endl;
            op = opDrawOnChessboard;

        } else if (key == 's') {
            // save imagePoints
            op = opSaveImageWorldPoints;

        } else if (key == 'r') {
            cout << "Recording starts.. " << endl;
            record = true;

        } else if (key == 32) {
            cout << "Reset..." << endl;
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
