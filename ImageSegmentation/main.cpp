//
//  main.cpp
//  ImageSegmentation
//
//  Created by Frank Zhao on 23/02/2014.
//  Copyright (c) 2014 Frank Zhao. All rights reserved.
//  <frank.zhao@anu.edu.au>
//

#ifdef WIN32
#include <windows.h>
#endif

#include <stdlib.h>
#include <iostream>
#include <fstream>

#include <opencv2/opencv.hpp>

#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>
#endif

#include "utility.h"

#define RECT_MASK 0
#define REFINE_MASK 1

#define MODE_IDLE 0
#define MODE_RECTMODE 1
#define MODE_PAINTMODE 2


using namespace cv;
using namespace std;

string windowName = "Viewer";
int key; // keyboard input
int mode = 0; //current mode we are in
int mouseX1 = 0, mouseY1 = 0, mouseX2 = 0, mouseY2 = 0;
bool drag = false, drawing = true, paintFG = false;

Mat img, imgWorkingCopy, viewport;
Mat mask, refineMask, fgMask, fgModel, bgModel;
cv::Rect rect;


/*********************
 *      Grabcut      *
 *********************/

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
            
        rect = cv::Rect( cv::Point(mouseX1, mouseY1), cv::Point (mouseX2, mouseY2) );
        
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
        
        imshow(windowName, viewport);
        
        // convert mask back to single channel for next iteration
        cvtColor(fgMask, fgMask, CV_BGR2GRAY);
        fgMask.convertTo(fgMask, CV_8UC1);
        
        // initalise mat for refine mask as all white
        refineMask = Mat(rows, cols, CV_8UC1, 255);
        
        fgModel.release();
        bgModel.release();
        
        return 0;
        
    } else if (grabCutMode == REFINE_MASK) {
        
        
        // apply refined mask to first mask
        for (int i=0; i<img.rows; i++) {
            for (int j=0; j<img.cols; j++) {
                int v = refineMask.at<uchar>(i,j);
                if (v == GC_BGD) {
                    mask.at<uchar>(i,j) = GC_BGD;
                } else if (v == GC_FGD) {
                    mask.at<uchar>(i,j) = GC_FGD;
                }
            }
        }
        
        imgWorkingCopy = img.clone();
        
        grabCut(img, mask, cv::Rect(), bgModel, fgModel, 5, GC_INIT_WITH_MASK); // grabcut with new mask
        
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
        
        mask.convertTo(mask, CV_32FC3); // multiplication requires 3-channel float
        imgWorkingCopy.convertTo(imgWorkingCopy, CV_32FC3); // gemm needs float matrix
        imgWorkingCopy = imgWorkingCopy.mul(mask);
        imgWorkingCopy.convertTo(imgWorkingCopy, CV_8UC3);
        mask.convertTo(mask, CV_8UC1);
        
        imshow(windowName, imgWorkingCopy);
        
        fgModel.release();
        bgModel.release();
        
        return 0;
        
    } else {
        return 1;
    }
    
    
    return 0;
}

/*********************
 *  Event handling   *
 *********************/

