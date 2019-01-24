import abc
from typing import Tuple, List
from enum import Enum

import numpy as np
import matplotlib.pyplot as plt

import Box2D
import cv2

from .utility_functions import Point, Point2f, Point2i, distance


class Channels(Enum):
    FIRST = 1
    LAST  = 2



def center_of_mass(point_list: List[Point2f]):
    center = np.zeros((2,))
    for point in point_list:
        center += point
    return center / len(point_list)


def draw_horizontal_line(image: np.array, y: int, x_begin: int, x_end: int,
                         max_x: int, max_y: int) -> None:
    if y < 0: 
        y = 0
    if y >= max_y:
        y = max_y - 1
    if x_begin < 0:
        x_begin = 0
    if x_begin >= max_x:
        x_begin = max_x - 1
    if x_end < 0:
        x_end = 0
    if x_end >= max_x:
        x_end = max_x - 1
    
    image[y, x_begin:x_end+1] = 1.0


def symmetry_points(image: np.array, x: int, y: int, 
                    x_0: int, y_0: int,
                    max_x: int, max_y: int) -> None:
    draw_horizontal_line(image, y_0+y, x_0-x, x_0+x, max_x, max_y)
    draw_horizontal_line(image, y_0-y, x_0-x, x_0+x, max_x, max_y)
    draw_horizontal_line(image, y_0+x, x_0-y, x_0+y, max_x, max_y)
    draw_horizontal_line(image, y_0-x, x_0-y, x_0+y, max_x, max_y)


def plot_circle(image: np.array, radius: int, x_0: int, y_0: int) -> None:
    max_x, max_y = image.shape[1], image.shape[0]
    x, y = 0, radius
    d = 5/4.0 - radius
    symmetry_points(image, x, y, x_0, y_0, max_x, max_y)
    #plt.imshow(image)
    while x < y:
        if d < 0:
            x += 1
            d += 2*x + 1
        else:
            x += 1
            y -= 1
            d += 2*(x-y) + 1
        symmetry_points(image, x, y, x_0, y_0, max_x, max_y)
        #plt.imshow(image)


class Renderer(object):
    """ This the abstract base class for rendering the simulated physical world. """

    # Box2D deals with meters, but we want to display pixels,
    # so define a conversion factor:
    PPM = 1.0  # pixels per meter

    def __init__(self, width: int, height: int):
        self.screen_width_px  = width
        self.screen_height_px = height
        self.screen = None
        self._visible = True

    @staticmethod
    def pixels_to_meters(pixels: int) -> float:
        return pixels / Renderer.PPM

    @staticmethod
    def meters_to_pixels(meters: float) -> float:
        return int(meters * Renderer.PPM)

    #@property
    #def is_visible(self):
    #    return self._visible

    #@is_visible.setter
    #def is_visible(self, visible):
    #    self._visible = visible

    def to_world_frame(self, point: Point2i) -> Point2f:
        return Renderer.pixels_to_meters(point[0]), Renderer.pixels_to_meters(self.screen_height_px - point[1])

    def to_screen_frame(self, point: Point2f) -> Point2i:
        return Renderer.meters_to_pixels(point[0]), self.screen_height_px - Renderer.meters_to_pixels(point[1])

    @abc.abstractmethod
    def get_frame(self, world: Box2D.b2World) -> np.array:
        pass

    @abc.abstractmethod
    def reset(self) -> None:
        pass


class VideoRenderer(Renderer):
    """ This class has the role of rendering the simulated physical world. """

    #COLOR_WHITE = (255, 255, 255, 255)
    #COLOR_BLACK = (0, 0, 0, 0)

    def __init__(self, width: int, height: int, channel_ordering: Channels):
        """ 
            Args:
             @width:  width (in pixels) of the rendered scene
             @height: heigth (in pixels) of the rendered scene
             @channel_ordering: Channel enum specifying if get_frame() returns
                                (channels, height, width) images or
                                (height, width, channels) images
        """
        super(VideoRenderer, self).__init__(width, height)
        self._channel_ordering = channel_ordering
        self._screen_shape = (1, height, width) if channel_ordering is Channels.FIRST else (height, width, 1)

    #@Renderer.is_visible.setter
    #def is_visible(self, visible: bool):
    #    if not visible:
    #        pygame.display.iconify()
    #    Renderer.is_visible.fset(self, visible)

    def get_frame(self, world: Box2D.b2World) -> np.array:
        screen = np.zeros(self._screen_shape)

        # Draw the world
        for body in world.bodies:
            # The body gives us the position and angle of its shapes
            if body.userData.visible:
                for fixture in body.fixtures:
                    # The fixture holds information like density, friction and shape
                    shape = fixture.shape

                    if isinstance(shape, Box2D.b2PolygonShape):
                        pass
                    elif isinstance(shape, Box2D.b2EdgeShape):
                        pass
                    elif isinstance(shape, Box2D.b2CircleShape):
                        center = self.to_screen_frame(body.position)
                        #draw_circle(screen, center, self.meters_to_pixels(shape.radius), 
                        #            self._channel_ordering)
                        plot_circle(screen[0,:,:], self.meters_to_pixels(shape.radius), center[0], center[1])

        return screen


    def reset(self) -> None:
        pass


