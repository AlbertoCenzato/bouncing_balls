#!/usr/bin/env python
# -*- coding: utf-8 -*-
import math, os
import argparse
from typing import Union, Tuple

import numpy as np
from random import random

from .bouncing_balls import BouncingBalls, Config
from .render import Channels
from .utility_functions import exists_and_isdir


def generate_dataset(config: Config, train_or_test: str, suppress_output: bool) -> None:
    bouncing_balls = BouncingBalls.from_config(config)
    bouncing_balls.generate(train_or_test, suppress_output)


def generate_data(config: Config, suppress_output: bool=True) -> None:
    """ 
        Generates a dataset with the parameters specified by config which is 
        a Config object. 
        
        Args:
         - config: a Config object specifing how to build the dataset
         - suppress_output: if False the sequences are showed while they are
                            rendered (default True) 
    """

    if exists_and_isdir(config.data_dir):  # if dataset already generated do nothing
        train_dir = os.path.join(config.data_dir, Config.TRAIN)
        test_dir  = os.path.join(config.data_dir, Config.TEST)
        valid_dir = os.path.join(config.data_dir, Config.VALID)
        if exists_and_isdir(train_dir) and exists_and_isdir(test_dir) and exists_and_isdir(valid_dir):
            if len(os.listdir(train_dir)) == config.sequences and len(os.listdir(test_dir))  == config.sequences // 10 and len(os.listdir(valid_dir)) == config.sequences // 10:
                return
    else:
        os.mkdir(config.data_dir)

    print("Generating training set...")
    generate_dataset(config, Config.TRAIN, suppress_output)
    print("Done!")
    print("Generating validation set...")
    config.sequences = config.sequences // 10
    generate_dataset(config, Config.TEST, suppress_output)
    os.rename(os.path.join(config.data_dir, Config.TEST), os.path.join(config.data_dir, 'validation'))
    print("Done!")
    print("Generating testing set...")
    generate_dataset(config, Config.TEST, suppress_output)
    print("Done!")



if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Generates bouncing balls dataset.')
    parser.add_argument('balls', required=True, type=int, nargs='+', help='number of balls')
    parser.add_argument('--sequence_len', required=True, type=int, help='number of frames in the sequence')
    parser.add_argument('--sequences', required=True, type=int, help='number of sequences to generate')
    parser.add_argument('--balls_radius', type=int, default=5, help='radius of the bouncing balls')
    parser.add_argument('--occlusion', action='store_true', help='if true puts an occlusion at the center of the screen')
    parser.add_argument('--data_dir', help='output directory')
    parser.add_argument('--height', type=int, default=48, help='image height')
    parser.add_argument('--width', type=int, default=64, help='image width')
    parser.add_argument('--channels_last', action='store_true', help='if true the frames have shape \
                        (height, width, channels), otherwise they have shape (channels, height, width)')
    parser.add_argument('--verbose', action='store_true', help='activates verbose mode showing the produced sequences')

    args = parser.parse_args()
    
    channels_ordering = Channels.LAST if args.channels_last else Channels.FIRST
    config = Config(args.sequences, args.sequence_len, args.occlusion, args.balls, args.data_dir, 
                    screen_height=args.height, screen_width=args.width, channels_ordering=channels_ordering)
    
    generate_data(config, suppress_output=not args.verbose)
