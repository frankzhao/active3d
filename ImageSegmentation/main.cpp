//
//  main.cpp
//  ImageSegmentation
//
//  Created by Frank Zhao on 23/02/2014.
//  Copyright (c) 2014 Frank Zhao. All rights reserved.
//

#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

int main(int argc, const char * argv[])
{

    Mat img = imread("/Users/frank/dev/COMP4550/dog.jpg");
    
    if (img.data == 0) {
        cerr << "Image not found!" << endl;
        return -1;
    }
    
    int rows = img.rows;
    int cols = img.cols;
    
    // initialise algorithm arrays with zeros
    Mat mask = Mat(rows, cols, CV_32FC1, double(0));
    Mat fgModel = Mat(1, 65, CV_64FC1, double(0));
    Mat bgModel = Mat(1, 65, CV_64FC1, double(0));
    
    int rectX = 0;
    int rectY = 0;
    int rectWidth  = 490;
    int rectHeight = 653;
    
    Rect rect = Rect(rectX, rectY, rectWidth, rectHeight);
    
    
    // GC_INIT_WITH_RECT -> using rectangle
    // GC_INIT_WITH_MACL -> using mask
    grabCut(img, mask, rect, bgModel, fgModel, 5, GC_INIT_WITH_RECT);
    
    // 0, 2 -> bg pixels; 1, 3 -> fg pixels
    Mat fgMask = img.clone();
    
    for (int i=0; i<rows; i++) {
        for (int j=0; j<cols; j++) {
            int v = mask.at<uchar>(i,j);
            if (v == 0 || v == 2) {
                fgMask.at<uchar>(i,j) = 0;
            } else if (v == 1 || v == 3) {
                fgMask.at<uchar>(i,j) = 1;
            }
        }
    }
    
    img.convertTo(img, CV_32FC3); // gemm needs float matrix
    // cvtColor(fgMask, frontMask, CV_GRAY2BGR); // convert from 1 channel mask to BW
    fgMask.convertTo(fgMask, CV_32FC3);
    img = img.mul(fgMask);
    img.convertTo(img, CV_8UC3);
    
    namedWindow("Viewer", CV_WINDOW_AUTOSIZE);
    imshow("Viewer", img);
    waitKey();
    
    return 0;
}

