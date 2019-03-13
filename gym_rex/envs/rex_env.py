import gym
from gym import error, spaces, utils
from gym.utils import seeding
import numpy as np
from pacman import *

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
		self.done = self.game.gameOver
		self.reward = self.game.state.data.score
		self.info = {}
		self.action_space = spaces.Discrete(5)	# len(game.state.getLegalPacmanActions())
		self.observation_space = spaces.Box(0, 255, (210, 160, 3), dtype=np.uint8)
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
		self.game.moveHistory.append((self.game.startingIndex, action))
		self.game.state = self.game.state.generateSuccessor((self.game.startingIndex, action))
		self.game.display.update(self.game.state.data)
		self.game.rules.process(self.game.state, self.game)
		self.game.numMoves += 1
		self.game.display.finish()
		return [self.game.state, self.reward, self.done, self.info]	# state: grid to image
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
		self.done = self.game.gameOver
		self.info = {}
		self.reward = self.game.state.data.score
		return self.game.state 	# state: grid to image
		# for i in range(3):
		# 	for j in range(3):
		# 		self.state[i][j] = "-"
		# self.counter = 0

	def render(self, mode='human', close=False):
		game.state.data.agentStates[0].getPosition()	# current pacman position

		# for i in range(3):
		# 	for j in range(3):
		# 		print(self.state[i][j], end=" ")
		# 	print("")

