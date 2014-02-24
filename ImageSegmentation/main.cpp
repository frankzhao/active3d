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

Mat img, imgWorkingCopy, viewport;
Mat mask, refineMask, fgMask, fgModel, bgModel;
Rect rect;

// Mouse event handler for mask painting
int brushRadius = 10;
static void mousePaintEvent(int event, int x, int y, int, void*) {
    switch (event) {
        case CV_EVENT_LBUTTONDOWN:
            mouseX1 = x;
            mouseY1 = y;
            drag = !drag;
            circle(viewport, Point (x,y), brushRadius, Scalar(150,150,150), -brushRadius);
            circle(refineMask, Point (x,y), brushRadius, GC_BGD, -brushRadius);
            break;
        case CV_EVENT_LBUTTONUP:
            mouseX2 = x;
            mouseY2 = y;
            drag = !drag;
        case CV_EVENT_MOUSEMOVE:
            if (drag) {
                circle(viewport, Point(x,y), brushRadius, Scalar(150,150,150), -brushRadius);
                circle(refineMask, Point(x,y), brushRadius, GC_BGD, -brushRadius);
                imshow("Viewer", viewport);

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
    mask = Mat(rows, cols, CV_8UC1, double(0));
    fgModel = Mat(1, 65, CV_64FC1, double(0));
    bgModel = Mat(1, 65, CV_64FC1, double(0));
    
    // check that the area of the selected rectangle is > 0 and less than total image area
    int rectArea = abs((mouseX1 - mouseX2) * (mouseY2 - mouseY1)) > 0;
    if ( rectArea > 0 && rectArea < (img.cols * img.rows) ) {
        
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
        
        imgWorkingCopy = img.clone();
        imgWorkingCopy.convertTo(imgWorkingCopy, CV_32FC3); // gemm needs float matrix
        
        cvtColor(fgMask, fgMask, CV_GRAY2BGR); // convert from 8-bit single channel mask to 3 channel
        fgMask.convertTo(fgMask, CV_32FC3);
        imgWorkingCopy = imgWorkingCopy.mul(fgMask);
        imgWorkingCopy.convertTo(imgWorkingCopy, CV_8UC3);

        viewport = imgWorkingCopy.clone();
        rectangle(viewport, rect, Scalar(0,0,255));
        
        imshow("Viewer", viewport);
        
        // convert mask back to single channel for next iteration
        cvtColor(fgMask, fgMask, CV_BGR2GRAY);
        fgMask.convertTo(fgMask, CV_8UC1);
        
        // initalise mat for refine mask as all FG
        refineMask = Mat(rows, cols, CV_8UC1, GC_FGD);
        
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
                viewport = img.clone();
                rectangle(viewport, Point(mouseX1, mouseY1), Point(x, y), Scalar(0,255,0));
                imshow("Viewer", viewport);
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
            
//            refineMask.convertTo(refineMask, CV_8UC1, 255);
//            imshow("Viewer", refineMask);
//            
//            waitKey();
            
            // apply refined mask to first mask
            for (int i=0; i<img.rows; i++) {
                for (int j=0; j<img.cols; j++) {
                    int v = refineMask.at<uchar>(i,j);
                    if (v == GC_BGD) {
                        fgMask.at<uchar>(i,j) = GC_BGD;
                    }
                }
            }
            
            imgWorkingCopy = img.clone();
            
            grabCut(img, fgMask, Rect(), bgModel, fgModel, 5, GC_INIT_WITH_MASK); // grabcut with new mask
            
            cvtColor(fgMask, fgMask, CV_GRAY2BGR);
            
//            fgMask.convertTo(fgMask, CV_8UC3, 255);
//            imshow("Viewer", fgMask);
//            waitKey();
            
            fgMask.convertTo(fgMask, CV_32FC3); // multiplication requires 3-channel float
            imgWorkingCopy.convertTo(imgWorkingCopy, CV_32FC3); // gemm needs float matrix
            imgWorkingCopy = imgWorkingCopy.mul(fgMask);
            imgWorkingCopy.convertTo(imgWorkingCopy, CV_8UC3);
            break;
        }
    }
    
    imshow("Viewer", imgWorkingCopy);
    
    waitKey(); // press any key to exit
    
    img.release();
    imgWorkingCopy.release();
    mask.release();
    fgMask.release();
    fgModel.release();
    bgModel.release();
    viewport.release();
    refineMask.release();
    
    return 0;
}

