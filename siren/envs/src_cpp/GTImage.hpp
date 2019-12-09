//
//  GTImage.hpp
//  Siren_Environment
//
//  Created by Alex on 3/20/19.
//  Copyright © 2019 Pinjing [Alex] Xu. All rights reserved.
//

#ifndef GTImage_hpp
#define GTImage_hpp

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

class GTImage {
public:
    
    // Initialize
    GTImage();
    GTImage(std::string& fileName, cv::Size cellSize = cv::Size(8, 8), cv::Size pixelSize = cv::Size(256, 256));    // cv::Size(width, height)
//    void generateGTPattern();
    
    //
//    void generateNextState();     // implement this function in RGBImage class
    cv::Mat getPattern();
    ~GTImage();
    
    
//    cv::Mat pixelImageRoad;

private:
    cv::Mat cellImage;  // 3 channels, Vec3b (building, road, x)
    cv::Mat pixelImage;
//    int numOfRemainPoints;
//    cv::Point2i currentPosition;    // (row, col)
//    std::string actionList;
};

#endif /* GTImage_hpp */
