//
//  BoardState.cpp
//  Siren_Environment
//
//  Created by Alex on 3/20/19.
//  Copyright © 2019 Pinjing [Alex] Xu. All rights reserved.
//

#include "BoardState.hpp"

BoardState::BoardState() {
    // TODO: read configuration from config.txt file
    // including 'action_list', 'original_patch_size', 'new_patch_size', 'cell_size'...
    readConfigFromFile();
    initActionList();
    this->currentFileNameNum = getNewFileNameNum(); // random number in range [0, actionList.size())
    initBoardState(currentFileNameNum); // random sample a patch to init the board
}

// configurations
void BoardState::readConfigFromFile() {
    std::string basePath = "/Users/alex/Documents/Concordia University/Project/Google Maps Data/Training Data/";
    this->pathToGTImages = basePath + "gt_in_tiles/";
    this->fileNameListGT = getImageFileNames(basePath + "name_list_building.txt");
    this->pathToImagery = basePath + "imagery/";
    this->fileNameListImagery = getImageFileNames(basePath + "name_list_imagery.txt");
    
    this->patchSize = cv::Size(200, 200);
}

// Chessboard initialization: background board(gt) + forground board(imagery for network)
void BoardState::initBoardState(int fileNameNum) {
    std::string fileNameImagery = this->pathToImagery + this->fileNameListImagery[fileNameNum];
    std::string fileNameGT = this->pathToGTImages + this->fileNameListGT[fileNameNum];
    
    // Load image patch (rgb + gt) from file system
    RGBImage* rgbImg = new RGBImage(fileNameImagery);
    // this->imagery = rgbImg->getImagery();        // See the comment below!
    rgbImg->getImagery().copyTo(this->imagery);
    delete rgbImg;
    GTImage* gtImg = new GTImage(fileNameGT, cv::Size(10, 10), this->imagery.size());
    
    // Convert from pixel-based image to cell-based board
    /** Two options here
        1. this->state = gtImg->getPattern(); without delete     --> use reference,
        2. gtImg->getPattern().copyTo(this->state) with delete   --> copy data.
        Becareful with the difference here!

        !! UPDATE: Should use option 2, otherwise it will cause problem in python!
                   Same with the 'rgbImg' above.
     */
    // this->state = gtImg->getPattern();
   gtImg->getPattern().copyTo(this->state);
   delete gtImg;

    setStartLocation();     // Set a start cell for the agent. (Choose the first road cell as start point)
    
    // Calculate cell size. Height and width can be different.
    cv::Size s_i = this->imagery.size();
    cv::Size s_g = this->state.size();
    this->cellSize = cv::Size(s_i.width / s_g.width, s_i.height / s_g.height);
    
    this->imageryPatch = cv::Mat::zeros(this->patchSize, CV_8UC3);
    
    // Label visited cell using a yellow block. (Here is the first one, starting point)
    addYellowCell2Imagery(currentPosition);
    
    // Count number of road cells
    for (int i = 0; i < this->state.rows; i++) {
        for (int j = 0; j < this->state.cols; j++) {
            if (this->state.at<uchar>(i, j) == 1) {
                this->remainingRoadPoints++;
            }
        }
    }
    this->totalNumRoadPoints = this->remainingRoadPoints;
}

// This function is used to draw yellow cells on current position patch
void BoardState::addYellowCell2Imagery(const cv::Point2i& position) {
    cv::Mat yellowCellChannel1 = cv::Mat::ones(this->cellSize, CV_8UC1) * 255;
    cv::Mat yellowCellChannel2 = cv::Mat::zeros(this->cellSize, CV_8UC1);
    std::vector<cv::Mat> yellowCellData;
    yellowCellData.push_back(yellowCellChannel2);
    yellowCellData.push_back(yellowCellChannel1);
    yellowCellData.push_back(yellowCellChannel1);
    cv::Mat yellowCell;
    cv::merge(yellowCellData, yellowCell);
    
    // Padding: zero padding at edges of imagery tile
    paddingForImageryPatch(position);

    // Only updating current patch, without draw yellow box at center
    // yellowCell.copyTo(this->imageryPatch(cv::Range(this->patchSize.height / 2 - cellSize.height / 2, this->patchSize.height / 2 + cellSize.height / 2),
                                   // cv::Range(this->patchSize.width / 2 - cellSize.width / 2, this->patchSize.width / 2 + cellSize.width / 2)));
}

