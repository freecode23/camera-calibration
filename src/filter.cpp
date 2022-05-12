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
#include <opencv2/viz.hpp>
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

// >>>>>>>>>>> Task1
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

// >>>>>>>>>>> Task2
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

void savePointsCsvVector(cv::Size chessboardSize,
                         vector<cv::Point2f> &imagePoints,
                         vector<cv::Point3f> &worldPoints,
                         vector<vector<cv::Point2f>> &listImagePoints,
                         vector<vector<cv::Point3f>> &listWorldPoints,
                         char *imgName, std::vector<char *> &imageNames) {
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
        appendPointVectorsToCsv(imagePoints, worldPoints, csvFile, imgNameChar,
                                0);

    } else {
        cout << "no chessboard detected " << endl;
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

// >>>>>>>>>>>>> Task3
// util functions to append vectors to csv files
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
    string w = "w";
    string modestr = string(mode);
    if (w.compare(modestr) == 0) {
        // 1. title
        char title[] =
            "imageName,rotRow_0,rotRow_1,rotRow2,tranlRow_0,tranlRow_1,"
            "tranlRow2";
        std::fwrite(title, sizeof(char), strlen(title), fp);
        std::fwrite("\n", sizeof(char), 1, fp);
    }

    // 1. write filename to buffer
    strcpy(buffer, imageName);
    std::fwrite(buffer, sizeof(char), strlen(buffer), fp);

    // 2. write rotation vector
    for (int i = 0; i < rotationVec.rows; i++) {
        for (int j = 0; j < rotationVec.cols; j++) {
            // - grab a single row
            char tmp[256];
            sprintf(tmp, ",%.4f", rotationVec.at<double>(i, j));
            std::fwrite(tmp, sizeof(char), strlen(tmp), fp);
        }
    }

    // 3. write translation vector (3rows X 1 col)
    for (int i = 0; i < translVec.rows; i++) {
        for (int j = 0; j < translVec.cols; j++) {
            // - grab a single row
            char tmp[256];
            sprintf(tmp, ",%.4f", translVec.at<double>(i, j));
            std::fwrite(tmp, sizeof(char), strlen(tmp), fp);
        }
    }

    std::fwrite("\n", sizeof(char), 1, fp);  // EOL
    fclose(fp);
}

void appendDistortionCalibMatrix(cv::Mat distortCoeff, cv::Mat calibMatrix,
                                 char *csvfilepath, int reset_file) {
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

    // 1. name
    char calibName[] = "calib_matrix";
    std::fwrite(calibName, sizeof(char), strlen(calibName), fp);

    // 2. calibration matrix
    for (int i = 0; i < calibMatrix.rows; i++) {
        for (int j = 0; j < calibMatrix.cols; j++) {
            // - append the row to csv
            char tmp[256];
            sprintf(tmp, ",%.4f", calibMatrix.at<double>(i, j));
            std::fwrite(tmp, sizeof(char), strlen(tmp), fp);
        }
    }
    std::fwrite("\n", sizeof(char), 1, fp);  // EOL

    char distortName[] = "distortion_coeff";
    std::fwrite(distortName, sizeof(char), strlen(distortName), fp);

    // 3. write distortion coeff
    for (int i = 0; i < distortCoeff.rows; i++) {
        for (int j = 0; j < distortCoeff.cols; j++) {
            // - append the row to csv
            char tmp[256];
            sprintf(tmp, ",%.4f", distortCoeff.at<double>(i, j));
            std::fwrite(tmp, sizeof(char), strlen(tmp), fp);
        }
    }
    std::fwrite("\n", sizeof(char), 1, fp);  // EOL
    fclose(fp);
}

