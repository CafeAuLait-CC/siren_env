//
//  GTImage.cpp
//  Siren_Environment
//
//  Created by Alex Xu on 3/20/19.
//  Copyright © 2019 Alex Xu. All rights reserved.
//

#include "GTImage.hpp"

GTImage::GTImage() {
    this->cellImage = cv::Mat::zeros(cv::Size(19, 14), CV_8UC3);    // TODO: to be changed to imread()
    generateGTPattern();
}

GTImage::GTImage(cv::Size cellSize, cv::Size pixelSize) {
    this->cellImage = cv::Mat::zeros(cellSize, CV_8UC3);    // TODO: to be changed to imread()
    generateGTPattern();
}

void GTImage::generateGTPattern() {
    // TODO: from ground truth to pattern
    
}
