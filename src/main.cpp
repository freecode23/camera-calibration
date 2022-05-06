#include <sys/stat.h>

#include <iostream>

#include "filter.hpp"
#define PI 3.14159265
using namespace std;

enum filter {
    none,
    greyCVT,
    greyAlt,
    gaussBlur,
};


string getNewFileName(string path_name) {
    // create img name
    int fileIdx = 0;
    string img_name = "own";
    img_name.append(to_string(fileIdx)).append(".png");

    // create full path
    // string path_name = "res/owntrial/";
    string path_copy = path_name;
    path_copy.append(img_name);
    struct stat buffer;
    bool isFileExist = (stat(path_copy.c_str(), &buffer) == 0);

    while (isFileExist) {
        fileIdx += 1;
        img_name = "own";
        img_name.append(to_string(fileIdx)).append(".png");

        // path_name = "res/owntrial/";
        string path_copy = path_name;
        path_copy.append(img_name);
        isFileExist = (stat(path_copy.c_str(), &buffer) == 0);
    }
    // file does not exists retunr this name
    return img_name;
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

    for (;;) {
        *capdev >> srcFrame; // 6. get a new frame from the camera, treat as a stream

        if (srcFrame.empty()) {
            printf("srcFrame is empty\n");
            break;
        }

        // Record
        if(record == 1) {
            output.write(dstFrame);
        }
       
        // 7. Apply filters depending on key pressed
        if (op == greyCVT) {
            cv::cvtColor(srcFrame, dstFrame, cv::COLOR_BGR2GRAY);
        } else if (op == gaussBlur) {
            blur5x5(srcFrame, dstFrame);
        } else { // op == none
            srcFrame.copyTo(dstFrame);
        }
        cv::imshow("Video", dstFrame);

        // 9. If key strokes are pressed, set flags
        char key = cv::waitKey(5);
        if (key == 'q') {
            cout << "Quit program." << endl;
            break;

        }  else if (key == 's') {
            cout << "saving file" <<endl;
            string path_name = "res/";
            string img_name = getNewFileName(path_name);
            path_name.append(img_name);

            cv::imwrite(path_name, dstFrame);
            cv::imwrite("res/myimg.jpg", dstFrame);

        } else if (key == 'r') {
            cout << "Recording starts.. " << endl;
            record = true;

        } else if (key == 'g') {
            cout << "Grey cvtColor..." << endl;
            op = greyCVT;

        } else if (key == 'b') {
            cout << "Gaussian 5X5 blur.." << endl;
            op = gaussBlur;
        } else if (key == 32) {
            cout << "Reset color..." << endl;
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
    srcImage = cv::imread("res/checkerboard.png", 1);
    filter op = none;

    while (1) {
        // 1. Apply filters depending on key pressed
        if (op == gaussBlur) {
            blur5x5(srcImage, dstImage);

        } else if (op == greyCVT) {

        }else {  // op == none
            srcImage.copyTo(dstImage);
        }
        cv::imshow("image", dstImage);

        int k = cv::waitKey(0);

        // 8. check keys
        if (k == 'q') {
            break;
        } else if(k == 'g') {
            cout << "greyscale.." << endl;
            op = greyCVT;

        } else if (k == 'b') {
            cout << "blurring.." << endl;
            op = gaussBlur;

        } else if (k == 2) {
            cv::rotate(srcImage, srcImage, cv::ROTATE_90_COUNTERCLOCKWISE);

        } else if (k == 32) {
            cout << "reset" << endl;
            k = -1;
            op = none;

        } else if (k == -1) {
            continue;  // 7. normally -1 returned,so don't print it
        } else {
            cout << k << endl;  // 8. else print its value
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
