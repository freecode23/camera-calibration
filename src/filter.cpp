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

#include <fstream>  //used for file handling
#include <iostream>
#include <string>  //used for strings
using namespace std;

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

/*
  reads a string from a CSV file fp. the 0-terminated string is returned in the
  char array os. The function returns false if it is successfully read. It
  returns true if it reaches the end of the line or the file.
 */
int getstring(FILE *fp, char os[]) {
    int p = 0;
    int eol = 0;

    for (;;) {
        char ch = fgetc(fp);
        if (ch == ',') {
            break;
        } else if (ch == '\n' || ch == EOF) {
            eol = 1;
            break;
        }
        // printf("%c", ch ); // uncomment for debugging
        os[p] = ch;
        p++;
    }
    // printf("\n"); // uncomment for debugging
    os[p] = '\0';
    cout << "calibration image name:" << os << endl;
    return (eol);  // return true if eol
}

int getint(FILE *fp, int *v) {
    char s[256];
    int p = 0;
    int eol = 0;

    for (;;) {
        char ch = fgetc(fp);
        if (ch == ',') {
            break;
        } else if (ch == '\n' || ch == EOF) {
            eol = 1;
            break;
        }

        s[p] = ch;
        p++;
    }
    s[p] = '\0';  // terminator
    *v = atoi(s);

    return (eol);  // return true if eol
}

/*
  Utility function for reading one float value from a CSV file
  The value is stored in the v parameter
  The function returns true if it reaches the end of a line or the file
 */
int getfloat(FILE *fp, float *v) {
    char s[256];
    int p = 0;
    int eol = 0;

    for (;;) {
        char ch = fgetc(fp);
        if (ch == ',') {
            break;
        } else if (ch == '\n' || ch == EOF) {
            eol = 1;
            break;
        }

        s[p] = ch;
        p++;
    }
    s[p] = '\0';  // terminator
    *v = atof(s);

    return (eol);  // return true if eol
}

