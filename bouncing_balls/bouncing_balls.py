import os
import time
import multiprocessing

from typing import List, Union
from random import random

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
                 balls: Union[List[int], int]=0, data_dir: str='', balls_radius: int=5, 
                 mean_vel: int=5000, dof: int=2, screen_height: int=48, 
                 screen_width: int=64, channels_ordering: Channels=Channels.FIRST, 
                 save_metadata: bool=True):
        self.sequences     = sequences
        self.sequence_len  = sequence_len
        self.occlusion     = occlusion
        self.balls         = balls if isinstance(balls, list) else [balls]
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
        if multiprocessing.get_start_method() != 'spawn':
            multiprocessing.set_start_method('spawn')

        self._environment = None
        self._config = Config()

        self._rectangle = (22, 19, 20, 10)


    @staticmethod
    def from_config(config: Config):
        bouncing_balls = BouncingBalls()
        bouncing_balls._config = config
        bouncing_balls.set_renderer(config.screen_height, config.screen_width, config.channels_ordering) \
                      .set_n_proc(multiprocessing.cpu_count())

        return bouncing_balls       
        

    def generate(self, train_or_test, suppress_output: bool=True) -> None:
        self._config.save_metadata = False  # temporaly disabling metadata because it's not thread-safe

        dataset_dir = os.path.join(self._config.data_dir, train_or_test)
        if not os.path.exists(dataset_dir): os.mkdir(dataset_dir)
        file_path = os.path.join(dataset_dir, BouncingBalls.FILE_NAME)

        per_process_sequences = self._config.sequences // self._n_proc
        print('Per-process sequences: {}'.format(per_process_sequences))
        mod = self._config.sequences % self._n_proc
        begin = 0
        processes = []
        print('Using {} processes'.format(self._n_proc))
        for proc in range(self._n_proc):
            end = begin + per_process_sequences
            if proc < mod:
                end += 1
            print('Process {}: generates from {} to {}'.format(proc, begin, end-1))
            process = multiprocessing.Process(
                target=self.generate_batch, 
                args=(dataset_dir, file_path, begin, end, suppress_output)
            )
            processes.append(process)
            begin = end

        for i, process in enumerate(processes):
            print('Starting process {}'.format(i))
            process.start()

        for i, process in enumerate(processes):
            print('Waiting to join process {}'.format(i))
            process.join()
        
        
    def generate_batch(self, dataset_dir: str, file_path: str,
                       begin: int, end: int, suppress_output: bool) -> None:
        renderer = VideoRenderer(self._config.screen_width, 
                                 self._config.screen_height,
                                 self._config.channels_ordering)
        writer = BufferedBinaryWriter()
        with EnvironmentSimulator(renderer, self._config.save_metadata) as env:
            env.fps = BouncingBalls.FPS
            env.suppress_output(suppress_output)
            metadata = []
            for i in range(begin, end):
                path = file_path + str(i) + BouncingBalls.FILE_EXTENSION
                env.save_to(path, writer)
                self._setup_environment(env)
                for _ in range(self._config.sequence_len):
                    env.step()
                if self._config.save_metadata:
                    metadata.append(env._metadata)
                env.reset()
            if self._config.save_metadata:
                np.save(os.path.join(dataset_dir, "metadata.npy"),np.array(metadata))


    def set_renderer(self, screen_height: int, screen_width: int, channel_ordering: Channels):
        self._config.screen_height = screen_height
        self._config.screen_width  = screen_width
        self._config.channels_ordering = channel_ordering
        return self

    def set_n_proc(self, n_proc: int):
        self._n_proc = n_proc
        return self


    def _setup_environment(self, env: EnvironmentSimulator) -> None:
        if self._config.occlusion:
            env.add_rectangular_occlusion(self._rectangle)
            for _ in self._config.balls:
                vel = np.random.normal(self._config.mean_vel, self._config.mean_vel // 10)
                position = random_pos_outside_rectangle(self._config.screen_height, self._config.screen_width, self._rectangle)
                (vx, vy) = random_trajectory_through_rectangle(position, self._rectangle)
                env.add_circle(position, (vx * vel, vy * vel))
        else:
            for _ in self._config.balls:
                if self._config.dof == 1:
                    env.add_circle(env.get_rand_pos(), (2 * 5000 * random() - 5000, 0))
                elif self._config.dof == 2:
                    env.add_rand_circle(mean_vel=self._config.mean_vel)