void calibrating(cv::Mat srcFrame, vector<vector<cv::Point3f>> &listWorldPoints,
                 vector<vector<cv::Point2f>> &listImagePoints,
                 std::vector<char *> &imageNames) {
    // 1, extrinsic output
    vector<cv::Mat> rotationVecs;
    vector<cv::Mat> translVecs;

    // 2. intrinsic output
    double calibVal[3][3] = {{1, 0, (double)srcFrame.cols / 2},
                             {0, 1, (double)srcFrame.rows / 2},
                             {0, 0, 1}};
    cv::Mat calibMatrix = cv::Mat(3, 3, CV_64FC1, &calibVal);
    cv::Mat distortCoeff;

    // 3. get the projection matrix and record the error
    double error = cv::calibrateCamera(listWorldPoints, listImagePoints, srcFrame.size(),
                                        calibMatrix, distortCoeff, rotationVecs, translVecs);

    cout << "\n=====Calibration result:" << endl;
    cv::Ptr<cv::Formatter> formatMat =
        cv::Formatter::get(cv::Formatter::FMT_DEFAULT);
    formatMat->set64fPrecision(4);
    formatMat->set32fPrecision(4);
    cout << "camera matrix:\n" << formatMat->format(calibMatrix) << endl;
    cout << "distortion coeff: " << formatMat->format(distortCoeff) << endl;
    cout << "projection error: " << error << endl;

    // debug column
    // cout << "distor coeff row col : " << distortCoeff.rows << "X"
    //      << distortCoeff.cols << endl;
    // cout << "rVec row col: " << rotationVecs.at(0).rows << "X"
    //      << rotationVecs.at(0).cols << endl;
    // cout << "tVec row col: " << translVecs.at(0).rows << "X"
    //      << translVecs.at(0).cols << endl;

    // 4. Write to csv
    char distortCalibCsv[] = "res/distortionCalibMatrix.csv";
    cout << "\n- saving distortion coeff and camera matrix to "
         << string(distortCalibCsv) << endl;
    appendDistortionCalibMatrix(distortCoeff, calibMatrix, distortCalibCsv, 1);

    char rtCsv[] = "res/rt.csv";
    cout << "-saving rotation and translation matrix to " << string(rtCsv)
         << endl;

    // -- overwrite at first
    appendRotationTranslationVector(rotationVecs.at(0), translVecs.at(0),
                                    imageNames.at(0), rtCsv, 1);

    for (int i = 1; i < rotationVecs.size(); i++) {
        // -- append to csv
        appendRotationTranslationVector(rotationVecs.at(i), translVecs.at(i),
                                        imageNames.at(i), rtCsv, 0);
    }
}

// >>>>>>>>>>>>> Task4

/**
 * @brief Util function for task 4 to load distortion coeff and calibration
 * matrix from a csv file.
 *
 * @param src_csv the csv filek
 * @param calibMatrix the calibration matrix
 * @param distortCoeff the distrotion coefficient
 */
void readCalibDistorCoeffFromCSV(char *src_csv, cv::Mat &calibMatrix,
                                 cv::Mat &distortCoeff) {
    calibMatrix = cv::Mat::zeros(3, 3, CV_64FC1);   // 3X3 matrix
    distortCoeff = cv::Mat::zeros(1, 5, CV_64FC1);  // 1X5 matrix

    FILE *fp;
    char img_file[256];

    fp = fopen(src_csv, "r");
    if (!fp) {
        printf("Unable to open file\n");
    }

    printf("\n>>>>>> Reading calibration matrix and coef %s\n", src_csv);

    // 1. calibration matrix
    getstring(fp, img_file);           // read "calibMatrix"
    for (int i = 0; i < 3; i++) {      // row
        for (int j = 0; j < 3; j++) {  // col
            float fval;
            float eol = getfloat(fp, &fval);
            calibMatrix.at<double>(i, j) = fval;
        }
    }

    // 2. distortion coeff
    getstring(fp, img_file);
    for (int j = 0; j < 5; j++) {  // cols
        float fval;
        float eol = getfloat(fp, &fval);
        distortCoeff.at<double>(0, j) = fval;
    }

    fclose(fp);
    printf("Finished reading CSV file\n");
}

