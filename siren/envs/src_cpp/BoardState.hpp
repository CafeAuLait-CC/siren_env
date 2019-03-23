//
//  BoardState.hpp
//  Siren_Environment
//
//  Created by Alex on 3/20/19.
//  Copyright © 2019 Pinjing [Alex] Xu. All rights reserved.
//

#ifndef BoardState_hpp
#define BoardState_hpp

#include <iostream>
#include <opencv2/core.hpp>

#include "RGBImage.hpp"
#include "GTImage.hpp"

//
// TODO: Go for 10 x 10 for cell size, 800 x 800 for pixel based patch size.
// 1. divide the 4096 x 4096 image into 10 x 10 grids;
// 2. determine color for each cell, median pool for rgb image, 70% threshold for road mask and building mask
// 3. combine building mask and road mask into single matrix
// 4. generatePatch to get patch from image. 80 x 80 cell resolution, 50% overlap (step size of 40 cells)
//

class BoardState {
public:
    
    // Init state (read config from file, setup stuffs)
    BoardState();
    
    // State operations
    void getCurrentState(const RGBImage& rgbImg, GTImage& gtImg);
    void getNextState(std::string action, const RGBImage& rgbImg, GTImage& gtImg);  // MARK: const RGBImage? I need to draw tracks on rgb, does it affect?
    
    std::vector<std::string> getActionList();       // Get all actions
    std::vector<std::string> getLegalActions();     // Get legal actions
    
    void isDone();
    bool currentImageDone = false;
    
    void reset(bool toCurrentImage);    // Ff done, reset the entire state.
                                        // If toCurrentImage is true, reset using current image patch, otherwise, reset using next image patch.
    
private:
    RGBImage rgbImg;
    GTImage gtImg;
    
    std::vector<std::string> actionList;
    cv::Point2i currentPosition;
    
    cv::Mat state;  // Use pointer or not?
    cv::Mat nextState;
};

#endif /* BoardState_hpp */
