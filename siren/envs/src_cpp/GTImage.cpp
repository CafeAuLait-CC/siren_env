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
            if (i/cellSize.height >= cellImage.rows || j/cellSize.width >= cellImage.cols) {
                continue;   // Mark: Double check boundary to fix an division issue in Python
            }
            cv::Vec3b value = pixelImage.at<cv::Vec3b>(i, j);
            if (value == cv::Vec3b(0, 0, 0)) {
                cellImage.at<uchar>(i/cellSize.height, j/cellSize.width) = 0;   // nothing
            } else if (value == cv::Vec3b(0, 255, 0)) {
                cellImage.at<uchar>(i/cellSize.height, j/cellSize.width) = 1;   // road
            } else if (value == cv::Vec3b(255, 0, 0)) {
                cellImage.at<uchar>(i/cellSize.height, j/cellSize.width) = 2;   // building
            } else {
                 std::cerr << "Ground truth image pixel value wrong: " << i << ", " << j << " " << value << std::endl;;
                 exit(-1);
            }
            
        }
    }
}

cv::Mat GTImage::getPattern() {
    return this->cellImage;
}

GTImage::~GTImage() {
    
}
