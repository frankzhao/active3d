//
//  transformation.cpp
//  active3d
//
//  Performs reconstruction of an image matrix into
//  a 3D view.
//
//  Created by Frank Zhao on 3/05/2014.
//  Copyright (c) 2014 Frank Zhao. All rights reserved.
//  <frank.zhao@anu.edu.au>
//

#include "transformation.h"
#include "utility.h"
#include <opencv2/opencv.hpp>
#include <stdlib.h>
#include <math.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>
#endif

#define PI 3.14159265

using namespace cv;
using namespace std;

// Define transformation matrices
// {-x, y, z} z is positive towards us
const float camera_height = -5.0;
const float dist = -10.0;
const float scaleFactor = 0.7;

// Scales a vector about the centre of the image
// scale(vector, sfactor, img width, img height, img depth)
Vec3f scale(Vec3f point, float scaleFactor, int x, int y, int z) {
    // Translate to top left
    Vec3f tvec;
    tvec[0] = x/2.0f; tvec[1] = y/2.0f; tvec[2] = -z/2.0f;
    point = point - tvec;
    
    point = scaleFactor * point;
    
    // Translate back
    point = point + tvec;
    
    return point;
}

void constructRotationMatrix(float angle, Mat dest) {
    float rotvalues[3][3] = {
        {1.0, 0.0       ,  0.0       },
        {0.0, cos(angle), -sin(angle)},
        {0.0, sin(angle),  cos(angle)}
    };

    Mat rotMatrix = Mat(3, 3, CV_32FC1, rotvalues);
    rotMatrix.copyTo(dest);
}

void constructInverseRotationMatrix(float angle, Mat dest) {
    float rotvalues[3][3] = {
        {1.0,  0.0       ,  0.0      },
        {0.0,  cos(angle), sin(angle)},
        {0.0, -sin(angle), cos(angle)}
    };
    
    Mat rotMatrix = Mat(3, 3, CV_32FC1, rotvalues);
    rotMatrix.copyTo(dest);
}

/*
 * Two element vector array for stereo pair
 * IPD is the distance between pupils
 */

// Converts image pixel into a 3D point
// Needs image width, height and depth
Vec3f reconstruct3D(Vec3f point, int width, int height, int eye) {
    
    Vec3f translationVector;
    translationVector[0] = 0;
    translationVector[1] = camera_height;
    translationVector[2] = dist;
    float ipd = 200.0; // pupil distance
    
    // Scale and Translate
    point = scale(point, scaleFactor, width, height, point[2]) + translationVector;
    
    // Convert point vector to a 3x1 matrix
    Mat vec = Mat(3, 1, CV_32FC1, &point);
    
    // rotate
    Mat rotationMatrix = Mat(3, 3, CV_32FC1, Scalar(0.0));
    constructInverseRotationMatrix(10, rotationMatrix);  // TODO calculate beta
    vec = rotationMatrix * vec;
    
    // generate stereo pair
    Mat view;
    if (eye == 0) {
        Vec3f stereoLeftVector;
        stereoLeftVector[0] = -ipd/2; stereoLeftVector[1] = 0, stereoLeftVector[2] = 0;
        Mat stereoLeftTranslation  = Mat(3, 1, CV_32FC1, &stereoLeftVector);
        view  = vec + stereoLeftTranslation;
    } else if (eye == 1) {
        Vec3f stereoRightVector;
        stereoRightVector[0] = ipd/2; stereoRightVector[1] = 0, stereoRightVector[2] = 0;
        Mat stereoRightTranslation = Mat(3, 1, CV_32FC1, &stereoRightVector);
        view = vec + stereoRightTranslation;
    }
    
    // Inverse rotate
    constructRotationMatrix(10, rotationMatrix);
    view = rotationMatrix * view;
    point[0] = view.at<float>(0,0);
    point[1] = view.at<float>(1,0);
    point[2] = view.at<float>(2,0);
    
    // Translate and scale back
    point = scale(point, 1+scaleFactor, width, height, point[2]) - translationVector;
    
    return point;
}

// Converts point into GL_POINT
void drawPoint(Vec3b point, int x, int y, Mat depthMask, Mat fgMask, int eye) {
    float depth; // normalised depth
    //float cameraHeight = 1.6;
    int rows = fgMask.rows;
    int cols = fgMask.cols;
    
    glColor3f(point[2]/255.0, point[1]/255.0, point[0]/255.0);
    
    // init glVertex, x -> horiz, y -> height, z -> depth
    depth = depthMask.at<float>(x, y);
    
    // 3D reconstruction
    // three points make a triangle
    Vec3f vertex;
    vertex[0] = (float) y;
    vertex[1] = (float) x;
    vertex[2] = (float) depth;
    
    // reconstruct 3D using transformation.cpp method
    point = reconstruct3D(vertex, cols, rows, eye);
    
    // OpenGl stores pixels upside down to OpenCV
    glVertex3f( (GLfloat) vertex[0], (GLfloat) rows - vertex[1], (GLfloat) vertex[2] );
}