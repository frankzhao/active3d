//
//  transformation.h
//  active3d
//
//  Created by Frank Zhao on 3/05/2014.
//  Copyright (c) 2014 Frank Zhao. All rights reserved.
//  <frank.zhao@anu.edu.au>
//

#ifndef __active3d__transformation__
#define __active3d__transformation__

#include <iostream>

#endif /* defined(__active3d__transformation__) */

#include <opencv2/opencv.hpp>
using namespace cv;

// Converts image pixel into a 3D point
// Needs image width, height and depth
Vec3f reconstruct3D(Vec3f point, int width, int height, int eye);

// Converts point into GL_POINT
void drawPoint(Vec3b point, Mat depthMask, Mat fgMask, int eye)