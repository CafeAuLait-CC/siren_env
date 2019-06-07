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
    cv::Mat getState();     // for debug
    cv::Mat getAlpha();     // for debug (temporary)
    cv::Mat getMiniMap();   // for debug (temporary)
    
    // State operations
    cv::Mat getCurrentState();
    cv::Mat getNextState(float action, bool checkActionLegality);
    
    std::vector<float> getActionList();       // Get all actions
    std::vector<float> getLegalActions();     // Get legal actions
    
    int getReward();    // Get current rewards
    int getMiniMapReward();
    bool isDone();      // Check if the game is done
    
    void reset(bool toCurrentImage);    // If done, reset the entire state.
                                        // If toCurrentImage is true, reset using current image tile, otherwise, reset using next image tile.
    
private:
    
    /** @brief Load configurations from file. Including file path, patch size, cell size, etc...
     */
    void readConfigFromFile();
    
    /** @brief Get all image file names from a text file. Each line in the .txt file is a image file name.
     @param fileName The name of text file (with its path, for example: ../images/name_list.txt )
     @return A vector that contains all the image file names.
     */
    std::vector<std::string> getImageFileNames(std::string fileName);
    
    
    // Initializations
    
    /** @brief Select the Nth imagery tile to initialize a new chessboard.
     @param fileNameNum The patch at fileNameList[fileNameNum] will be used.
     */
    void initBoardState(int fileNameNum);
    
    /** @brief Initialize all possible actions and save them into a vector of strings. (std::vector<std::string> actionList)
     */
    void initActionList();
    
    /** @brief Random number generator.
     @return A random number in the range [0, maxNum).
     */
    int getRandomNumInRange(int maxNum);
    
    /** @brief Set a location to start the progress. The first road cell from the top left corner of a patch is used.
     */
    void setStartLocation();
    
    /** @brief Get the next position given an action.
     A legal actions means this action won't make the agent go off the road.
     @param action The action givin to the agent. A float number in range [0, 180)
     @param nextPosition The destination position after applying the given action.
     @param checkActionLegality If it's necessory to check the action legality. Normally true for training, false for testing in learning agent
     @return if it is a legal action and output the next position.
     */
    bool getNextPosition(float action, cv::Point2i &nextPosition, const bool &checkActionLegality);
    
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
    
    /** @brief Generate the observation space for learning agent.
     @param position The observation space will be around the (current) position.
     */
    void generateObservationPatch(const cv::Point2i& position);
    
    /** @brief Padding for imagery patches if it goes out of image tile boundary.
     @param pad_up How much space above the imagery patch.
     @param pad_down How much space below the imagery patch.
     @param pad_left How much space on the left of the imagery patch.
     @param pad_right How much space on the right of the imagery patch.
     @param x_up Start row in the entire imagery tile.
     @param x_down End row in the entire imagery tile.
     @param y_left Start column in the entire imagery tile.
     @param y_right end column in the entire imagery tile.
     */
    void paddingForImageryPatch(int &pad_up, int &pad_down, int &pad_left, int &pad_right, int &x_up, int &x_down, int &y_left, int &y_right);
    
    /** @brief Generate a 4th channel (alpha) that shows the history visited road cell in this window/patch.
     Parameters are from the previous step - `paddingForImageryPatch()`
     @param pad_up How much space above the imagery patch.
     @param pad_down How much space below the imagery patch.
     @param pad_left How much space on the left of the imagery patch.
     @param pad_right How much space on the right of the imagery patch.
     @param x_up Start row in the entire imagery tile.
     @param y_left Start column in the entire imagery tile.
     */
    void generateAlphaChannel(const int &pad_up, const int &pad_down, const int &pad_left, const int &pad_right, const int &x_up, const int &y_left);
    
    /** @brief Generate a mini map indicating the current area in the complete imagery.
     */
    void generateMiniMap();
    
    /** @brief Apply action and return the type of the next cell, update "currentPosition" in the mean time.
     @param nextPosition Update current position to next position
     @return Type of the next cell, one of "VisitedRoad", "UnvisitedRoad", "RoadNeighbor", "TravelPath".
     */
    std::string applyAction(const cv::Point2i& nextPosition);
    
    /** @brief Draw line on the imagery to show the visited pixels, color of this line shows the number of visited time of this pixel.
     @param prevPositionOnState Start point of this line segment, represented with state-coordinate, not pixel coordinate.
     @param nextPositionOnState end point of this line segment, represented with state-coordinate, not pixel coordinate.
     @param pixelValue The value of pixels on this line.
     */
    void drawLineOnImagery(const cv::Point2i& prevPositionOnState, const cv::Point2i& nextPositionOnState, const int pixelValue);
    
    /** @brief Draw line on the state to record the visited cells, color of this line shows the number of visited time of this pixel.
     @param prevPosition Start point of this line segment.
     @param nextPosition end point of this line segment.
     @param pixelValue The value of pixels on this line.
     */
    void drawLineOnState(const cv::Point2i& prevPosition, const cv::Point2i& nextPosition, const int pixelValue);
    
    
    std::string pathToGTImages;
    std::string pathToImagery;
    std::vector<std::string> fileNameListGT;
    std::vector<std::string> fileNameListImagery;
    
    std::vector<float> actionList;
    cv::Point2i currentPosition;
    
    cv::Mat imagery;
    cv::Mat state;      // Use pointer or not? - No.
    
    cv::Mat imageryPatch;    // The imagery patch around current position
    cv::Mat alphaPatch; // a fourth channel shows recent visited road cells
    cv::Mat miniMap;    // not using
    
    cv::Size cellSize;  // basic unit in chessboard state
    cv::Size patchSize; // patch size for the network
    cv::Size miniMapCellSize;   // cell size in mini map, not using
    int stepSize = 2;
    int currentFileNameNum = 0;
    
    int reward = 0;
    int totalNumRoadPoints = 0;
    int remainingRoadPoints = 0;
    int completenessReward = 0;
    
    bool gameover = false;  // A flag used to finish the current epoch
};

#endif /* BoardState_hpp */