class CentroidVideoRenderer(Renderer):

    def __init__(self, width: int, height: int):
        super(CentroidVideoRenderer, self).__init__(width, height)
        self.downsampling = 0.0


    def get_frame(self, world: Box2D.b2World) -> np.array:

        if self.downsampling == 0:
            self.downsampling = self._compute_downsampling_factor(world)

        self.reset()

        # Draw the world
        for body in world.bodies:
            # The body gives us the position and angle of its shapes
            if body.userData.visible:
                for fixture in body.fixtures:
                    # The fixture holds information like density and friction,
                    # and also the shape.
                    shape = fixture.shape

                    if isinstance(shape, Box2D.b2CircleShape):
                        center = self.to_screen_frame(body.position)
                        center = self._fit_in_screen(center)
                        self.screen[center[0], center[1]] = 255
                    elif isinstance(shape, Box2D.b2PolygonShape) or isinstance(shape, Box2D.b2EdgeShape):
                        vertices = [body.transform * v for v in shape.vertices]
                        center = self.to_screen_frame(center_of_mass(vertices))
                        center = self._fit_in_screen(center)
                        self.screen[center[0], center[1]] = 255

        return self.screen

    def _compute_downsampling_factor(self, world: Box2D.b2World) -> float:
        # for body in world.bodies:
        #  if body.userData.visible:
        #     for fixture in body.fixtures:
        #        shape = fixture.shape
        #        if isinstance(shape, Box2D.b2CircleShape):
        #           return 1.0 / self.meters_to_pixels(2*shape.radius)
        # raise LookupError("World contains no circle!")
        return 1.0 / 8.0

    def _fit_in_screen(self, point: Point2i) -> Point2i:
        x, y = point
        if x < 0:
            x = 0
        if x >= int(self.screen_height_px * self.downsampling):
            x = int(self.screen_height_px * self.downsampling) - 1
        if y < 0:
            y = 0
        if y >= int(self.screen_width_px * self.downsampling):
            y = int(self.screen_width_px * self.downsampling) - 1
        return x, y

    def to_screen_frame(self, point: Point2i) -> Point2i:
        point = super(CentroidVideoRenderer, self).to_screen_frame(point)
        return int(point[1] * self.downsampling), int(point[0] * self.downsampling)

    def reset(self) -> None:
        if self.downsampling == 0:
            return
        self.screen = np.zeros((int(self.screen_height_px * self.downsampling),
                                int(self.screen_width_px * self.downsampling)), dtype='uint8')


class PositionAndVelocityExtractor(Renderer):

    def __init__(self, width: int, height: int):
        super(PositionAndVelocityExtractor, self).__init__(width, height)
        self.screen = np.zeros((0, 4), dtype=np.float32)
        self.is_visible = False

    def get_frame(self, world: Box2D.b2World) -> np.array:
        """ FIXME! This function works only in some specific conditions """
        self.reset()

        body_index = 0
        num_of_objects = len(world.bodies) - 1
        self.screen = np.zeros((num_of_objects * 4,), dtype=np.float32)
        for body in world.bodies:
            # The body gives us the position and angle of its shapes
            if body.userData.visible:
                for fixture in body.fixtures:
                    # The fixture holds information like density, friction and shape.
                    shape = fixture.shape

                    if isinstance(shape, (Box2D.b2PolygonShape, Box2D.b2CircleShape)):
                        if isinstance(shape, Box2D.b2PolygonShape):
                            vertices = [self.to_screen_frame(body.transform * v) for v in shape.vertices]
                            position = center_of_mass(vertices)
                        else:
                            position = self.to_screen_frame(body.position)
                        velocity = body.linearVelocity
                        self.screen[body_index:body_index + 4] = np.array(
                            [position[0], position[1], velocity[0], velocity[1]])
                        body_index += 4

        return self.screen

    def reset(self) -> None:
        self.screen = np.zeros_like(self.screen)
