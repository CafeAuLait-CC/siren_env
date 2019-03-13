from gym.envs.registration import register
register(
	id='rex-v0',
	entry_point='gym_rex.envs:RexEnv',
)
