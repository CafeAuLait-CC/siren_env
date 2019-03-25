//
//  GTImage.cpp
//  Siren_Environment
//
//  Created by Alex on 3/20/19.
//  Copyright © 2019 Pinjing [Alex] Xu. All rights reserved.
//

#include "GTImage.hpp"

GTImage::GTImage() {
    std::cerr << "Nothing to initial!" << std::endl;;
    exit(-1);
}

GTImage::GTImage(std::string& fileName, cv::Size cellSize, cv::Size pixelSize) {
    this->pixelImage = cv::imread(fileName);
    if (!this->pixelImage.data) {
        std::cerr << "Could not open ground truth image: " + fileName << std::endl;
        exit(-1);
    }
    this->cellImage = cv::Mat::zeros(cv::Size(pixelSize.width/cellSize.width, pixelSize.height/cellSize.height), 0);
    for (int i  = cellSize.height / 2; i < pixelImage.rows; i += cellSize.height) {
        for (int j = cellSize.width / 2; j < pixelImage.cols; j += cellSize.width) {
            cv::Vec3b value = pixelImage.at<cv::Vec3b>(i, j);
            if (value == cv::Vec3b(0, 0, 0)) {
                cellImage.at<uchar>(i/cellSize.height, j/cellSize.width) = 0;
            } else if (value == cv::Vec3b(0, 255, 0)) {
                cellImage.at<uchar>(i/cellSize.height, j/cellSize.width) = 1;
            } else if (value == cv::Vec3b(255, 0, 0)) {
                cellImage.at<uchar>(i/cellSize.height, j/cellSize.width) = 2;
            } else {
                // TODO: needs to be refined
                // std::cerr << "Ground truth image pixel value wrong: " << i << ", " << j << " " << value << std::endl;;
                // exit(-1);
                cellImage.at<uchar>(i/cellSize.height, j/cellSize.width) = 2;
            }
            
        }
    }
}

cv::Mat GTImage::getPattern() {
    return this->cellImage;
}

GTImage::~GTImage() {
    
}