void BoardState::paddingForImageryPatch(const cv::Point2i& position) {
    
    // Generate new current patch and draw yellow cell at center
    int radius_row = this->imageryPatch.size().height / this->cellSize.height / 2;
    int radius_col = this->imageryPatch.size().width / this->cellSize.width / 2;

    int pad_up = 0;
    int pad_down = 0;
    int pad_left = 0;
    int pad_right = 0;
    int x_up = (position.x - radius_row) * this->cellSize.height;
    int x_down = (position.x + radius_row) * this->cellSize.height;
    int y_left = (position.y - radius_col) * this->cellSize.width;
    int y_right = (position.y + radius_col) * this->cellSize.width;
    
    if (x_up < 0) {
        pad_up = x_up * -1;
        x_up = 0;
    } else if (x_down > this->imagery.rows - 1) {
        pad_down = x_down - this->imagery.rows;
        x_down = this->imagery.rows;
    }
    if (y_left < 0) {
        pad_left = y_left * -1;
        y_left = 0;
    } else if (y_right > this->imagery.cols - 1) {
        pad_right = y_right - this->imagery.cols;
        y_right = this->imagery.cols;
    }

    this->imageryPatch = cv::Mat::zeros(this->imageryPatch.size(), this->imageryPatch.type());
    this->imagery(cv::Range(x_up, x_down), cv::Range(y_left, y_right)).copyTo(
        this->imageryPatch(cv::Range(pad_up, this->imageryPatch.rows - pad_down), cv::Range(pad_left, this->imageryPatch.cols - pad_right)));
}

// Init all posible actions
void BoardState::initActionList() {
    // TODO: modify to read from file
    this->actionList.push_back("Stop");
    this->actionList.push_back("North");
    this->actionList.push_back("NE");
    this->actionList.push_back("East");
    this->actionList.push_back("SE");
    this->actionList.push_back("South");
    this->actionList.push_back("SW");
    this->actionList.push_back("West");
    this->actionList.push_back("NW");
}

// Return all posible actions
std::vector<std::string> BoardState::getActionList() {
    return this->actionList;
}

// Return only legal actions at current state
std::vector<std::string> BoardState::getLegalActions() {
    std::vector<std::string> legalActions;
    for (int i = 0; i < this->actionList.size(); i++) {
        if (checkActionLegality(this->actionList[i])) {
            legalActions.push_back(this->actionList[i]);
        }
    }
    return legalActions;
}

// Return the current state (the RGB imagery)
cv::Mat BoardState::getCurrentState() {
    // Wait...what?
    // No need to return chessboard state to others.
    return this->imageryPatch;
}

// Calculate the next state with a specific action as input
cv::Mat BoardState::getNextState(std::string action) {
    if (checkActionLegality(action)) {
        if (applyAction(action) == "UnvisitedRoad") {       // If the step lies on an unvisited road cell
            this->reward += 10;
            this->remainingRoadPoints--;
        } else if (applyAction(action) == "VisitedRoad") {  // If the step lies on a visited road cell
            this->reward += 5;
        } else {                                            // If the step lies on a neighbor of road cell
            this->reward--; // Deduction of rewards is optional
        }
        addYellowCell2Imagery(currentPosition);     // Label visited cells
    } else {
        // Illegal action, do nothing or reduce reward.
        this->reward -= 50;
    }
    return this->imageryPatch;
}

// Return current reward.
int BoardState::getReward() {
    return this->reward;
}

// Check if the game is done.
bool BoardState::isDone() {
    if (currentImageDone) { // If some other function force the game to finish.
        return true;
    }
    /** Two options here:
        1. All road cells have been visited.
        2. Most road cells have been visited.
     */
    bool done = false;
    if (float(remainingRoadPoints) / float(totalNumRoadPoints) < 0.1) {   // Using option 2
        done = true;
        currentImageDone = true;
    }
    return done;
}

