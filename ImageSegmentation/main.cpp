//
//  main.cpp
//  ImageSegmentation
//
//  Created by Frank Zhao on 23/02/2014.
//  Copyright (c) 2014 Frank Zhao. All rights reserved.
//

#include <iostream>
#include <opencv2/opencv.hpp>
#include <stdlib.h>

using namespace cv;
using namespace std;

int mouseX1 = 0, mouseY1 = 0, mouseX2 = 0, mouseY2 = 0;
bool drag = false;

Mat img;
Mat imgWorkingCopy;

static void interactiveGrabCut() {
    int rows = img.rows;
    int cols = img.cols;
    
    //printf("%d, %d\n", rows, cols);
    
    // initialise algorithm arrays with zeros
    Mat mask = Mat(rows, cols, CV_32FC1, double(0));
    Mat fgModel = Mat(1, 65, CV_64FC1, double(0));
    Mat bgModel = Mat(1, 65, CV_64FC1, double(0));
    
    // check that the area of the selected rectangle is > 0
    if ( abs((mouseX1 - mouseX2) * (mouseY2 - mouseY1)) > 0 ) {
        
        // TODO error when rectangle is the same size as image
        Rect rect = Rect( Point(mouseX1, mouseY1), Point (mouseX2, mouseY2) );
        
        
        // GC_INIT_WITH_RECT -> using rectangle
        // GC_INIT_WITH_MACL -> using mask
        grabCut(img, mask, rect, bgModel, fgModel, 5, GC_INIT_WITH_RECT);
        
        // 0, 2 -> bg pixels; 1, 3 -> fg pixels
        Mat fgMask = mask.clone();
        
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
        
        Mat out = mask.clone();
        
        img.convertTo(img, CV_32FC3); // gemm needs float matrix
        
        cvtColor(fgMask, fgMask, CV_GRAY2BGR); // convert from 8-bit single channel mask to 3 channel
        fgMask.convertTo(fgMask, CV_32FC3);
        img = img.mul(fgMask);
        img.convertTo(img, CV_8UC3);
        
        rectangle(img, rect, Scalar(0,0,255));
        
        imshow("Viewer", img);
        waitKey();
    } else {
        imshow("Viewer", img);
        waitKey();
    }
}

// Mouse event handlers, call after image is loaded and window is made
static void mouseEvent(int event, int x, int y, int, void*) {
    switch (event) {
        case CV_EVENT_LBUTTONDOWN:
            mouseX1 = x;
            mouseY1 = y;
            // printf("DOWN %d, %d\n", mouseX1, mouseX2);
            drag = true;
            break;
        case CV_EVENT_LBUTTONUP:
            mouseX2 = x;
            mouseY2 = y;
            // printf("UP %d, %d\n", mouseY1, mouseY2);
            drag = false;
            interactiveGrabCut(); // run grabcut
        case CV_EVENT_MOUSEMOVE:
            if (drag) {
                imgWorkingCopy = img.clone();
                rectangle(imgWorkingCopy, Point(mouseX1, mouseY1), Point(x, y), Scalar(0,255,0));
                imshow("Viewer", imgWorkingCopy);
            }
        default:
            break;
    }
}

int main(int argc, const char * argv[])
{
    
    img = imread("/Users/frank/dev/COMP4550/coffee.jpg");
    
    if (img.data == 0) {
        cerr << "Image not found!" << endl;
        return -1;
    }
    
    // Prepare window
    namedWindow("Viewer", CV_WINDOW_AUTOSIZE);
    imshow("Viewer", img);
    setMouseCallback("Viewer", mouseEvent);
    
    waitKey();
    
    return 0;
}

