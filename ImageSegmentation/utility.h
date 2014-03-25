//
//  utility.h
//  ImageSegmentation
//
//  Created by Frank Zhao on 25/03/2014.
//  Copyright (c) 2014 Frank Zhao. All rights reserved.
//

#ifndef ImageSegmentation_utility_h
#define ImageSegmentation_utility_h

#include <opencv2/opencv.hpp>
using namespace cv;

int overlayImage(Mat src, Mat overlay, Mat dest);

// apply a mask with unmasked areas in alpha=0
int applyMask(Mat src, Mat mask, Mat dest);

#endif
