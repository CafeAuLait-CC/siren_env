//
//  BoardState.cpp
//  Siren_Environment
//
//  Created by Alex Xu on 3/20/19.
//  Copyright © 2019 Alex Xu. All rights reserved.
//

#include "BoardState.hpp"

BoardState::BoardState() {
    // TODO: read configuration from config.txt file
    // including 'action_list', 'original_patch_size', 'new_patch_size', 'cell_size'...
    
}

std::vector<std::string> BoardState::getActionList() {
    return this->actionList;
}
