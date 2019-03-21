import gym
from gym import error, spaces, utils
from gym.utils import seeding
import numpy as np
import cv2
import sys
from os import path

dir = path.dirname(__file__)
sys.path.append(dir)

from pacman import *
from game import Actions

class SirenEnv(gym.Env):
	metadata = {'render.modes' : ['human']}

	def __init__(self):
		# create the game board
		args = readCommand() # Get game components based on input
		args['pacman'] = "gymAgents"
		rules = ClassicGameRules(args['timeout'])
		beQuiet = True
		import textDisplay
		gameDisplay = textDisplay.NullGraphics()
		rules.quiet = True
		self.game = rules.newGame(args['layout'], args['pacman'], args['ghosts'], gameDisplay, beQuiet, args['catchExceptions'])
		self.game.numMoves = 0
		self.done = self.game.gameOver
		self.previousReward = 0
		self.reward = self.game.state.getScore()
		self.info = {}
		self.action_space = spaces.Discrete(5)	# len(self.unwrapped.get_action_meanings())
		self.observation_space = spaces.Box(0, 255, (210, 160, 3), dtype=np.uint8)
		self.gameBoard = initGameBoard()
		self.state = stateGrid2Img(self.gameBoard, np.array(self.game.state.data.food.data), self.game.state.data.agentStates[0])

	def step(self, action):
		if action == 0:
			action = 'Stop'
		elif action == 1:
			action = 'North'
		elif action == 2:
			action = 'South'
		elif action == 3:
			action = 'East'
		else:
			action = 'West'
		legalAction = self.game.state.getLegalPacmanActions()
		if action not in legalAction:
			self.game.numMoves += 1
			if self.game.numMoves > 500:
				self.done = True
				self.info['info'] = 'Failed to complete. Too many moves.'
				self.info['numMoves'] = self.game.numMoves
			return [self.state, 0, self.done, self.info]
		# action = list(Actions._directions.keys())[action]	# action format from number to string
		self.previousReward = self.reward
		self.game.state = self.game.state.generateSuccessor(self.game.startingIndex, action)
		self.game.display.update(self.game.state.data)
		self.game.rules.process(self.game.state, self.game)
		self.game.numMoves += 1
		self.game.display.finish()
		self.state = stateGrid2Img(self.gameBoard, np.array(self.game.state.data.food.data), self.game.state.data.agentStates[0])
		self.reward = self.game.state.getScore()
		self.done = self.game.gameOver
		if self.game.numMoves > 500:
			self.done = True
			self.info['info'] = 'Failed to complete. Too many moves.'
			self.info['numMoves'] = self.game.numMoves
		rewardChanged = self.reward - self.previousReward
		if rewardChanged < 0:
			rewardChanged = 0
		return [self.state, rewardChanged, self.done, self.info]

	def reset(self):
		args = readCommand() # Get game components based on input
		rules = ClassicGameRules(args['timeout'])
		beQuiet = True
		import textDisplay
		gameDisplay = textDisplay.NullGraphics()
		rules.quiet = True
		self.game = rules.newGame(args['layout'], args['pacman'], args['ghosts'], gameDisplay, beQuiet, args['catchExceptions'])
		self.game.numMoves = 0
		self.done = self.game.gameOver
		self.info = {}
		self.previousReward = 0
		self.reward = self.game.state.getScore()
		self.gameBoard = initGameBoard()
		self.state = stateGrid2Img(self.gameBoard, np.array(self.game.state.data.food.data), self.game.state.data.agentStates[0])
		return self.state

	def render(self, mode='human', close=False):
		cv2.imshow("Recman", self.state)
		cv2.waitKey(10)

	def get_action_meanings(self):
		return list(Actions._directions.keys())


def stateGrid2Img(gameBoard, foodData, pacman_pos):
	foodData = np.rot90(foodData)
	foodData = foodData[1:(foodData.shape[0]-1), 1:(foodData.shape[1]-1)]
	blue = [136, 28, 0]
	triangle_man = np.ones((11, 6, 3), np.uint8) * np.array(blue, dtype=np.uint8)
	pt1 = (2, 3)
	pt2 = (8, 0)
	pt3 = (8, 6)
	triangle = np.array( [pt1, pt2, pt3] )
	cv2.drawContours(triangle_man, [triangle], 0, (0,255,0), -1)
	pos = pacman_pos.getPosition()
	pos_row = 14 - pos[1]
	pos_col = pos[0]
	for row in range(2, 169, 12):
		for col in range(3, 149, 8):
			if not foodData[row//12][col//8]:
				if col > 76: col = col - 1
				gameBoard[2+row:13+row, 3+col:10+col] = blue
	off_set = 0
	if pos_col*7+7+off_set > 76:
		off_set = 1
	if pos_col*7+7+off_set > 93:
		off_set = 2
	if pos_col*7+7+off_set > 95:
		off_set = 3
	if pos_col*7+7+off_set > 97:
		off_set = 3
	if pos_col*7+7+off_set > 98:
		off_set = 3
	if pos_col*7+7+off_set > 105:
		off_set = 5
	if pos_col*7+7+off_set > 137:
		off_set = 6
	if pos_col*7+7+off_set > 140:
		off_set = 7
	gameBoard[pos_row*12+4:pos_row*12+15, pos_col*7+7+off_set:pos_col*7+13+off_set] = triangle_man #+ gameBoard[pos_row*12:pos_row*12+12, pos_col*8:pos_col*8+8]
	return gameBoard

def initGameBoard():
	# Background image
	img = np.zeros((210, 160, 3), dtype=np.uint8)
	orange = [111,111,228]
	blue = [136, 28, 0]
	img[1, :] = orange
	img[170:172, :] = orange
	img[1:171, 0:4] = orange
	img[1:171, 156:160] = orange
	img[2:171, 4:156] = blue
	# Single food image
	foodImg = np.zeros((12, 7, 3), dtype=np.uint8)
	foodImg[3:5, 2:6] = orange
	foodPattern = np.zeros((210, 160, 3), dtype=np.uint8)
	for i in range(2, 169, 12):
		for j in range(3, 149, 8):
			if j > 76: j = j - 2
			foodPattern[2+i:14+i, 3+j:10+j] = foodImg
	return img+foodPattern
