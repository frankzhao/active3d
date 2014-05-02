//
//  utility.h
//  ImageSegmentation
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
Mat depthMap(Mat mask, Mat dest, const string &winname, int iterations, bool render);

// Class to store a 3D coordinate
class Coord {
public:
    Coord() {}
public:
    float x, y, z;
    
public:
    Coord(float x_c, float y_c, float z_c) {
        x = x_c;
        y = y_c;
        z = z_c;
    }
};

// Rotate about 0,0
Coord rotate(float x, float y, float z, float r_x, float r_y, float r_z);

// Rotate about center of image
Coord AffineRotate(float x, float y, float z, float r_x, float r_y, float r_z, int width, int height);

// depthMap(fgMask, drawDest, iterations, colorView)
void depthMap(Mat mask, Mat dest, int iterations, const string &winname, bool color);

#endif
