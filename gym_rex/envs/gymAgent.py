# gymAgent.py

from game import Agent
from game import Directions
import random

class gymAgent(Agent):
    """
    An agent controlled by Gym.
    """
    WEST_KEY  = 0
    EAST_KEY  = 1
    NORTH_KEY = 2
    SOUTH_KEY = 3
    STOP_KEY = 4

    def __init__( self, index = 0 ):

        self.lastMove = Directions.STOP
        self.index = index
        self.keys = []

    def getAction(self, state):
        from graphicsUtils import keys_waiting
        from graphicsUtils import keys_pressed
        keys = list(keys_waiting()) + list(keys_pressed())
        if keys != []:
            self.keys = keys

        legal = state.getLegalActions(self.index)
        move = self.getMove(legal)

        if move == Directions.STOP:
            # Try to move in the same direction as before
            if self.lastMove in legal:
                move = self.lastMove

        if (self.STOP_KEY in self.keys) and Directions.STOP in legal: move = Directions.STOP

        if move not in legal:
            move = random.choice(legal)

        self.lastMove = move
        return move

    def getMove(self, legal):
        move = Directions.STOP
        if   (self.WEST_KEY in self.keys or 'Left' in self.keys) and Directions.WEST in legal:  move = Directions.WEST
        if   (self.EAST_KEY in self.keys or 'Right' in self.keys) and Directions.EAST in legal: move = Directions.EAST
        if   (self.NORTH_KEY in self.keys or 'Up' in self.keys) and Directions.NORTH in legal:   move = Directions.NORTH
        if   (self.SOUTH_KEY in self.keys or 'Down' in self.keys) and Directions.SOUTH in legal: move = Directions.SOUTH
        return move
