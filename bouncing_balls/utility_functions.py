import os
import math
from random import random
from typing import Tuple, Union, List

import numpy as np


# typedefs
ArrayLike = Union[np.array, list, tuple]
Point2i = Tuple[int, int]
Point2f = Tuple[float, float]
Point = Union[Point2i, Point2f]



def exists_and_isdir(path: str) -> bool:
    return os.path.exists(path) and os.path.isdir(path)

def get_angle(vect1: ArrayLike, vect2: ArrayLike) -> float:
    """ Returns the angle (in radians) between two given vectors """
    return math.acos(np.dot(vect1, vect2) / (np.linalg.norm(vect1) * np.linalg.norm(vect2)))


def random_trajectory_through_rectangle(init_pos: ArrayLike, rect: ArrayLike) -> Tuple[float, float]:
    """ 
        Returns a random unit norm vector defining the trajectory
        that starts from point initPos and intersects with the rectangle rect
    """

    min_x, max_x = rect[0] + 10, rect[0] + rect[2] - 10
    min_y, max_y = rect[1] + 10, rect[1] + rect[3] - 10
    vertices = [(min_x, min_y),
                (min_x, max_y),
                (max_x, max_y),
                (max_x, min_y)]

    if init_pos[0] < min_x:
        if init_pos[1] < min_y:
            vertex1, vertex2 = vertices[1], vertices[3]
        elif init_pos[1] > max_y:
            vertex1, vertex2 = vertices[0], vertices[2]
        else:
            vertex1, vertex2 = vertices[0], vertices[1]
    elif init_pos[0] > max_x:
        if init_pos[1] < min_y:
            vertex1, vertex2 = vertices[0], vertices[2]
        elif init_pos[1] > max_y:
            vertex1, vertex2 = vertices[1], vertices[3]
        else:
            vertex1, vertex2 = vertices[2], vertices[3]
    else:
        if init_pos[1] < min_y:
            vertex1, vertex2 = vertices[0], vertices[3]
        elif init_pos[1] > max_y:
            vertex1, vertex2 = vertices[1], vertices[2]

    vect1 = (vertex1[0] - init_pos[0], vertex1[1] - init_pos[1])  # work with a coordinate frame centered in initPos
    vect2 = (vertex2[0] - init_pos[0], vertex2[1] - init_pos[1])  # work with a coordinate frame centered in initPos
    alpha1 = get_angle((1, 0), vect1)
    if vect1[1] < 0:
        alpha1 = -alpha1
    alpha2 = get_angle((1, 0), vect2)
    if vect2[1] < 0:
        alpha2 = -alpha2
    alpha12 = get_angle(vect1, vect2)

    rand_angle = random() * alpha12
    if alpha1 > alpha2:
        alpha1, alpha2 = alpha2, alpha1
    if alpha1 < -math.pi / 2 and alpha2 > math.pi / 2:
        rand_angle -= alpha1
    else:
        rand_angle += alpha1

    return (math.cos(rand_angle), math.sin(rand_angle))


def random_pos_outside_rectangle(screen_height: int, screen_width: int, 
                                 rect: ArrayLike) -> Tuple[float, float]:
    """ 
        Returns a random position, (x,y) tuple, such that (x,y)
        is not a point inside the rectangle area
    """
    assert screen_height > 0
    assert screen_width  > 0

    tol = 3
    x = random() * screen_width
    if rect[0] - tol < x < rect[0] + rect[2] + tol:
        empty_interval_y = screen_height - rect[3] - 2 * tol
        y = random() * empty_interval_y
        if y > rect[1] - tol:
            y += rect[3] + 2 * tol
    else:
        y = random() * screen_height
    return x, y


def distance(p1: Point, p2: Point) -> float:
    return math.sqrt((p1[0]-p2[0])**2 + (p1[1]-p2[1])**2)


def check_generated_data(path: str) -> List[Tuple[bool, str]]:
    error_list = []

    # check dataset root directory exists
    if not (os.path.exists(path) and os.path.isdir(path)):
        error_list.append((False, '{} does not exist or is not a folder'.format(path)))

    # check dataset subdir
    dirs = [os.path.join(path, d) for d in ['train', 'test', 'validation']]
    for dir in dirs:

        # check if dataset subdir exists
        if not (os.path.exists(dir) and os.path.isdir(dir)):
            error_list.append((False, '{} does not exist or is not a folder'.format(dir)))

        # check if dir is empty
        files = os.listdir(dir)
        if len(files) == 0:
            error_list.append((False, '{} is empty'.format(dir)))

        for file in files:
            file_path = os.path.join(dir,file)
            image = np.load(file_path)
            if not image.any():
                error_list.append((False, '{} is an empty sequence'.format(file_path)))

    return error_list
        


    
