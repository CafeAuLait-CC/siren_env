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
    this->currentFileNameNum = getRandomNumInRange(int(this->fileNameListGT.size())); // random number in range [0, actionList.size())
    initBoardState(currentFileNameNum); // random sample an imagery tile to init the board
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
    this->miniMap = cv::Mat::zeros(this->patchSize, CV_8UC1);
    this->miniMapCellSize = cv::Size(this->patchSize.height / (this->imagery.size().height / this->patchSize.height),
                                        this->patchSize.width / (this->imagery.size().width / this->patchSize.width));
    
    // Generate the observation patch, 5 channels
    generateObservationPatch(currentPosition);
    
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

// Generate the observation space
void BoardState::generateObservationPatch(const cv::Point2i& position) {
    
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
    
    // Padding: zero padding at edges of imagery tile
    paddingForImageryPatch(pad_up, pad_down, pad_left, pad_right, x_up, x_down, y_left, y_right);
    
    // Generate corresponding alpha channel to tell the network about recent move history
    generateAlphaChannel(pad_up, pad_down, pad_left, pad_right, x_up, y_left);
    
    // Generate the mini map
    generateMiniMap();
}

void BoardState::paddingForImageryPatch(int &pad_up, int &pad_down, int &pad_left, int &pad_right, int &x_up, int &x_down, int &y_left, int &y_right) {
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
    
    this->imageryPatch = cv::Mat::zeros(this->imageryPatch.size(), CV_8UC3);
    this->imagery(cv::Range(x_up, x_down), cv::Range(y_left, y_right))
        .copyTo(this->imageryPatch(cv::Range(pad_up, this->imageryPatch.rows - pad_down),
                                   cv::Range(pad_left, this->imageryPatch.cols - pad_right)));
}

void BoardState::generateAlphaChannel(const int &pad_up, const int &pad_down, const int &pad_left, const int &pad_right, const int &x_up, const int &y_left) {
    this->alphaPatch = cv::Mat::zeros(this->imageryPatch.size(), CV_8UC1);
    cv::Mat visitedCell = cv::Mat::ones(this->cellSize, this->alphaPatch.type()) * 255;
    for (int i = pad_up; i < this->alphaPatch.rows-pad_down; i+=this->cellSize.height) {
        for (int j = pad_left; j < this->alphaPatch.cols-pad_right; j+=this->cellSize.width) {
            int x = i + x_up - pad_up;
            int y = j + y_left - pad_left;
            int cellHeight = this->cellSize.height;
            int cellWidth = this->cellSize.width;
            if (this->state.at<uchar>(x / cellHeight, y / cellWidth) == 3 ||
                this->state.at<uchar>(x / cellHeight, y / cellWidth) > 100) {
                visitedCell.copyTo(this->alphaPatch(cv::Range(i, i + cellHeight), cv::Range(j, j + cellWidth)));
            }
        }
    }
}

void BoardState::generateMiniMap() {
    // No need to reset mini map every time
    // TODO: step size has a bug!
    cv::Mat visitedRegion = cv::Mat::ones(this->miniMapCellSize, this->miniMap.type()) * 255;
    int rowStepSize = this->state.size().height / (this->patchSize.height / this->miniMapCellSize.height);
    int colStepSize = this->state.size().width / (this->patchSize.width / this->miniMapCellSize.width);
    int x = this->currentPosition.x / rowStepSize;// * this->miniMapCellSize.height;
    int y = this->currentPosition.y / colStepSize;// * this->miniMapCellSize.width;
    if (this->miniMap.at<uchar>(x + miniMapCellSize.height / 2, y + this->miniMapCellSize.width / 2) == 0) {
        visitedRegion.copyTo(this->miniMap(cv::Range(x, x + this->miniMapCellSize.height), cv::Range(y, y + this->miniMapCellSize.width)));
        this->completenessReward += 20;     // Additional term for completeness
    }
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
    cv::Point2i tempPoint;      // tempPoint is useless here, just to fit into the parameters of checkActionLegality() function
    for (int i = 0; i < this->actionList.size(); i++) {
        if (getNextPosition(this->actionList[i], tempPoint)) {
            legalActions.push_back(this->actionList[i]);
        }
    }
    this->gameover = false; // Previous for-loop won't take real actions, but will trigger the gameover flag, reset it here.
    return legalActions;
}

// Return the current state (the RGB imagery)
cv::Mat BoardState::getCurrentState() {
    // Wait...what?
    // No need to return chessboard state to others.
    return this->imageryPatch;
}

