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
                cellImage.at<uchar>(i/cellSize.height, j/cellSize.width) = 0;   // label 0 for nothing (black pixels, including non-centerlines and unknown labels)
            } else if (value == cv::Vec3b(0, 255, 0)) {
                cellImage.at<uchar>(i/cellSize.height, j/cellSize.width) = 1;   // label 1 for road
            } else if (value == cv::Vec3b(255, 0, 0)) {
                cellImage.at<uchar>(i/cellSize.height, j/cellSize.width) = 0;   // label 0 (or 2) for building
            } else {
                 std::cerr << "Ground truth image pixel value wrong: " << i << ", " << j << " " << value << std::endl;;
                 exit(-1);
            }
            
        }
    }
    
    // Add a travel path to the edges
    // When agent reaches an edge, use the travel path to find next entry point.
    for (int i = 0; i < cellImage.rows; i++) {
        if (i > 0 && i < cellImage.rows - 1) {  // not the first or last row, only label the left and right edges.
            cellImage.at<uchar>(i, 0) = 4;      // label 4 for travel path
            cellImage.at<uchar>(i, cellImage.cols - 1) = 4;
        } else {        // the first and last row, label all pixel as 4
            cv::Mat allOnesRow = cv::Mat::ones(1, cellImage.cols, 0) * 4;
            allOnesRow.copyTo(cellImage(cv::Range(0, 1), cv::Range(0, cellImage.cols)));
            allOnesRow.copyTo(cellImage(cv::Range(cellImage.rows - 1, cellImage.rows), cv::Range(0, cellImage.cols)));
        }
    }
}

cv::Mat GTImage::getPattern() {
    return this->cellImage;
}

GTImage::~GTImage() {
    
}
