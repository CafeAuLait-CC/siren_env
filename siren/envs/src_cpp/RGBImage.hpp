//
//  RGBImage.hpp
//  Siren_Environment
//
//  Created by Alex on 3/20/19.
//  Copyright © 2019 Pinjing [Alex] Xu. All rights reserved.
//

#ifndef RGBImage_hpp
#define RGBImage_hpp

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
//#include <opencv2/imgproc.hpp>

class RGBImage {
public:
    RGBImage();
    RGBImage(std::string fileName, cv::Size patchSize = cv::Size(256, 256));
    cv::Mat getImagery();
    
private:
    cv::Mat imagery;
};

#endif /* RGBImage_hpp */
