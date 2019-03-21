//
//  RGBImage.hpp
//  Siren_Environment
//
//  Created by Alex Xu on 3/20/19.
//  Copyright © 2019 Alex Xu. All rights reserved.
//

#ifndef RGBImage_hpp
#define RGBImage_hpp

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

class RGBImage {
public:
    RGBImage();
    RGBImage(std::string fileName, cv::Size dstSize);
    
    cv::Mat rgbImage;
};

#endif /* RGBImage_hpp */
