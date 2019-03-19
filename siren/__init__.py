from gym.envs.registration import register

register(
	id='Siren-v0',
	entry_point='siren.envs:SirenEnv',
)
