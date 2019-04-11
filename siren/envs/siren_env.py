import gym
from gym import error, spaces, utils
from gym.utils import seeding
import numpy as np
import cv2
import sys
from os import path

dir = path.dirname(__file__)
sys.path.append(dir)

from src_cpp import sirencontroller as sc

class SirenEnv(gym.Env):
	metadata = {'render.modes' : ['human']}

	def __init__(self):
		# create the chess board
		self.board = sc.BoardState()

		# required values for gym
		rgbChannel = np.array(self.board.getCurrentState())
		alpha = np.array(self.board.getAlpha())
		miniMap = np.array(self.board.getMiniMap())
		lastTwoChannel = np.dstack((alpha, miniMap))
		self.state = np.dstack((rgbChannel, lastTwoChannel))	# five channel as input
		self.reward = self.board.getReward() + self.board.getMiniMapReward()
		self.done = self.board.isDone()
		self.info = {}

		# additional informations
		self.isTraining = True
		self.numMoves = 0
		self.previousReward = 0
		self.action_space = spaces.Discrete(9)	# len(self.unwrapped.get_action_meanings())
		self.observation_space = spaces.Box(0, 255, (200, 200, 5), dtype=np.uint8)
		

	def step(self, action):
		action = list(self.board.getActionList())[action]
		# legalAction = self.board.getLegalActions()
		self.numMoves += 1

		# if action not in legalAction:
		# 	return [self.state, -50, self.done, self.info]

		self.previousReward = self.reward

		
		rgbChannel = np.array(self.board.getNextState(action, self.isTraining))
		alpha = np.array(self.board.getAlpha())
		miniMap = np.array(self.board.getMiniMap())
		lastTwoChannel = np.dstack((alpha, miniMap))
		self.state = np.dstack((rgbChannel, lastTwoChannel))
		self.reward = self.board.getReward() + self.board.getMiniMapReward()
		self.done = self.board.isDone()
		self.info = {}

		rewardChanged = self.reward - self.previousReward
		# if rewardChanged < 0:
		# 	rewardChanged = 0
		if self.done:
			if self.numMoves < 10000:
				self.info['info'] = "Finished by environment. Maybe due to illegal action."
		if self.numMoves > 1000:
			self.done = True
			self.info['info'] = 'Failed to complete. Too many moves.'
			self.info['numMoves'] = self.numMoves
		# if self.reward < -500:
		# 	self.done = True
		# 	self.info['info'] = "Too many wrong movements"
		return [self.state, rewardChanged, self.done, self.info]

	def reset(self):
		if len(self.info) > 0:
			self.board.reset(False)
		else:
			self.board.reset(True)

		rgbChannel = np.array(self.board.getCurrentState())
		alpha = np.array(self.board.getAlpha())
		miniMap = np.array(self.board.getMiniMap())
		lastTwoChannel = np.dstack((alpha, miniMap))
		self.state = np.dstack((rgbChannel, lastTwoChannel))
		self.reward = self.board.getReward() + self.board.getMiniMapReward()
		self.done = self.board.isDone()
		self.info = {}

		self.numMoves = 0
		self.previousReward = 0

		return self.state

	def render(self, mode='human', close=False, name = ""):
		imagery = np.array(self.board.getCurrentState())
		route = np.array(self.board.getAlpha())
		imagery[route==255] = [0, 255, 255]
		if name == "":
			cv2.imshow("Test Agent", imagery)
		else:
			cv2.imshow("Training - " + name, imagery)
		# cv2.imshow("Test Agent - Mini Map", np.array(self.board.getMiniMap()))
		cv2.waitKey(50)

	def get_action_meanings(self):
		return list(self.board.getActionList())
