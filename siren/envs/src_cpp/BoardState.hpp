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
#include <fstream>
#include <stdlib.h>
#include <opencv2/core.hpp>

#include "RGBImage.hpp"
#include "GTImage.hpp"

class BoardState {
public:
    
    // Init state (read config from file, setup stuffs)
    BoardState();
    cv::Mat getState(); // for debug
    
    // State operations
    cv::Mat getCurrentState();
    cv::Mat getNextState(std::string action);
    
    std::vector<std::string> getActionList();       // Get all actions
    std::vector<std::string> getLegalActions();     // Get legal actions
    
    int getReward();    // Get current rewards
    bool isDone();      // Check if the game is done
    
    void reset(bool toCurrentImage);    // If done, reset the entire state.
                                        // If toCurrentImage is true, reset using current image patch, otherwise, reset using next image patch.
    
private:
    
    /** @brief Load configurations from file. Including file path, patch size, cell size, etc...
     */
    void readConfigFromFile();
    
    /** @brief Get all image file names from a text file. Each line in the .txt file is a image file name.
     @param fileName The name of text file (with its path, for example: ../images/name_list.txt )
     */
    std::vector<std::string> getImageFileNames(std::string fileName);
    
    
    // inits
    
    /** @brief Select the Nth image patch to initialize a new chessboard.
     @param fileNameNum The patch at fileNameList[fileNameNum] will be used.
     */
    void initBoardState(int fileNameNum);
    
    /** @brief Initialize all possible actions and save them into a vector of strings. (std::vector<std::string> actionList)
     */
    void initActionList();
    
    /** @brief Random number generator used for patch sampling.
     */
    int getNewFileNameNum();
    
    /** @brief Set a location to start the progress. The first road cell from the top left corner of a patch is used.
     */
    void setStartLocation();
    
    /** @brief Check if a given action is a legal action.
     A legal actions means this action won't make the agent go across a building.
     @param action The action givin to the agent. One of ['Stop', 'North', 'South', 'East', 'West', 'NE', 'NW', 'SE', 'SW']
     */
    bool checkActionLegality(std::string action);
    
    /** @brief Check the two neighbors along the direction that perpendicular to the action direction.
     If any of the two neighbors is road cell, return true.
     @param action The action givin to the agent. One of ['North', 'South', 'East', 'West', 'NE', 'NW', 'SE', 'SW']. It's different than checkActionLegality() !
     */
    bool checkMoveDirectionNeighbors(cv::Point2i prevPosition, const std::string& action);
    
    /** @brief Draw a yellow cell at the visited cell (on RGB imagery patch around current position).
     @param position Draw the cell at this(current) position
     */
    void addYellowCell2Imagery(const cv::Point2i& position);

    /** @brief Padding for imagery patches if it goes out of image tile boundary.
     @param position Draw the cell at this(current) position
     */
    void paddingForImageryPatch(const cv::Point2i& position);
    
    /** @brief Apply action and return if the next step lies on road pixel, update "currentPosition" in the mean time.
     @param action Draw the cell at this(current) position
     */
    bool applyAction(std::string action); //
    
    
    std::string pathToGTImages;
    std::string pathToImagery;
    std::vector<std::string> fileNameListGT;
    std::vector<std::string> fileNameListImagery;
    
    std::vector<std::string> actionList;
    cv::Point2i currentPosition;
    
    cv::Mat imagery;
    cv::Mat state;      // Use pointer or not?
    cv::Mat imageryPatch;    // The imagery patch around current position
    cv::Size cellSize;  // basic unit in chessboard state
    cv::Size patchSize; // patch size for the network
    int stepSize = 1;   // not used yet
    int currentFileNameNum = 0;
    int reward = 0;
    int totalNumRoadPoints = 0;
    int remainingRoadPoints = 0;
    bool currentImageDone = false;  // A flag used to decide if reset to a new patch or use the same patch
};

#endif /* BoardState_hpp */
