# gymAgent.py
# This file is currently useless, actions are given by the Gym environment

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
        pass

    def getMove(self, legal):
        pass
