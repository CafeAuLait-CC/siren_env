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

class RexEnv(gym.Env):
	metadata = {'render.modes' : ['human']}

	def __init__(self):
		# create the game board
		args = readCommand( sys.argv[1:] ) # Get game components based on input
		args['pacman'] = "gymAgents"
		rules = ClassicGameRules(args['timeout'])
		beQuiet = True
		import textDisplay
		gameDisplay = textDisplay.NullGraphics()
		rules.quiet = True
		self.game = rules.newGame(args['layout'], args['pacman'], args['ghosts'], gameDisplay, beQuiet, args['catchExceptions'])
		self.game.numMoves = 0
		self.done = self.game.gameOver
		self.reward = self.game.state.getScore()
		self.info = {}
		self.action_space = spaces.Discrete(5)	# len(game.state.getLegalPacmanActions())
		self.observation_space = spaces.Box(0, 255, (210, 160, 3), dtype=np.uint8)
		self.gameBoard = initGameBoard()
		self.state = stateGrid2Img(self.gameBoard, np.array(self.game.state.data.food.data))
		# # create the game board -- Tic-Tac-Toe
		# self.state = []
		# for i in range(3):
		# 	self.state += [[]]
		# 	for j in range(3):
		# 		self.state[i] += ["-"]
		# self.counter = 0
		# self.done = 0
		# self.add = [0, 0]
		# self.reward = 0

	# def check(self):
	# 	if(self.counter<5):
	# 		return 0
	# 	for i in range(3):
	# 		if(self.state[i][0] != "-" and self.state[i][1] == self.state[i][0] and self.state[i][1] == self.state[i][2]):
	# 			if(self.state[i][0] == "o"):
	# 				return 1
	# 			else:
	# 				return 2
	# 		if(self.state[0][i] != "-" and self.state[1][i] == self.state[0][i] and self.state[1][i] == self.state[2][i]):
	# 			if(self.state[0][i] == "o"):
	# 				return 1
	# 			else:
	# 				return 2
	# 	if(self.state[0][0] != "-" and self.state[1][1] == self.state[0][0] and self.state[1][1] == self.state[2][2]):
	# 		if(self.state[0][0] == "o"):
	# 			return 1
	# 		else:
	# 			return 2
	# 	if(self.state[0][2] != "-" and self.state[0][2] == self.state[1][1] and self.state[1][1] == self.state[2][0]):
	# 		if(self.state[1][1] == "o"):
	# 			return 1
	# 		else:
	# 			return 2

	def step(self, action):
		action = list(Actions._directions.keys())[action]	# action format from number to string
		self.game.moveHistory.append((self.game.startingIndex, action))
		self.game.state = self.game.state.generateSuccessor(self.game.startingIndex, action)
		self.game.display.update(self.game.state.data)
		self.game.rules.process(self.game.state, self.game)
		self.game.numMoves += 1
		self.game.display.finish()
		self.state = stateGrid2Img(self.gameBoard, np.array(self.game.state.data.food.data))
		self.reward = self.game.state.getScore()
		self.done = self.game.gameOver
		return [self.state, self.reward, self.done, self.info]
		# if self.done == 1:
		# 	print("Game Over")
		# 	return [self.state, self.reward, self.done, self.info]
		# elif self.state[action//3][action%3] != "-":
		# 	print("Invalid Step")
		# 	return [self.state, self.reward, self.done, self.info]
		# else:
		# 	if self.counter % 2 == 0:
		# 		self.state[action//3][action%3] = "o"
		# 	else:
		# 		self.state[action//3][action%3] = "x"
		# 	self.counter += 1
		# 	if self.counter == 9:
		# 		self.done = 1
		# 	self.render()

		# 	win = self.check()
		# 	if win :
		# 		self.done = 1
		# 		print("Player ", win, " wins.", sep="", end="\n")
		# 		self.info[win-1] = 1
		# 		if win == 1:
		# 			self.reward = 100
		# 		else:
		# 			self.reset = -100
		#	return [self.state, self.reward, self.done, self.info]

	def reset(self):
		args = readCommand( sys.argv[1:] ) # Get game components based on input
		rules = ClassicGameRules(args['timeout'])
		beQuiet = True
		import textDisplay
		gameDisplay = textDisplay.NullGraphics()
		rules.quiet = True
		self.game = rules.newGame(args['layout'], args['pacman'], args['ghosts'], gameDisplay, beQuiet, args['catchExceptions'])
		self.game.numMoves = 0
		self.done = self.game.gameOver
		self.info = {}
		self.reward = self.game.state.getScore()
		self.gameBoard = initGameBoard()
		self.state = stateGrid2Img(self.gameBoard, np.array(self.game.state.data.food.data))
		return self.state
		# for i in range(3):
		# 	for j in range(3):
		# 		self.state[i][j] = "-"
		# self.counter = 0

	def render(self, mode='human', close=False):
		# self.game.state.data.agentStates[0].getPosition()	# current pacman position
		print(self.game.state)
		cv2.imshow("Recman - Score: " + str(self.reward), self.state)
		cv2.waitKey(10)
		cv2.destroyAllWindows()
		# for i in range(3):
		# 	for j in range(3):
		# 		print(self.state[i][j], end=" ")
		# 	print("")

	def get_action_meanings(self):
		return list(Actions._directions.keys())


def stateGrid2Img(gameBoard, foodData):
	foodData = np.rot90(foodData)
	foodData = foodData[1:(foodData.shape[0]-1), 1:(foodData.shape[1]-1)]
	blue = [136, 28, 0]
	for row in range(2, 169, 12):
		for col in range(3, 140, 8):
			if not foodData[row//12][col//8]:
				if col > 70: col = col + 4
				gameBoard[2+row:13+row, 3+col:10+col] = blue
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
		for j in range(3, 140, 8):
			if j > 70: j = j + 4
			foodPattern[2+i:14+i, 3+j:10+j] = foodImg
	return img+foodPattern


# import cv2
# import numpy as np

# image = np.ones((300, 300, 3), np.uint8) * 255

# pt1 = (150, 100)
# pt2 = (100, 200)
# pt3 = (200, 200)

# cv2.circle(image, pt1, 2, (0,0,255), -1)
# cv2.circle(image, pt2, 2, (0,0,255), -1)
# cv2.circle(image, pt3, 2, (0,0,255), -1)

# triangle_cnt = np.array( [pt1, pt2, pt3] )

# cv2.drawContours(image, [triangle_cnt], 0, (0,255,0), -1)

# cv2.imshow("image", image)
# cv2.waitKey()