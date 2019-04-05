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
     @return A vector that contains all the image file names.
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
     @return A random number indicate the file index in the fileNameList
     */
    int getNewFileNameNum();
    
    /** @brief Set a location to start the progress. The first road cell from the top left corner of a patch is used.
     */
    void setStartLocation();
    
    /** @brief Check if a given action is a legal action.
     A legal actions means this action won't make the agent go across a building.
     @param action The action givin to the agent. One of ['Stop', 'North', 'South', 'East', 'West', 'NE', 'NW', 'SE', 'SW'].
     @param nextPosition The destination position after applying the given action.
     @return true and output the next position if it is a legal action.
     */
    bool checkActionLegality(std::string action, cv::Point2i& nextPosition);
    
    /** @brief Check the two neighbors along the direction that perpendicular to the action direction.
     If any of the two neighbors is road cell, return true.
     @param prevPosition The source position before action.
     @param nextPosition The destination position after action.
     @return true if at least one of the three cells in the moving direction is road cell.
     */
    bool checkMoveDirectionNeighbors(const cv::Point2i& prevPosition, const cv::Point2i& nextPosition);
    
    /** @brief Calculate the neighbors of next cell when moving from current cell.
     @param prevPosition The source position (cell) before moving (currentPosition).
     @param nextPosition The destination cell after moving.
     @return Vector of cv::Point2i, contains 2 or 3 points (nextPosition and its neighbor(s))
     */
    std::vector<cv::Point2i> getNeighbors(const cv::Point2i& prevPosition, const cv::Point2i& nextPosition);
    

    /** @brief Padding for imagery patches if it goes out of image tile boundary.
     @param position zero padding the image patch around (current) position.
     */
    void paddingForImageryPatch(const cv::Point2i& position);
    
    /** @brief Apply action and return the type of the next cell, update "currentPosition" in the mean time.
     @param nextPosition Update current position to next position
     @return Type of the next cell, one of "VisitedRoad", "UnvisitedRoad", "RoadNeighbor", "TravelPath".
     */
    std::string applyAction(const cv::Point2i& nextPosition);
    
    
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