string getNewFileName(string pathName, string imgName) {
    // create img name
    int fileIdx = 0;
    string imgNameCopy = imgName;
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

void drawOnChessboard(cv::Mat &src, cv::Mat &dst,
                      vector<cv::Point2f> &outputImagePoints,
                      cv::Size chessboardSize) {
    // 1. make grey frame
    cv::Mat srcGray;
    cv::cvtColor(src, srcGray, cv::COLOR_BGR2GRAY);

    // 2. find chessboardimagePoints
    bool found = findChessboardCorners(
        src, chessboardSize, outputImagePoints,
        cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_FILTER_QUADS);

    // cout << "corners: " <<imagePoints << "size: " << chessboardSize;

    // 3. parameters for corner subpix
    cv::Size wnSize = cv::Size(5, 5);
    cv::Size zeroZone = cv::Size(-1, -1);
    cv::TermCriteria criteria = cv::TermCriteria(
        cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 40, 0.001);
    ;

    // 4. if it finds something use cornersSubpix on the grey image to get more
    // accurate location
    src.copyTo(dst);
    if (found) {
        // cout << "found" << endl;
        cv::Size winSize = cv::Size(5, 5);
        cv::Size zeroZone = cv::Size(-1, -1);
        cv::cornerSubPix(srcGray, outputImagePoints, winSize, zeroZone,
                         criteria);

        cv::drawChessboardCorners(dst, chessboardSize, outputImagePoints,
                                  found);
    }
}

string saveImage(cv::Mat frame, string imgPrefix) {
    string pathName = "res/";
    string imgName = getNewFileName(pathName, imgPrefix);
    pathName.append(imgName);
    cout << "saving image at: " << pathName << endl;
    cv::imwrite(pathName, frame);

    return imgName;
}

/**
 * @brief utility function to save 2D and 3D points to csv file called
 * imageWorldPoints.csv
 *
 * @param v2 the 2d points
 * @param v3 the 3d points
 * @param csvfilepath the file to save these points to
 * @param image_filename the image file name
 * @param reset_file
 * @return int
 */
int appendPointVectorsToCsv(vector<cv::Point2f> &v2, vector<cv::Point3f> &v3,
                            char *csvfilepath, const char *image_filename,
                            int reset_file) {
    char buffer[256];
    char mode[8];
    FILE *fp;

    strcpy(mode, "a");

    if (reset_file) {
        strcpy(mode, "w");
    }

    fp = fopen(csvfilepath, mode);
    if (!fp) {
        printf("Unable to open output file %s\n", csvfilepath);
        exit(-1);
    }

    // write the filename and the feature vector to the CSV file
    strcpy(buffer, image_filename);

    std::fwrite(buffer, sizeof(char), strlen(buffer), fp);
    for (int i = 0; i < v2.size(); i++) {
        char tmp[256];
        sprintf(tmp, ",%.4f,%.4f", v2[i].x, v2[i].y);
        std::fwrite(tmp, sizeof(char), strlen(tmp), fp);
    }

    for (int i = 0; i < v3.size(); i++) {
        char tmp[256];
        sprintf(tmp, ",%.4f,%.4f,%.4f", v3[i].x, v3[i].y, v3[i].z);
        std::fwrite(tmp, sizeof(char), strlen(tmp), fp);
    }

    std::fwrite("\n", sizeof(char), 1, fp);  // EOL
    fclose(fp);
    return (0);
}

void appendRotationTranslationVector(cv::Mat rotationVec, cv::Mat translVec,
                                     char *&imageName, char *csvfilepath,
                                     int reset_file) {
    char buffer[256];
    char mode[8];
    FILE *fp;

    strcpy(mode, "a");  // append

    if (reset_file) {
        strcpy(mode, "w");
    }

    fp = fopen(csvfilepath, mode);
    if (!fp) {
        printf("Unable to open output file %s\n", csvfilepath);
        exit(-1);
    }

    // 1. write filename to buffer
    strcpy(buffer, imageName);
    std::fwrite(buffer, sizeof(char), strlen(buffer), fp);

    // 2. write rotation vector
    for (int j = 0; j < rotationVec.rows; j++) {
        cv::Vec3d vectorRow = rotationVec.at<cv::Vec3d>(j, 0);
        char tmp[256];
        sprintf(tmp, ",%.4f,%.4f,%.4f", vectorRow[0], vectorRow[1],
                vectorRow[2]);
        std::fwrite(tmp, sizeof(char), strlen(tmp), fp);
    }

    // 3. write translation vector
    //   for (int i = 0; i < translVec.rows; i++) {
    //     //   for (int j = 0; j < translVec.cols; j++) {
    //         // cv::Vec3d vectorRow = translVec.at<cv::Vec3d>(i, j);
    //         // cout << vectorRow << endl;
    //         // char tmp[256];
    //         // sprintf(tmp, ",%.4f,%.4f,%.4f", vectorRow[0], vectorRow[1],
    //         vectorRow[2]);
    //         // std::fwrite(tmp, sizeof(char), strlen(tmp), fp);
    //       }

    std::fwrite("\n", sizeof(char), 1, fp);  // EOL
    fclose(fp);
}

void savePointsCsvVector(cv::Size chessboardSize,
                         vector<cv::Point2f> &imagePoints,
                         vector<cv::Point3f> &worldPoints,
                         vector<vector<cv::Point2f>> &listImagePoints,
                         vector<vector<cv::Point3f>> &listWorldPoints,
                         char* imgName, std::vector<char *> &imageNames) {
    if (imagePoints.size() > 0) {
        // 1. save image points to vector 
        listImagePoints.push_back(vector<cv::Point2f>(imagePoints));

        // - create world points if it doesnt exist yet
        if (worldPoints.size() == 0) {
            cout << "create the first world points" << endl;
            for (int i = 0; i < chessboardSize.height; i++) {
                for (int j = 0; j < chessboardSize.width; j++) {
                    worldPoints.push_back(
                        cv::Point3f((float)j * 1.0, (float)i * -1.0, 0));
                }
            }
        }

        // 2. save world points to vector 
        listWorldPoints.push_back(vector<cv::Point3f>(worldPoints));

        // 3. write to csv
        char csvFile[] = "res/imageWorldPoints.csv";

        char *imgNameChar = new char[strlen(imgName) + 1];
        strcpy(imgNameChar, imgName);
        imageNames.push_back(imgNameChar);

        for (int i = 0; i < imageNames.size(); i++) {
            cout << imageNames.at(i) << endl;
        }
        appendPointVectorsToCsv(imagePoints, worldPoints, csvFile, imgNameChar, 0);

    } else {
        cout << "no chessboard detected " << endl;
    }
}

int read2d3DVectorsFromCSV(char *src_csv, cv::Size chessboardSize,
                           vector<vector<cv::Point2f>> &listImagePoints,
                           vector<vector<cv::Point3f>> &listWorldPoints,
                           std::vector<char *> &imageNames, int echo_file) {
    FILE *fp;
    char img_file[256];

    fp = fopen(src_csv, "r");
    if (!fp) {
        printf(
            "Unable to open calibration file. You might not have start "
            "calibrating yet\n");
        return (-1);
    }

    printf("Reading %s\n", src_csv);

    int numPoints = chessboardSize.width * chessboardSize.height;

    // 2. get the 2D vector and 3D vector of 1 image
    int xImage = 0;
    for (;;) {
        cout << "\n>>>> printing image: " << xImage << endl;
        xImage += 1;
        vector<cv::Point3f> singleImage3f;  // Point3f vector of a single image
        vector<cv::Point2f> singleImage2f;  // Point2f vector of a single image
        // 1. read the image name
        if (getstring(fp, img_file)) {
            break;
        }
        // save the image name
        char *fname = new char[strlen(img_file) + 1];
        strcpy(fname, img_file);
        imageNames.push_back(fname);
        float eol;

        // 3. for 54 times (in one image) get 2D
        cv::Point2f imagePoint;
        for (int i = 0; i < numPoints; i++) {
            // cout << "imagePoint" << i << "=";
            float fval;

            // 4. get the value and check if its end of the line
            getfloat(fp, &fval);
            imagePoint.x = fval;

            getfloat(fp, &fval);
            imagePoint.y = fval;

            // cout << imagePoint.x << "," << imagePoint.y << endl;
            // 5. create 3D point and push to single vector
            singleImage2f.push_back(cv::Point2f(imagePoint.x, imagePoint.y));
        }

        // push to the list of vector
        listImagePoints.push_back(singleImage2f);

        // 6. for 54 times get 3D
        // cout << "\n>>>> printing world points " << endl;
        cv::Point3f worldPoint;
        for (int i = 0; i < numPoints; i++) {
            // cout << "worldPoint" << i << "=";
            float fval;
            // - get the value and check if its end of the line
            getfloat(fp, &fval);
            worldPoint.x = fval;

            getfloat(fp, &fval);
            worldPoint.y = fval;

            eol = getfloat(fp, &fval);
            worldPoint.z = fval;

            // 7. push to single vector
            // cout << worldPoint.x << "," << worldPoint.y << "," <<
            // worldPoint.z
            //      << endl;
            singleImage3f.push_back(
                cv::Point3f(worldPoint.x, worldPoint.y, worldPoint.z));
        }
        listWorldPoints.push_back(singleImage3f);

        // if (eol) {
        //     cout << "end of line" << endl;
        //     break;
        // }

    }  // end of loop of image

    // --> print out the list of 2D to debug
    // cout << ">>>> printing image points << " << endl;
    // for (int i = 0; i < listImagePoints.size(); i++) {
    //     cout << i << "=" << listImagePoints.at(i) << endl;
    // }

    // --> print out the list of 3D to debug
    // cout << "printing world points << " << endl;
    // for (int i = 0; i < listWorldPoints.size(); i++) {
    //     cout << i << "=" << listWorldPoints.at(i) << endl;
    // }

    fclose(fp);
    printf("Finished reading CSV file\n");
    return (0);
}