void BoardState::reset(bool toCurrentImage) {
    // insert code here...
    if (this->currentImageDone || toCurrentImage) {
        this->currentFileNameNum = getNewFileNameNum(); // Get a new random number
    }
    this->currentImageDone = false;
    this->reward = 0;
    this->remainingRoadPoints = 0;
    initBoardState(currentFileNameNum); // sample a patch to init the board
}

// Check the two neighbors along the direction that perpendicular to the action direction
// If any of the two neighbors is road cell, return true.
bool BoardState::checkMoveDirectionNeighbors(cv::Point2i nextPosition, const std::string& action) {
    bool hasRoadNeighbor = false;
    std::vector<cv::Point2i> threeNeighbours;
    threeNeighbours.push_back(nextPosition);
    
    // Collect two neighbors of nextPosition
    if (action == "North" || action == "South") {
        if (nextPosition.y == 0) {      // nextPosition is on the left edge
            threeNeighbours.push_back(cv::Point2i(nextPosition.x, nextPosition.y+1));
        } else if (nextPosition.y == this->state.cols-1) {  // nextPosition is on the right edge
            threeNeighbours.push_back(cv::Point2i(nextPosition.x, nextPosition.y-1));
        } else {
            threeNeighbours.push_back(cv::Point2i(nextPosition.x, nextPosition.y+1));
            threeNeighbours.push_back(cv::Point2i(nextPosition.x, nextPosition.y-1));
        }
    } else if (action == "East" || action == "West") {
        if (nextPosition.x == 0) {      // nextPosition is on the upper edge
            threeNeighbours.push_back(cv::Point2i(nextPosition.x+1, nextPosition.y));
        } else if (nextPosition.x == this->state.rows-1) {  // nextPosition is on the bottom edge
            threeNeighbours.push_back(cv::Point2i(nextPosition.x-1, nextPosition.y));
        } else {
            threeNeighbours.push_back(cv::Point2i(nextPosition.x+1, nextPosition.y));
            threeNeighbours.push_back(cv::Point2i(nextPosition.x-1, nextPosition.y));
        }
    } else if (action == "NE") {
        threeNeighbours.push_back(cv::Point2i(nextPosition.x, nextPosition.y-1));
        threeNeighbours.push_back(cv::Point2i(nextPosition.x+1, nextPosition.y));
    } else if (action == "NW") {
        threeNeighbours.push_back(cv::Point2i(nextPosition.x, nextPosition.y+1));
        threeNeighbours.push_back(cv::Point2i(nextPosition.x+1, nextPosition.y));
    } else if (action == "SE") {
        threeNeighbours.push_back(cv::Point2i(nextPosition.x, nextPosition.y-1));
        threeNeighbours.push_back(cv::Point2i(nextPosition.x-1, nextPosition.y));
    } else if (action == "SW") {
        threeNeighbours.push_back(cv::Point2i(nextPosition.x, nextPosition.y+1));
        threeNeighbours.push_back(cv::Point2i(nextPosition.x-1, nextPosition.y));
    } else {
        std::cerr << "checkMoveDirectionNeighbors: Impossible action: " + action << std::endl;
        exit(-1);
    }
    
    // Check if the three points contains road
    for (int i = 0; i < threeNeighbours.size(); i++) {
        if (this->state.at<uchar>(threeNeighbours[i].x, threeNeighbours[i].y) != 0) {
            hasRoadNeighbor = true;
            break;
        }
    }
    
    return hasRoadNeighbor;
}

