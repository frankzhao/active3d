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

using namespace cv;
using namespace std;

// Define transformation matrices
// {-x, y, z} z is positive towards us
const float height = 0.0;
const float dist = 0.0;
const Vec3f translationVector = {0, height, dist};

Mat3f constructRotationMatrix(float angle) {
    float rotvalues[3][3] = {
        {1.0,0.0,0.0},
        {0.0, cos(angle), -sin(angle)},
        {0.0, sin(angle), cos(angle)}
    };
    
    Mat3f rotMatrix = Mat(3, 3, CV_32FC2, &rotvalues);
    
    return rotMatrix;
}

// Converts image pixel into a 3D point
Vec3f reconstruct3D(Vec3f point) {
    
    // Translate
    point = point - translationVector;
    
    // Convert point to a 3x1 matrix
    Mat3f vec = Mat(3, 1, CV_32FC2, 0.0);
    vec.at<float>(0,0) = point[0];
    vec.at<float>(1,0) = point[1];
    vec.at<float>(2,0) = point[2];
    
    
    
    // rotate
    Mat3f rotationMatrix = constructRotationMatrix(-10.0);
    //printMatrix(vec);
    cout << "----" << endl;
    //printMatrix(rotationMatrix);
    //vec = rotationMatrix * vec;
    
    // Convert back to a Vec3f
    point[0] = vec.at<float>(0,0);
    point[1] = vec.at<float>(1,0);
    point[2] = vec.at<float>(2,0);
    
    return point;
}