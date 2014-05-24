//
//  utility.h
//  ImageSegmentation
//  Utility methods for active3d
//
//  Created by Frank Zhao on 25/03/2014.
//  Copyright (c) 2014 Frank Zhao. All rights reserved.
//  <frank.zhao@anu.edu.au>
//

#ifndef ImageSegmentation_utility_h
#define ImageSegmentation_utility_h

#include <opencv2/opencv.hpp>

using namespace cv;

int overlayImage(Mat src, Mat overlay, Mat dest);

// apply a mask with unmasked areas in alpha=0
int applyMask(Mat src, Mat mask, Mat dest);

// Count how many of the 8 neighbouring pixels
// are of a particular value (8-bit, 1-channel)
int countNeighbours(Mat m, int v, int row, int col);

// Generate depth map with specified iterations
float depthMap(Mat mask, Mat dest, const string &winname, int iterations);

// Print float matricies nicely
void printMatrix(Mat m);

#endif
