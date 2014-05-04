//
//  transformation.cpp
//  active3d
//
//  Performs reconstruction of an image matrix into
//  a 3D view.
//
//  Created by Frank Zhao on 3/05/2014.
//  Copyright (c) 2014 Frank Zhao. All rights reserved.
//  frank.zhao@anu.edu.au
//

#include "transformation.h"
#include "utility.h"
#include <opencv2/opencv.hpp>
#include <stdlib.h>
#include <math.h>

#define PI 3.14159265

using namespace cv;
using namespace std;

// Define transformation matrices
// {-x, y, z} z is positive towards us
const float height = -5.0;
const float dist = -10.0;
const float scaleFactor = 0.7;
const Vec3f translationVector = {0, height, dist};

// Scales a vector about the centre of the image
// scale(vector, sfactor, img width, img height, img depth)
Vec3f scale(Vec3f point, float scaleFactor, int x, int y, int z) {
    // Translate to top left
    Vec3f tvec = {x/2.0f, y/2.0f, -z/2.0f};
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

// Converts image pixel into a 3D point
// Needs image width, height and depth
Vec3f reconstruct3D(Vec3f point, int width, int height) {
    
    // Scale and Translate
    point = scale(point, scaleFactor, width, height, point[2]) + translationVector;
    
    // Convert point vector to a 3x1 matrix
    Mat vec = Mat(3, 1, CV_32FC1, &point);
    
    // rotate
    Mat rotationMatrix = Mat(3, 3, CV_32FC1, Scalar(0.0));
    constructInverseRotationMatrix(PI/5, rotationMatrix);
    vec = rotationMatrix * vec;
    
    //printMatrix(rotationMatrix);
    
    // Convert back to a Vec3f
    point[0] = vec.at<float>(0,0);
    point[1] = vec.at<float>(1,0);
    point[2] = vec.at<float>(2,0);
    
    return point;
}