// Calculate the next state with a specific action as input
cv::Mat BoardState::getNextState(std::string action, bool checkActionLegality) {
    cv::Point2i nextPosition = cv::Point2i(-1, -1);
    if (getNextPosition(action, nextPosition) || !checkActionLegality) {
        if (nextPosition == cv::Point2i(-1, -1)) {
            std::cerr << "getNextState: Illegal action: " << action << " from " << currentPosition << " to " << nextPosition << std::endl;
        }
        std::string nextCellType = applyAction(nextPosition);    // Apply the action, return the next cell Type
        if (nextCellType == "UnvisitedRoad") {       // If the step lies on an unvisited road cell
            this->reward += 20;
            this->remainingRoadPoints--;
        } else if (nextCellType == "VisitedRoad") { // If the step lies on a visited road cell
            this->reward--;
        } else if (nextCellType == "TravelPath") {  // If the step lies on a travel path cell (edges)
            this->reward--;
        } else {                                    // If the step lies on a neighbor of road cell
            this->reward--; // Deduction of rewards is optional
        }
        generateObservationPatch(currentPosition);
    } else {
        // Illegal action, do nothing or reduce reward.
        this->reward -= 5000;
    }
    return this->imageryPatch;
}

// Return current reward.
int BoardState::getReward() {
    return this->reward;
}

// Return reward of completeness
int BoardState::getMiniMapReward() {
    return this->completenessReward;
}

// Check if the game is done.
bool BoardState::isDone() {
    if (gameover) { // If some other function force the game to finish.
        return true;
    }
    /** Two options here:
        1. All road cells have been visited.
        2. Most road cells have been visited.
     */
    bool done = false;
    if (float(remainingRoadPoints) / float(totalNumRoadPoints) < 0.1) {   // Using option 2
        done = true;
    }
    return done;
}

void BoardState::reset(bool toCurrentImage) {
    // insert code here...
    if (this->gameover || toCurrentImage) {
        this->currentFileNameNum = getRandomNumInRange(int(this->fileNameListGT.size())); // Get a new random number
    }
    this->gameover = false;
    this->reward = 0;
    this->remainingRoadPoints = 0;
    this->completenessReward = 0;
    initBoardState(currentFileNameNum); // sample a imagery to init the board
}

// Check the two neighbors along the direction that perpendicular to the action direction
// If any of the two neighbors is road cell, return true.
bool BoardState::checkMoveDirectionNeighbors(const cv::Point2i& prevPosition, const cv::Point2i& nextPosition) {
    bool hasRoadNeighbor = false;
    std::vector<cv::Point2i> threeNeighbours;
    threeNeighbours = getNeighbors(prevPosition, nextPosition);
    
    // Check if the three points contains road
    for (int i = 0; i < threeNeighbours.size(); i++) {
        if (this->state.at<uchar>(threeNeighbours[i].x, threeNeighbours[i].y) != 0) {
            hasRoadNeighbor = true;
            break;
        }
    }
    return hasRoadNeighbor;
}

// Calculate neighbors of nextPosition
std::vector<cv::Point2i> BoardState::getNeighbors(const cv::Point2i& prevPosition, const cv::Point2i& nextPosition) {
    std::vector<cv::Point2i> neighbors;
    neighbors.push_back(nextPosition);  // careful with the & reference here!
    cv::Point2i neighbor1;
    cv::Point2i neighbor2;
    if (prevPosition.x == nextPosition.x || prevPosition.y == nextPosition.y) { // relation between prev and next is N, E, S, W
        cv::Point2i move = nextPosition - prevPosition;
        neighbor1 = nextPosition + cv::Point2i(move.y, move.x);
        neighbor2 = nextPosition - cv::Point2i(move.y, move.x);
    } else {        // relation between prev and next is NE, SE, NW, SW
        neighbor1 = cv::Point2i(prevPosition.x, nextPosition.y);
        neighbor2 = cv::Point2i(nextPosition.x, prevPosition.y);
    }
    
    // Boundary check
    if (neighbor1.x >= 0 && neighbor1.x < this->state.rows &&
        neighbor1.y >= 0 && neighbor1.y < this->state.cols) {
        neighbors.push_back(neighbor1);
    }
    if (neighbor2.x >= 0 && neighbor2.x < this->state.rows &&
        neighbor2.y >= 0 && neighbor2.y < this->state.cols) {
        neighbors.push_back(neighbor2);
    }
    return neighbors;
}

