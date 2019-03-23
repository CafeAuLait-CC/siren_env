//
//  RGBImage.cpp
//  Siren_Environment
//
//  Created by Alex on 3/20/19.
//  Copyright © 2019 Pinjing [Alex] Xu. All rights reserved.
//

#include "RGBImage.hpp"

RGBImage::RGBImage() {
    std::cout << "No input file!" << std::endl;
    exit(-1);
}

RGBImage::RGBImage(std::string fileName, cv::Size dstSize) {
    this->rgbImage = cv::imread(fileName);
    cv::resize(this->rgbImage, this->rgbImage, dstSize);    // TODO: need more process here
}
