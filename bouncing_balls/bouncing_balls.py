import os
import multiprocessing, threading
import random
from typing import List

import numpy as np

from .environment_simulator import EnvironmentSimulator
from .render import Renderer, VideoRenderer, Channels
from .video_writer import Writer, BufferedBinaryWriter

from .utility_functions import random_pos_outside_rectangle, \
                               random_trajectory_through_rectangle


class Config:

    TRAIN = 'train'
    TEST  = 'test'
    VALID = 'validation'

    def __init__(self, sequences: int=0, sequence_len: int=0, occlusion: bool=False, 
                 balls: List[int]=[], data_dir: str='', balls_radius: int=5, 
                 mean_vel: int=5000, dof: int=2, screen_height: int=48, 
                 screen_width: int=64, channels_ordering: Channels=Channels.LAST, 
                 save_metadata: bool=True):
        self.sequences     = sequences
        self.sequence_len  = sequence_len
        self.occlusion     = occlusion
        self.balls         = balls
        self.balls_radius  = balls_radius
        self.data_dir      = data_dir if data_dir is not None else 'data'
        self.mean_vel      = mean_vel
        self.dof           = dof
        self.screen_height = screen_height
        self.screen_width  = screen_width
        self.save_metadata = save_metadata
        self.channels_ordering = channels_ordering



class BouncingBalls():

    SCREEN_WIDTH  = 64
    SCREEN_HEIGHT = 64
    CHANNEL_ORDERING = Channels.FIRST

    FPS = 60
    FILE_NAME = 'bouncing_balls_'
    FILE_EXTENSION = '.npy'

    def __init__(self):
        self._environment = None
        self._config = Config()
        self._writer = BufferedBinaryWriter()

        self._rectangle = (22, 19, 20, 10)


    @staticmethod
    def from_config(config: Config):
        bouncing_balls = BouncingBalls()
        bouncing_balls._config = config
        bouncing_balls.set_renderer(VideoRenderer(config.screen_height, config.screen_width, config.channels_ordering))

        return bouncing_balls       
        

    def generate(self, train_or_test, suppress_output: bool=True) -> None:
        dataset_dir = os.path.join(self._config.data_dir, train_or_test)
        if not os.path.exists(dataset_dir): os.mkdir(dataset_dir)
        file_path = os.path.join(dataset_dir, BouncingBalls.FILE_NAME)

        self._generate(dataset_dir, file_path, suppress_output)
        
        
      
    def _generate(self, dataset_dir: str, file_path: str, suppress_output: bool):
        with EnvironmentSimulator(self._renderer, self._config.save_metadata) as env:
            env.fps = BouncingBalls.FPS
            env.suppress_output(suppress_output)
            metadata = []
            for i in range(self._config.sequences):
                path = file_path + str(i) + BouncingBalls.FILE_EXTENSION
                env.save_to(path, self._writer)
                self._setup_environment(env)
                for _ in range(self._config.sequence_len):
                    env.step()
                if self._config.save_metadata:
                    metadata.append(env._metadata)
                env.reset()

            if self._config.save_metadata:
                np.save(os.path.join(dataset_dir, "metadata.npy"),np.array(metadata))
        


    def set_renderer(self, renderer: Renderer):
        self._renderer = renderer
        return self

    def _setup_environment(self, env: EnvironmentSimulator) -> None:
        balls = random.choice(self._config.balls)
        if self._config.occlusion:
            env.add_rectangular_occlusion(self._rectangle)
            for _ in range(balls):
                vel = np.random.normal(self._config.mean_vel, self._config.mean_vel // 10)
                position = random_pos_outside_rectangle(self._config.screen_height, self._config.screen_width, self._rectangle)
                (vx, vy) = random_trajectory_through_rectangle(position, self._rectangle)
                env.add_circle(position, (vx * vel, vy * vel))
        else:
            for _ in range(balls):
                if self._config.dof == 1:
                    env.add_circle(env.get_rand_pos(), (2 * 5000 * random.random() - 5000, 0))
                elif self._config.dof == 2:
                    env.add_rand_circle(mean_vel=self._config.mean_vel)