// Check if a givin action is legal action - not going out of a road
bool BoardState::getNextPosition(std::string action, cv::Point2i& nextPosition) {
    bool isLegal = true;
    int currentX = currentPosition.x;
    int currentY = currentPosition.y;
    if (action == "North") {
        if (currentX == 0 ||
            !checkMoveDirectionNeighbors(currentPosition, cv::Point2i(currentX-1, currentY))) {
            isLegal = false;
        }
        nextPosition = cv::Point2i(currentPosition.x-1, currentPosition.y);
    } else if (action == "South") {
        if (currentX == (this->state.rows - 1) ||
            !checkMoveDirectionNeighbors(currentPosition, cv::Point2i(currentX+1, currentY))) {
            isLegal = false;
        }
        nextPosition = cv::Point2i(currentPosition.x+1, currentPosition.y);
    } else if (action == "East") {
        if (currentY == (this->state.cols - 1) ||
            !checkMoveDirectionNeighbors(currentPosition, cv::Point2i(currentX, currentY+1))) {
            isLegal = false;
        }
        nextPosition = cv::Point2i(currentPosition.x, currentPosition.y+1);
    } else if (action == "West") {
        if (currentY == 0 ||
            !checkMoveDirectionNeighbors(currentPosition, cv::Point2i(currentX, currentY-1))) {
            isLegal = false;
        }
        nextPosition = cv::Point2i(currentPosition.x, currentPosition.y-1);
    } else if (action == "NE") {
        if (currentX == 0 || currentY == (this->state.cols - 1) ||
            !checkMoveDirectionNeighbors(currentPosition, cv::Point2i(currentX-1, currentY+1))) {
            isLegal = false;
        }
        nextPosition = cv::Point2i(currentPosition.x-1, currentPosition.y+1);
    } else if (action == "NW") {
        if (currentX == 0 || currentY == 0 ||
            !checkMoveDirectionNeighbors(currentPosition, cv::Point2i(currentX-1, currentY-1))) {
            isLegal = false;
        }
        nextPosition = cv::Point2i(currentPosition.x-1, currentPosition.y-1);
    } else if (action == "SE") {
        if (currentX == (this->state.rows - 1) || currentY == (this->state.cols - 1) ||
            !checkMoveDirectionNeighbors(currentPosition, cv::Point2i(currentX+1, currentY+1))) {
            isLegal = false;
        }
        nextPosition = cv::Point2i(currentPosition.x+1, currentPosition.y+1);
    } else if (action == "SW") {
        if (currentX == (this->state.rows - 1) || currentY == 0 ||
            !checkMoveDirectionNeighbors(currentPosition, cv::Point2i(currentX+1, currentY-1))) {
            isLegal = false;
        }
        nextPosition = cv::Point2i(currentPosition.x+1, currentPosition.y-1);
    } else if (action == "Stop") {
        isLegal = false; // do not allow 'stop' action any more
    } else {
        std::cerr << "Wrong action: " + action << std::endl;
        exit(-1);
    }
    if (!isLegal) {
        this->gameover = true;
    }
    return isLegal;
}

// Apply an action, update currentPosition,
// and return the type of the new currentPosition cell
// the agent give rewards according to this returned value
std::string BoardState::applyAction(const cv::Point2i& nextPosition) {
    this->state.at<uchar>(currentPosition.x, currentPosition.y) -= 100;
    if (this->state.at<uchar>(currentPosition.x, currentPosition.y) == 1) {
        // Change the value of agent's current position from 100+ to 3,
        // 100 represents the current position, 3 means this position has been visited before.
        this->state.at<uchar>(currentPosition.x, currentPosition.y) = 3;
    }
    currentPosition = nextPosition;     // using & reference here!
    std::string  cellType = "";
    if (this->state.at<uchar>(currentPosition.x, currentPosition.y) == 1) {
        cellType = "UnvisitedRoad";
    } else if (this->state.at<uchar>(currentPosition.x, currentPosition.y) == 3) {
        cellType = "VisitedRoad";
    } else if (this->state.at<uchar>(currentPosition.x, currentPosition.y) == 4) {
        cellType = "TravelPath";
    } else {
        cellType = "RoadNeighbor";
    }
    this->state.at<uchar>(currentPosition.x, currentPosition.y) += 100;  // Update agent's current position
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
    this->state.at<uchar>(currentPosition.x, currentPosition.y) = 103;
}

// Random number generator used for sampling patches
int BoardState::getRandomNumInRange(int maxNum) {
    return rand() % maxNum;
}

// For debug use
// Return current chessboard status
cv::Mat BoardState::getState() {
    return this->state;
}

// Return the alpha channel
cv::Mat BoardState::getAlpha() {
    return this->alphaPatch;
}

// Return the mini map
cv::Mat BoardState::getMiniMap() {
    return this->miniMap;
}
