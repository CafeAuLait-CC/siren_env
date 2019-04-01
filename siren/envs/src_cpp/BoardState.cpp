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
    this->imagery = rgbImg->getImagery();
    GTImage* gtImg = new GTImage(fileNameGT, cv::Size(10, 10), this->imagery.size());
    
    // Convert from pixel-based image to cell-based board
    /** Two options here
        1. this->state = gtImg->getPattern(); without delete     --> use reference,
        2. gtImg->getPattern().copyTo(this->state) with delete   --> copy data.
        Becareful with the difference here!
     */
    this->state = gtImg->getPattern();
//    gtImg->getPattern().copyTo(this->state);
//    delete gtImg;

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
}

// This function is used to draw yellow cells on current position patch
void BoardState::addYellowCell2Imagery(const cv::Point2i& position) {
    cv::Mat yellowCellChannel1 = cv::Mat::ones(this->cellSize, CV_8UC1) * 255;
    cv::Mat yellowCellChannel2 = cv::Mat::zeros(this->cellSize, CV_8UC1);
    std::vector<cv::Mat> yellowCellData;
    yellowCellData.push_back(yellowCellChannel2);
    yellowCellData.push_back(yellowCellChannel1);
    yellowCellData.push_back(yellowCellChannel1);
    cv::Mat yelloCell;
    cv::merge(yellowCellData, yelloCell);
    
    // Generate new current patch and draw yellow cell at center
    int radius_row = this->imageryPatch.size().height / this->cellSize.height / 2;
    int radius_col = this->imageryPatch.size().width / this->cellSize.width / 2;
    this->imagery(cv::Range((position.x - radius_row) * this->cellSize.height, (position.x + radius_row) * this->cellSize.height),
                  cv::Range((position.y - radius_col) * this->cellSize.width, (position.y + radius_col) * this->cellSize.width)).copyTo(this->imageryPatch) ;
    
    yelloCell.copyTo(this->imageryPatch(cv::Range(this->patchSize.height / 2 - cellSize.height / 2, this->patchSize.height / 2 + cellSize.height / 2),
                                   cv::Range(this->patchSize.width / 2 - cellSize.width / 2, this->patchSize.width / 2 + cellSize.width / 2)));
}

// Init all posible actions
void BoardState::initActionList() {
    // TODO: modify to read from file
    this->actionList.push_back("Stop");
    this->actionList.push_back("North");
    this->actionList.push_back("South");
    this->actionList.push_back("East");
    this->actionList.push_back("West");
    this->actionList.push_back("NE");
    this->actionList.push_back("SE");
    this->actionList.push_back("NW");
    this->actionList.push_back("SW");
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
        if (applyAction(action)) {  // If the step lies on a road cell
            this->reward += 10;
            this->remainingRoadPoints--;
        } else {
            this->reward--; // Deduction of rewards is optional
        }
        addYellowCell2Imagery(currentPosition);     // Label visited cells
    } else {
        // Illegal action, do nothing or reduce reward.
        this->reward -= 2;
//        this->currentImageDone = true;
    }
    return this->imagery;
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
    if (remainingRoadPoints == 0) {
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

// Check if a givin action is legal action - no crossing buildings
bool BoardState::checkActionLegality(std::string action) {
    bool isLegal = true;
    int currentX = currentPosition.x;
    int currentY = currentPosition.y;
    if (action == "North") {
        if (currentX == 0 || this->state.at<uchar>(currentX-1, currentY) == 2) {
            isLegal = false;
        }
    } else if (action == "South") {
        if (currentX == (this->state.rows - 1) || this->state.at<uchar>(currentX+1, currentY) == 2) {
            isLegal = false;
        }
    } else if (action == "East") {
        if (currentY == (this->state.cols - 1) || this->state.at<uchar>(currentX, currentY+1) == 2) {
            isLegal = false;
        }
    } else if (action == "West") {
        if (currentY == 0 || this->state.at<uchar>(currentX, currentY-1) == 2) {
            isLegal = false;
        }
    } else if (action == "NE") {
        if (currentX == 0 || currentY == (this->state.cols - 1) || this->state.at<uchar>(currentX-1, currentY+1) == 2) {
            isLegal = false;
        }
    } else if (action == "NW") {
        if (currentX == 0 || currentY == 0 || this->state.at<uchar>(currentX-1, currentY-1) == 2) {
            isLegal = false;
        }
    } else if (action == "SE") {
        if (currentX == (this->state.rows - 1) || currentY == (this->state.cols - 1) || this->state.at<uchar>(currentX+1, currentY+1) == 2) {
            isLegal = false;
        }
    } else if (action == "SW") {
        if (currentX == (this->state.rows - 1) || currentY == 0 || this->state.at<uchar>(currentX+1, currentY-1) == 2) {
            isLegal = false;
        }
    } else if (action == "Stop") {
        ; // do nothing
    } else {
        std::cerr << "Wrong action: " + action << std::endl;
        exit(-1);
    }
    
    return isLegal;
}

// Apply an action, update currentPosition,
// and return if the new currentPosition is on a road cell
// the agent give rewards according to this returned value
bool BoardState::applyAction(std::string action) {
    this->state.at<uchar>(currentPosition.x, currentPosition.y) = 0;    // Change the value of agent's current position from 100 to 0,
                                                                        // 100 represents the current position
    bool isRoad = false;
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
    } else if (action == "Stop") {
        // do nothing
    } else {
        std::cerr << "Wrong action: " + action << std::endl;
        exit(-1);
    }
    if (this->state.at<uchar>(currentPosition.x, currentPosition.y) == 1) {
        isRoad = true;
    }
    this->state.at<uchar>(currentPosition.x, currentPosition.y) = 100;  // Update agent's current position
    return isRoad;
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