bool getCameraPosition(cv::Size chessboardSize,
                       vector<cv::Point3f> &worldPoints,
                       vector<cv::Point2f> &imagePoints, cv::Mat &calibMatrix,
                       cv::Mat &distortCoeff, cv::Mat &rotVec,
                       cv::Mat &transVec) {
    if (imagePoints.size() > 0) {
        // - load calibration matrix and distort coeff if its not
        // initialized yet
        if (calibMatrix.empty() || distortCoeff.empty()) {
            char distortCalibCsv[] = "res/distortionCalibMatrix.csv";
            readCalibDistorCoeffFromCSV(distortCalibCsv, calibMatrix,
                                        distortCoeff);
        }

        // - create world points if it doesnt exist yet
        if (worldPoints.size() == 0) {
            cout << "2. create the first world points" << endl;
            for (int i = 0; i < chessboardSize.height; i++) {
                for (int j = 0; j < chessboardSize.width; j++) {
                    worldPoints.push_back(
                        cv::Point3f((float)j * 1.0, (float)i * -1.0, 0));
                }
            }
        }

        rotVec = cv::Mat(3, 1, cv::DataType<double>::type);
        transVec = cv::Mat(3, 1, cv::DataType<double>::type);

        // - solve
        cv::solvePnP(worldPoints, imagePoints, calibMatrix, distortCoeff,
                     rotVec, transVec);

        return true;

    } else {
        return false;
    }
}

// >>>>>>>>>>>>> Task5

void draw2DLines(cv::Mat &srcFrame, cv::Point3f origin_3D, cv::Point3f dst_3D,
                 cv::Mat &calibMatrix, cv::Mat &distortCoeff, cv::Mat &rotVec,
                 cv::Mat &transVec, cv::Scalar color) {
    // create 3d points to do projection of
    // line 3D
    vector<cv::Point3f> vec3D;
    vec3D.push_back(origin_3D);
    vec3D.push_back(dst_3D);

    // line 2D
    cv::Point2f origin_2D = cv::Point2f(0, 0);
    cv::Point2f dst_2D = cv::Point2f(0, 0);
    vector<cv::Point2f> vec2D;
    vec2D.push_back(origin_2D);
    vec2D.push_back(dst_2D);

    // project the 3D
    cv::projectPoints(vec3D, rotVec, transVec, calibMatrix, distortCoeff,
                      vec2D);

    int thickness = 2;
    cv::line(srcFrame, vec2D.at(0), vec2D.at(1), color, thickness, cv::LINE_8);
}

// void draw3DAxesOnChessboard(cv::Mat &srcFrame, cv::Mat &calibMatrix,
//                             cv::Mat &distortCoeff, cv::Mat &rotVec,
//                             cv::Mat &transVec) {
//     // create 3d points to do projection of
//     cv::Point3f origin_3D = cv::Point3f(0, 0, 0);

//     // X axes 3D
//     cv::Point3f x_3D = cv::Point3f(6, 0, 0);
//     draw2DLines(srcFrame, origin_3D, x_3D, calibMatrix, distortCoeff, rotVec,
//                 transVec, cv::Scalar(255, 0, 0));

//     // Y AXes
//     cv::Point3f y_3D = cv::Point3f(0, -6, 0);
//     draw2DLines(srcFrame, origin_3D, y_3D, calibMatrix, distortCoeff, rotVec,
//                 transVec, cv::Scalar(0, 255, 0));

//     // Z AXes
//     cv::Point3f z_3D = cv::Point3f(0, 0, 6);
//     draw2DLines(srcFrame, origin_3D, z_3D, calibMatrix, distortCoeff, rotVec,
//                 transVec, cv::Scalar(0, 0, 255));
// }

// Task 6