// Check if a givin action is legal action - no crossing buildings
bool BoardState::checkActionLegality(std::string action) {
    bool isLegal = true;
    int currentX = currentPosition.x;
    int currentY = currentPosition.y;
    if (action == "North") {
        if (currentX == 0 ||
            !checkMoveDirectionNeighbors(cv::Point2i(currentX-1, currentY), action)) {
            isLegal = false;
        }
    } else if (action == "South") {
        if (currentX == (this->state.rows - 1) ||
            !checkMoveDirectionNeighbors(cv::Point2i(currentX+1, currentY), action)) {
            isLegal = false;
        }
    } else if (action == "East") {
        if (currentY == (this->state.cols - 1) ||
            !checkMoveDirectionNeighbors(cv::Point2i(currentX, currentY+1), action)) {
            isLegal = false;
        }
    } else if (action == "West") {
        if (currentY == 0 ||
            !checkMoveDirectionNeighbors(cv::Point2i(currentX, currentY-1), action)) {
            isLegal = false;
        }
    } else if (action == "NE") {
        if (currentX == 0 || currentY == (this->state.cols - 1) ||
            !checkMoveDirectionNeighbors(cv::Point2i(currentX-1, currentY+1), action)) {
            isLegal = false;
        }
    } else if (action == "NW") {
        if (currentX == 0 || currentY == 0 ||
            !checkMoveDirectionNeighbors(cv::Point2i(currentX-1, currentY-1), action)) {
            isLegal = false;
        }
    } else if (action == "SE") {
        if (currentX == (this->state.rows - 1) || currentY == (this->state.cols - 1) ||
            !checkMoveDirectionNeighbors(cv::Point2i(currentX+1, currentY+1), action)) {
            isLegal = false;
        }
    } else if (action == "SW") {
        if (currentX == (this->state.rows - 1) || currentY == 0 ||
            !checkMoveDirectionNeighbors(cv::Point2i(currentX+1, currentY-1), action)) {
            isLegal = false;
        }
    } else if (action == "Stop") {
        isLegal = false; // do not allow 'stop' action anymore
    } else {
        std::cerr << "Wrong action: " + action << std::endl;
        exit(-1);
    }
    
    return isLegal;
}

// Apply an action, update currentPosition,
// and return the type of the new currentPosition cell
// the agent give rewards according to this returned value
std::string BoardState::applyAction(std::string action) {
    this->state.at<uchar>(currentPosition.x, currentPosition.y) = 3;    // Change the value of agent's current position from 100 to 3,
                                                                        // 100 represents the current position, 3 means this position has been visited before
    std::string  cellType = "";
    if (action == "North") {
        currentPosition.x--;
    } else if (action == "South") {
        currentPosition.x++;
    } else if (action == "East") {
        currentPosition.y++;
    } else if (action == "West") {
        currentPosition.y--;
    } else if (action == "NE") {
        currentPosition.x--;
        currentPosition.y++;
    } else if (action == "NW") {
        currentPosition.x--;
        currentPosition.y--;
    } else if (action == "SE") {
        currentPosition.x++;
        currentPosition.y++;
    } else if (action == "SW") {
        currentPosition.x++;
        currentPosition.y--;
    } else {
        std::cerr << "Wrong action: " + action << std::endl;
        exit(-1);
    }
    if (this->state.at<uchar>(currentPosition.x, currentPosition.y) == 1) {
        cellType = "UnvisitedRoad";
    } else if (this->state.at<uchar>(currentPosition.x, currentPosition.y) == 3) {
        cellType = "VisitedRoad";
    } else {
        cellType = "RoadNeighbor";
    }
    this->state.at<uchar>(currentPosition.x, currentPosition.y) = 100;  // Update agent's current position
    return cellType;
}

// Get all image file names from a text file -- name_list_xxx.txt
// Return a vector of strings
std::vector<std::string> BoardState::getImageFileNames(std::string fileName) {
    std::ifstream inFile(fileName);
    std::vector<std::string> imageNames;
    std::string line;
    int countLines = 0;
    while (getline(inFile, line)) {
        imageNames.push_back(line);
        countLines++;
    }
    if (countLines == 0) {
        std::cerr << "readFile2Vector: Nothing in file! (Wrong file name?)" << std::endl;
        exit(-1);
    }
    sort(imageNames.begin(), imageNames.end());
    inFile.close();
    return imageNames;
}

// Set the start location in the chessboard, mark current position as 100
void BoardState::setStartLocation() {
    bool isSet = false;
    for (int i = 10; i < this->state.rows; i++) {
        if (isSet) {
            break;
        }
        for (int j = 10; j < this->state.cols; j++) {
            if (this->state.at<uchar>(i, j) == 1) {
                this->currentPosition = cv::Point2i(i, j);
                isSet = true;
                break;
            }
        }
    }
    this->state.at<uchar>(currentPosition.x, currentPosition.y) = 100;
}

// Random number generator used for sampling patches
int BoardState::getNewFileNameNum() {
    return rand() % this->fileNameListGT.size();
}

// For debug use
// Return current chessboard status
cv::Mat BoardState::getState() {
    return this->state;
}