// Mouse event handler for mask painting
int brushRadius = 15;
static void mousePaintEvent(int event, int x, int y, int, void*) {
    
    switch (event) {
            
        case CV_EVENT_LBUTTONDOWN:
            drag = !drag;
            if (!paintFG) {
                circle(viewport, cv::Point (x,y), brushRadius, Scalar(150,150,150), -1);
                circle(refineMask, cv::Point (x,y), brushRadius, GC_BGD, -1);
            } else if (paintFG) {
                circle(viewport, cv::Point (x,y), brushRadius, Scalar(255,255,255), -1);
                circle(refineMask, cv::Point (x,y), brushRadius, GC_FGD, -1);
            }
            imshow(windowName, viewport);
            break;
        case CV_EVENT_LBUTTONUP:
            drag = !drag;
            imshow(windowName, viewport);
            break;
        case CV_EVENT_MOUSEMOVE:
            if (drag) {
                if (!paintFG) {
                    circle(viewport, cv::Point (x,y), brushRadius, Scalar(150,150,150), -1);
                    circle(refineMask, cv::Point (x,y), brushRadius, GC_BGD, -1);
                } else if (paintFG) {
                    circle(viewport, cv::Point (x,y), brushRadius, Scalar(255,255,255), -1);
                    circle(refineMask, cv::Point (x,y), brushRadius, GC_FGD, -1);
                }
                imshow(windowName, viewport);
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
            setMouseCallback(windowName, NULL, NULL); // remove mouse callback
            interactiveGrabCut(RECT_MASK); // run grabcut
            
            // Let the user define a refine mask in paint mode
            setMouseCallback(windowName, mousePaintEvent);
            mode = MODE_PAINTMODE;
            break;
        case CV_EVENT_MOUSEMOVE:
            if (drag) {
                viewport = img.clone();
                rectangle(viewport, cv::Point(mouseX1, mouseY1), cv::Point(x, y), Scalar(0,255,0));
                imshow(windowName, viewport);
            }
            break;
        default:
            break;
    }
}

/*********************
 *       Image       *
 *********************/

int initialize_image() {
    img = imread("/Users/frank/dev/COMP4550/coffee.jpg");
    
    if (img.data == 0) {
        return -1;
    }
    
    // Prepare window
    namedWindow(windowName, CV_WINDOW_AUTOSIZE);
    imshow(windowName, img);
    waitKey(200);
    
    // Let the user define a rectangle
    setMouseCallback(windowName, mouseRectangleEvent);
    mode = MODE_RECTMODE;
    return 0;
}

void release_memory() {
    img.release();
    imgWorkingCopy.release();
    mask.release();
    fgMask.release();
    fgModel.release();
    bgModel.release();
    viewport.release();
    refineMask.release();
}

/***************************
 *  Convert to point cloud *
 ***************************/
//
//void renderForeground() {
//    
//    //float cameraHeight = 1.6;
//    
//    // initialize the vector array
//    //GLfloat *points = (GLfloat*) calloc(fgMask.rows * fgMask.cols, sizeof(GLfloat));
//    //Vector3Df *points = (Vector3Df*) calloc(fgMask.rows * fgMask.cols, sizeof(float) * 3);
//    
//    int rows = fgMask.rows, cols = fgMask.cols;
//    
//    glBegin(GL_POINTS);
//    for (int i=0; i<rows; i++) {
//        for (int j=0; j<cols; j++) {
//            
//            // if the pixel is marked fg, set colour and add to array
//            if (fgMask.at<uchar>(i,j) == 1) {
//                // set colour
//                Vec3i pixel = imgWorkingCopy.at<Vec3i>(i,j);
//                glColor3i(pixel[2], pixel[1], pixel[0]);
//                
//                // init glVertex, x -> horiz, y -> height, z -> depth
//                glVertex3i(j, i, 10);
//            }
//        }
//    }
//    glEnd();
//}
//
//void setCamera() {
//    glTranslatef(0.0f,-0.75f, -2.0f);
//    glRotatef(0.0f, 0.0f, 1.0f, 0.0f);
//}
//
//void display() {
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//    
//    glBegin(GL_QUADS);
//    glVertex2f(-0.75f, -0.75f);
//    glVertex2f(0.75f, -0.75f);
//    glVertex2f(0.75f, 0.75f);
//    glVertex2f(-0.75f, 0.75f);
//    glEnd();
//    
//    //renderForeground();
//    
//    glutSwapBuffers();
//    
//}
//
//void reshape(int width, int height) {
//    glViewport(0, 0, width, height);
//}
//
//void idle() {
//    glutPostRedisplay();
//}
//
//void renderGL(int argc, char* argv[]) {
//    //glutInit(&argc, argv);
//    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
//    glutInitWindowSize(img.rows, img.cols);
//    
//    glutCreateWindow("GLUT");
//    glutDisplayFunc(display);
//    //glutReshapeFunc(reshape);
//    //glutIdleFunc(idle);
//    
//    //display();
//    
//    glutMainLoop();
//}

/*********************
 *       Main        *
 *********************/

int main(int argc, char * argv[]) {

    int retval = initialize_image();
    
    if (retval == -1) {
        cerr << "Image not found!" << endl;
        return -1;
    }
    
    //esc to exit
    while (1) {
        key = waitKey();
        //printf("%d\n", key);
        
        if (key == 27) {
            break;
        } else if (key == 109 && mode == MODE_PAINTMODE) {
            // m key to toggle mask
            paintFG = !paintFG;
        } else if (key == 13 && mode == MODE_PAINTMODE) {
            // enter key to run refine mask
            setMouseCallback(windowName, NULL, NULL); // remove mouse callback
            interactiveGrabCut(REFINE_MASK); // run grabcut
            mode = MODE_IDLE;
            depthMap(fgMask, imgWorkingCopy, 10, windowName, true);
            imshow(windowName, imgWorkingCopy);
            
            //renderGL(argc, argv);

            
        } else if (key == 114) { //restart if 'r' is pressed
            release_memory();
            initialize_image();
        }
    }
    
    //destroyAllWindows();
    release_memory();
    
    return 0;
}