void draw3DAxesOnChessboard(cv::Mat &srcFrame, cv::Mat &calibMatrix,
                            cv::Mat &distortCoeff, cv::Mat &rotVec,
                            cv::Mat &transVec) {
    vector<cv::Point3f> lines;
    // 0.73156748, 0.77794309, 0.42523719], [0.62113842, 0.87886158,
    // 0.12113842], [0.77794309, 0.92523719, 0.26843252], [0.57476281,
    // 0.73156748, 0.27794309], [0.87886158, 0.62113842, 0.37886158],
    // [0.72205691, 0.57476281, 0.23156748], [0.76843252, 0.72205691,
    // 0.07476281], [0.92523719, 0.76843252, 0.22205691
    lines.push_back(cv::Point3f(1, 0, 4));
    lines.push_back(cv::Point3f(0, 0, 1));
    lines.push_back(cv::Point3f(1, -4, 3));
    lines.push_back(cv::Point3f(0, -4, 1));
    lines.push_back(cv::Point3f(4, -4, 1));

    for (int i = 0; i < lines.size(); i++) {
        for (int j = 0; j < lines.size(); j++) {
            draw2DLines(srcFrame, lines.at(i), lines.at(j), calibMatrix,
                        distortCoeff, rotVec, transVec, cv::Scalar(255, 0, 0));
        }
    }

    struct dataType {
        cv::Point3d point;
        int red;
        int green;
        int blue;
    };
    typedef dataType SpacePoint;
    vector<SpacePoint> pointCloud;

    ofstream outfile("res/pointcloud.ply");
    outfile << "ply\n"
            << "format ascii 1.0\n"
            << "comment VTK generated PLY File\n";
    outfile << "obj_info vtkPolyData points and polygons : vtk4.0\n"
            << "element vertex " << pointCloud.size() << "\n";
    outfile << "property float x\n"
            << "property float y\n"
            << "property float z\n"
            << "element face 0\n";
    outfile << "property list uchar int vertex_indices\n"
            << "end_header\n";
    for (int i = 0; i < pointCloud.size(); i++) {
        cv::Point3d point = pointCloud.at(i).point;
        cout << point.x << " ";
        cout << point.y << " ";
        cout << point.z << " ";
        cout << "\n";
    }
    outfile.close();
}

// >>>>>>>>>>> Util
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

    // 1. get the 2D vector and 3D vector of 1 image
    int xImage = 0;
    for (;;) {
        xImage += 1;
        vector<cv::Point3f> singleImage3f;  // Point3f vector of a single image
        vector<cv::Point2f> singleImage2f;  // Point2f vector of a single image

        // 2. read the image name
        if (getstring(fp, img_file)) {
            break;
        }

        // 3. save the image name to vector
        char *fname = new char[strlen(img_file) + 1];
        strcpy(fname, img_file);
        imageNames.push_back(fname);

        // 4. for 54 times (in one image) get 2D
        cv::Point2f imagePoint;
        for (int i = 0; i < numPoints; i++) {
            // cout << "imagePoint" << i << "=";
            float fval;

            // 4. get the value and check if its end of the line
            getfloat(fp, &fval);
            imagePoint.x = fval;

            getfloat(fp, &fval);
            imagePoint.y = fval;

            // 5. create 3D point and push to single vector
            singleImage2f.push_back(cv::Point2f(imagePoint.x, imagePoint.y));
        }

        // push to the list of vector
        listImagePoints.push_back(singleImage2f);

        // 5. for 54 times get 3D
        // cout << "\n>>>> printing world points " << endl;
        cv::Point3f worldPoint;
        for (int i = 0; i < numPoints; i++) {
            float fval;
            // - get the value and check if its end of the line
            getfloat(fp, &fval);
            worldPoint.x = fval;

            getfloat(fp, &fval);
            worldPoint.y = fval;

            getfloat(fp, &fval);
            worldPoint.z = fval;

            // 7. push to single vector
            singleImage3f.push_back(
                cv::Point3f(worldPoint.x, worldPoint.y, worldPoint.z));
        }
        listWorldPoints.push_back(singleImage3f);

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