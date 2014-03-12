//
//  main.cpp
//  ImageSegmentation
//
//  Created by Frank Zhao on 23/02/2014.
//  Copyright (c) 2014 Frank Zhao. All rights reserved.
//

#define RECT_MASK 0
#define REFINE_MASK 1

#define MODE_RECTMODE 0
#define MODE_PAINTMODE 1

#include <iostream>
#include <opencv2/opencv.hpp>
#include <stdlib.h>

using namespace cv;
using namespace std;

int key; // keyboard input
int mode = -1; //current mode we are in
int mouseX1 = 0, mouseY1 = 0, mouseX2 = 0, mouseY2 = 0;
bool drag = false, drawing = true, paintFG = false;

Mat img, imgWorkingCopy, viewport;
Mat mask, refineMask, fgMask, fgModel, bgModel;
Rect rect;

//void grabCut(InputArray img, InputOutputArray mask, Rect rect, InputOutputArray bgdModel, InputOutputArray fgdModel, int iterCount, int mode=GC_EVAL )

static int interactiveGrabCut(int grabCutMode) {
    int rows = img.rows;
    int cols = img.cols;
    
    if (grabCutMode == RECT_MASK) {
    
        // initialise algorithm arrays with zeros
        mask = Mat(rows, cols, CV_8UC1, double(0));
        fgModel = Mat(1, 65, CV_64FC1, double(0));
        bgModel = Mat(1, 65, CV_64FC1, double(0));
        
        // check that the area of the selected rectangle is > 0 and less than total image area
        int rectArea = abs((mouseX1 - mouseX2) * (mouseY2 - mouseY1)) > 0;
        assert(rectArea > 0 && rectArea < (img.cols * img.rows) ); // check that the rectangle is valid
            
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
        
        cvtColor(fgMask, fgMask, CV_GRAY2BGR); // convert from 8-bit single channel mask to 3 channel
        fgMask.convertTo(fgMask, CV_32FC3);
        imgWorkingCopy.convertTo(imgWorkingCopy, CV_32FC3); // gemm needs float matrix
        imgWorkingCopy = imgWorkingCopy.mul(fgMask);
        imgWorkingCopy.convertTo(imgWorkingCopy, CV_8UC3);
        
        viewport = imgWorkingCopy.clone();
        rectangle(viewport, rect, Scalar(0,0,255));
        
        imshow("Viewer", viewport);
        
        // convert mask back to single channel for next iteration
        cvtColor(fgMask, fgMask, CV_BGR2GRAY);
        fgMask.convertTo(fgMask, CV_8UC1);
        
        // initalise mat for refine mask as all white
        refineMask = Mat(rows, cols, CV_8UC1, 255);
        
        return 0;
        
    } else if (grabCutMode == REFINE_MASK) {
        
        
        // apply refined mask to first mask
        for (int i=0; i<img.rows; i++) {
            for (int j=0; j<img.cols; j++) {
                int v = refineMask.at<uchar>(i,j);
                if (v == GC_BGD) {
                    mask.at<uchar>(i,j) = GC_BGD;
                }
            }
        }
        
        imgWorkingCopy = img.clone();
        
        grabCut(img, mask, Rect(), bgModel, fgModel, 5, GC_INIT_WITH_MASK); // grabcut with new mask
        
        for (int i=0; i<img.rows; i++) {
            for (int j=0; j<img.cols; j++) {
                int v = mask.at<uchar>(i,j);
                if (v == 0 || v == 2) {
                    mask.at<uchar>(i,j) = 0;
                } else if (v == 1 || v == 3) {
                    mask.at<uchar>(i,j) = 1;
                }
            }
        }
        
        cvtColor(mask, mask, CV_GRAY2BGR);
        imshow("Viewer", mask);
        
        mask.convertTo(mask, CV_32FC3); // multiplication requires 3-channel float
        imgWorkingCopy.convertTo(imgWorkingCopy, CV_32FC3); // gemm needs float matrix
        imgWorkingCopy = imgWorkingCopy.mul(mask);
        imgWorkingCopy.convertTo(imgWorkingCopy, CV_8UC3);
        
        return 0;
        
    } else {
        return 1;
    }
    
    
    return 0;
}

// Mouse event handler for mask painting
int brushRadius = 20;
static void mousePaintEvent(int event, int x, int y, int, void*) {
    
    switch (event) {

        case CV_EVENT_LBUTTONDOWN:
            drag = !drag;
            if (!paintFG) {
                circle(viewport, Point (x,y), brushRadius, Scalar(150,150,150), -1);
                circle(refineMask, Point (x,y), brushRadius, GC_BGD, -1);
            } else if (paintFG) {
                circle(viewport, Point (x,y), brushRadius, Scalar(255,255,255), -1);
                circle(refineMask, Point (x,y), brushRadius, GC_FGD, -1);
            }
            imshow("Viewer", viewport);
            break;
        case CV_EVENT_LBUTTONUP:
            drag = !drag;
            if (!drawing) {
                setMouseCallback("Viewer", NULL, NULL); // remove mouse callback
                interactiveGrabCut(REFINE_MASK); // run grabcut
            }
            imshow("Viewer", viewport);
            break;
        case CV_EVENT_MOUSEMOVE:
            if (drag) {
                if (!paintFG) {
                    circle(viewport, Point (x,y), brushRadius, Scalar(150,150,150), -1);
                    circle(refineMask, Point (x,y), brushRadius, GC_BGD, -1);
                } else if (paintFG) {
                    circle(viewport, Point (x,y), brushRadius, Scalar(255,255,255), -1);
                    circle(refineMask, Point (x,y), brushRadius, GC_FGD, -1);
                }
                imshow("Viewer", viewport);
            }
            break;
        default:
            break;
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
            setMouseCallback("Viewer", NULL, NULL); // remove mouse callback
            interactiveGrabCut(RECT_MASK); // run grabcut
            
            // Let the user define a refine mask in paint mode
            setMouseCallback("Viewer", mousePaintEvent);
            mode = MODE_PAINTMODE;
            break;
        case CV_EVENT_MOUSEMOVE:
            if (drag) {
                viewport = img.clone();
                rectangle(viewport, Point(mouseX1, mouseY1), Point(x, y), Scalar(0,255,0));
                imshow("Viewer", viewport);
            }
            break;
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
    waitKey(200);
    
    // Let the user define a rectangle
    setMouseCallback("Viewer", mouseRectangleEvent);
    mode = MODE_RECTMODE;
        
    //esc to exit
    while (1) {
        key = waitKey();
        
        if (key == 27) {
            break;
        } else if (key == 17 && mode == MODE_PAINTMODE) {
            paintFG = !paintFG;
        }
    }
    
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

