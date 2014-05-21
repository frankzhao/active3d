//
//  utility.cpp
//  ImageSegmentation
//
//  Created by Frank Zhao on 25/03/2014.
//  Copyright (c) 2014 Frank Zhao. All rights reserved.
//  <frank.zhao@anu.edu.au>
//

#include "utility.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <stdlib.h>
#include <math.h>

using namespace cv;
using namespace std;

/*********************
 *  Utility methods  *
 *********************/

//TODO add option to specify placement location and blending coefficients
int overlayImage(Mat src, Mat overlay, Mat dest) {
    
    // ensure the source and overlay/dest have the same size
    assert ( (src.rows * src.cols) == (dest.rows * dest.cols) );
    assert ( (src.rows * src.cols) == (overlay.rows * overlay.cols) );
    
    for (int i=0; i<src.rows; i++) {
        for (int j=0; j<src.cols; j++) {
            dest.at<uchar>(i,j) = src.at<uchar>(i,j);
            dest.at<uchar>(i,j) = overlay.at<uchar>(i,j);
        }
    }
    
    return 0;
}

// apply a mask with unmasked areas in alpha=0
int applyMask(Mat src, Mat mask, Mat dest) {
    
    assert(src.size > 0);
    assert(mask.size > 0);
    assert(dest.size > 0);
    assert( (src.size == mask.size) && (mask.size == dest.size) );
    
    for (int i=0; i<src.rows; i++) {
        for (int j=0; j<src.cols; j++) {
            Vec4b& destv = dest.at<Vec4b>(i,j);
            Vec4b& srcv  = src.at<Vec4b>(i,j);
            Vec4b& maskv  = mask.at<Vec4b>(i,j);
            
            destv[0] = srcv[0] * maskv[0];
            destv[1] = srcv[1] * maskv[1];
            destv[2] = srcv[2] * maskv[2];
            
            if ( (destv[0] + destv[1] + destv[2]) == 0) {
                destv[3] = 0;
            } else {
                destv[3] = 1;
            }
            
            
        }
    }
    
    return 0;
}

// Count how many of the 8 neighbouring pixels
// are of a particular value (8-bit, 1-channel)
int countNeighbours(Mat m, int v, int row, int col) {
    
    assert(m.size > 0);
    
    
    int count = 0;
            
    // make sure indicies are within bounds
    if(row<m.rows - 1 && col>0 && m.cols-1 && col>0) {
        // Check whether neighbouring pixels are v
        if (m.at<uchar>(row+1, col-1) == v)
            count++;
        if (m.at<uchar>(row+1, col) == v)
            count++;
        if (m.at<uchar>(row+1, col+1) == v)
            count++;
        if (m.at<uchar>(row, col-1) == v)
            count++;
        if (m.at<uchar>(row, col+1) == v)
            count++;
        if (m.at<uchar>(row-1, col-1) == v)
            count++;
        if (m.at<uchar>(row-1, col) == v)
            count++;
        if (m.at<uchar>(row-1, col+1) == v)
            count++;
    }
    return count;
}

// Generate depth map with specified iterations
Mat depthMap(Mat mask, Mat dest, const string &winname, int iterations, bool render) {
    
    Mat depthMask = mask.clone();
    Mat prevMask = Mat(mask.rows, mask.cols, CV_8UC1, double(0)); // mask from previous iteration
    int rows = depthMask.rows, cols = depthMask.cols;
    
    //apply depth contour
    int count = 0;
    //for (int depth=2; depth<iterations; depth++) {
        prevMask = depthMask.clone();
    float depth;
        for (int i=0; i<rows; i++) {
            for (int j=0; j<cols; j++) {
                Vec3b& destv = dest.at<Vec3b>(i,j);
                
                depth = floor(sqrt((depthMask.cols/2 - j)^2 + (depthMask.rows/2 - i)^2));
                cout << floor(sqrt((depthMask.cols/2 - j)^2 + (depthMask.rows/2 - i)^2)) << endl;
//                if (render) {
//                    destv = Vec3b(0,0, depth * ((int) (255 / depth)) );
//                }
                depthMask.at<uchar>(i,j) = depth;
                
//                // count neighbouring pixels
//                if (prevMask.at<uchar>(i,j) == depth-1) {
//                    count = countNeighbours(prevMask, depth-1, i, j);
//
//                    //printf("%d ", count);
//                }
//                
//                // write depth
//                if (count == 8) {
//                    if (render) {
//                        destv = Vec3b(0,0, depth * ((int) (255 / iterations)) );
//                    }
//                    depthMask.at<uchar>(i,j) = depth/rows;
//                }

            }
        }
  //  }
    prevMask.release();
    
//    if (render) {
//        imshow(winname, dest);
//    }
    
    return depthMask;
}

// Print float matricies nicely
void printMatrix(Mat m) {
    int i, j;
    for (i=0; i<m.rows; i++) {
        for (j=0; j<m.cols; j++) {
            printf("%f ", m.at<float>(i,j));
        }
        cout << endl;
    }
    cout << endl;
}