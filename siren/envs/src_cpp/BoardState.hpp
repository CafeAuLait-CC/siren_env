//
//  BoardState.hpp
//  Siren_Environment
//
//  Created by Alex Xu on 3/20/19.
//  Copyright © 2019 Alex Xu. All rights reserved.
//

#ifndef BoardState_hpp
#define BoardState_hpp

#include <iostream>
#include <opencv2/core.hpp>

#include "RGBImage.hpp"
#include "GTImage.hpp"

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
