//
//  RGBImage.cpp
//  Siren_Environment
//
//  Created by Alex on 3/20/19.
//  Copyright © 2019 Pinjing [Alex] Xu. All rights reserved.
//

#include "RGBImage.hpp"

RGBImage::RGBImage() {
    std::cerr << "No input file!" << std::endl;
    exit(-1);
}

RGBImage::RGBImage(std::string fileName, cv::Size dstSize) {
    this->imagery = cv::imread(fileName);
    if (!this->imagery.data) {
        std::cerr << "Failed to load Imagery patch: " << fileName << std::endl;
        exit(-1);
    }
}

cv::Mat RGBImage::getImagery() {
    return this->imagery;
}
