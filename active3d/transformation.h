//
//  transformation.h
//  active3d
//
//  Created by Frank Zhao on 3/05/2014.
//  Copyright (c) 2014 Frank Zhao. All rights reserved.
//  frank.zhao@anu.edu.au
//

#ifndef __active3d__transformation__
#define __active3d__transformation__

#include <iostream>

#endif /* defined(__active3d__transformation__) */

#include <opencv2/opencv.hpp>
using namespace cv;

// Transform a pixel vector into 3D space
Vec3f reconstruct3D(Vec3f pixel);