//
//  GTImage.hpp
//  Siren_Environment
//
//  Created by Alex Xu on 3/20/19.
//  Copyright © 2019 Alex Xu. All rights reserved.
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
    GTImage(cv::Size cellSize, cv::Size pixelSize = cv::Size(160, 170));    // cv::Size(width, height)
    void generateGTPattern();
    
    //
//    void generateNextState();     // implement this function in RGBImage class
    
    
    cv::Mat cellImage;  // 3 channels, Vec3b (building, road, x)
    cv::Mat pixelImageBuilding;
    cv::Mat pixelImageRoad;

private:
    int numOfRemainPoints;
    cv::Point2i currentPosition;    // (row, col)
    std::string actionList;
};

#endif /* GTImage_hpp */
