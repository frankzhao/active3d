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

Mat img, imgWorkingCopy;
Mat mask, fgMask, fgModel, bgModel;
Rect rect;

// Mouse event handler for mask painting
int brushRadius = 10;
static void mousePaintEvent(int event, int x, int y, int, void*) {
    switch (event) {
        case CV_EVENT_LBUTTONDOWN:
            mouseX1 = x;
            mouseY1 = y;
            drag = !drag;
            circle(imgWorkingCopy, Point (x,y), 3, Scalar(100,100,100));
            circle(mask, Point (x,y), 3, Scalar(0));
            break;
        case CV_EVENT_LBUTTONUP:
            mouseX2 = x;
            mouseY2 = y;
            drag = !drag;
        case CV_EVENT_MOUSEMOVE:
            if (drag) {
                circle(imgWorkingCopy, Point(x,y), brushRadius, Scalar(150,150,150), -brushRadius);
                circle(fgMask, Point(x,y), 10, Scalar(0), -brushRadius);
                imshow("Viewer", imgWorkingCopy);
            }
        default:
            break;
    }
}

static void interactiveGrabCut() {
    int rows = img.rows;
    int cols = img.cols;
    
    //printf("%d, %d\n", rows, cols);
    
    // initialise algorithm arrays with zeros
    mask = Mat(rows, cols, CV_32FC1, double(0));
    fgModel = Mat(1, 65, CV_64FC1, double(0));
    bgModel = Mat(1, 65, CV_64FC1, double(0));
    
    // check that the area of the selected rectangle is > 0
    if ( abs((mouseX1 - mouseX2) * (mouseY2 - mouseY1)) > 0 ) {
        
        // TODO error when rectangle is the same size as image
        rect = Rect( Point(mouseX1, mouseY1), Point (mouseX2, mouseY2) );
        
        
        // GC_INIT_WITH_RECT -> using rectangle
        // GC_INIT_WITH_MASK -> using mask
        grabCut(img, mask, rect, bgModel, fgModel, 5, GC_INIT_WITH_RECT);
        
        // 0, 2 -> bg pixels; 1, 3 -> fg pixels
        fgMask = mask.clone();
        
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
        
        cvtColor(fgMask, fgMask, CV_GRAY2BGR); // convert from 8-bit single channel mask to 3 channel
        fgMask.convertTo(fgMask, CV_32FC3);
        img = img.mul(fgMask);
        img.convertTo(img, CV_8UC3);

        imgWorkingCopy = img.clone();
        rectangle(imgWorkingCopy, rect, Scalar(0,0,255));
        
        imshow("Viewer", imgWorkingCopy);
        setMouseCallback("Viewer", mousePaintEvent);
    }
}

// Mouse event handler for initial rectangle, call after image is loaded and window is made
static void mouseRectangleEvent(int event, int x, int y, int, void*) {
    switch (event) {
        case CV_EVENT_LBUTTONDOWN:
            mouseX1 = x;
            mouseY1 = y;
            // printf("DOWN %d, %d\n", mouseX1, mouseX2);
            drag = !drag;
            break;
        case CV_EVENT_LBUTTONUP:
            mouseX2 = x;
            mouseY2 = y;
            // printf("UP %d, %d\n", mouseY1, mouseY2);
            drag = !drag;
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
    
    img = imread("/Users/frank/dev/COMP4550/dog.jpg");
    
    if (img.data == 0) {
        cerr << "Image not found!" << endl;
        return -1;
    }
    
    // Prepare window
    namedWindow("Viewer", CV_WINDOW_AUTOSIZE);
    imshow("Viewer", img);
    setMouseCallback("Viewer", mouseRectangleEvent);
    
    int k=0;
    while (1) {
        k = waitKey();
        if (k == 13) { // wait for enter key
            cvtColor(fgMask, fgMask, CV_BGR2GRAY);
            fgMask.convertTo(fgMask, CV_8UC1);
            grabCut(img, fgMask, rect, bgModel, fgModel, 15, GC_INIT_WITH_MASK); // grabcut with new mask
            cvtColor(fgMask, fgMask, CV_GRAY2BGR);
            fgMask.convertTo(fgMask, CV_32FC3); // multiplication requires 3-channel float
            img.convertTo(img, CV_32FC3); // gemm needs float matrix
            img = img.mul(fgMask);
            img.convertTo(img, CV_8UC3);
            break;
        }
    }
    
    imshow("Viewer", img);
    
    waitKey(); // press any key to exit
    
    img.release();
    imgWorkingCopy.release();
    mask.release();
    fgModel.release();
    bgModel.release();
    
    return 0;
}

