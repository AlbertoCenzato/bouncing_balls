from typing import Tuple
import math, random

#import pygame
import Box2D

from .video_writer import Writer

# typedefs
Point2i = Tuple[int, int]
Point2f = Tuple[float, float]


def get_rand_vel(mean_vel: float) -> Tuple[float, float]:
    angle = 2 * math.pi * random.random()
    norm = random.gauss(mean_vel, mean_vel / 10)
    vx = math.cos(angle) * norm
    vy = math.sin(angle) * norm
    return vx, vy


class EnvironmentSimulator():

    # --- constants ---
    DEFAULT_FPS = 60

    class BodyData(object):

        def __init__(self, name, visible=True):
            self.name = name
            self.visible = visible

    def __init__(self, renderer, save_metadata=True):
        """ Instantiates a dynamic bouncing balls model. """
        self._fps = 0
        self._timeStep = 0
        self._bounding_box = None
        self._writer = None
        self._save_metadata = save_metadata
        self._metadata = []

        self._save = False
        self.fps = self.DEFAULT_FPS

        # --- pybox2D setup ---
        self._world = Box2D.b2World(gravity=(0, 0), doSleep=True)
        #self._clock = pygame.time.Clock()

        # --- pygame setup ---
        self._screenOutput = True
        self._renderer = renderer
        self._screenWidth_m = self._renderer.pixels_to_meters(self._renderer.screen_width_px)
        self._screenHeight_m = self._renderer.pixels_to_meters(self._renderer.screen_height_px)

        self._create_screen_bounding_box()


    def __enter__(self):
        return self


    def __exit__(self, exc_type, exc_value, traceback):
        self.quit()


    @property
    def fps(self) -> int:
        return self._fps

    @fps.setter
    def fps(self, fps: int) -> None:
        self._fps = fps
        self._timeStep = 1.0 / fps


    def get_rand_pos(self) -> Tuple[float, float]:
        return random.random() * self._renderer.screen_width_px, random.random() * self._renderer.screen_height_px


    def _create_screen_bounding_box(self) -> None:
        screen_center = (self._screenWidth_m / 2, self._screenHeight_m / 2)
        bottom_left   = (-screen_center[0], -screen_center[1])
        bottom_right  = ( screen_center[0], -screen_center[1])
        top_right     = ( screen_center[0],  screen_center[1])
        top_left      = (-screen_center[0],  screen_center[1])

        self._bounding_box = self._world.CreateStaticBody(
            position=screen_center,
            shapes=[Box2D.b2EdgeShape(vertices=[bottom_left,  bottom_right]),
                    Box2D.b2EdgeShape(vertices=[bottom_right, top_right]),
                    Box2D.b2EdgeShape(vertices=[top_right,    top_left]),
                    Box2D.b2EdgeShape(vertices=[top_left,     bottom_left])]
        )
        self._bounding_box.userData = EnvironmentSimulator.BodyData("world_bounding_box", visible=False)


    def enable_bounding_box(self, enable: bool) -> None:
        self._bounding_box.active = enable


    def suppress_output(self, no_output: bool) -> None:
        """ Runs the simulation at maximum allowable speed without visualization """
        self._renderer.is_visible = not no_output


    def save_to(self, path: str, writer: Writer) -> None:
        self._save = True
        self._writer = writer
        self._writer.FPS = self._fps
        self._writer.resolution = (self._renderer.screen_width_px, self._renderer.screen_height_px)
        self._writer.open(path)


    def add_line(self, p1: Point2f, p2: Point2f) -> None:
        p1 = self._renderer.to_world_frame(p1)
        p2 = self._renderer.to_world_frame(p2)
        m = ((p1[0] + p2[0]) / 2, (p1[1] + p2[1]) / 2)
        p1 = (p1[0] - m[0], p1[1] - m[1])  # compute p1 coordinates wrt m
        p2 = (p2[0] - m[0], p2[1] - m[1])  # compute p2 coordinates wrt m
        line = self._world.CreateStaticBody(position=m, shapes=[Box2D.b2EdgeShape(vertices=[p1, p2])])
        line.active = False
        line.userData = EnvironmentSimulator.BodyData("line", visible=False)
        print("Warning line rendering disabled!")


    # TODO: add rect type
    def add_rectangular_occlusion(self, rect) -> None:
        width_px, height_px = rect[2], rect[3]
        center_px = (rect[0] + width_px / 2, rect[1] + height_px / 2)
        width_m, height_m = self._renderer.pixels_to_meters(width_px), self._renderer.pixels_to_meters(height_px)
        occlusion = self._world.CreateStaticBody(position=self._renderer.to_world_frame(center_px))
        occlusion.CreatePolygonFixture(box=(width_m / 2, height_m / 2), density=0, friction=0, restitution=0)
        occlusion.active = False
        occlusion.userData = EnvironmentSimulator.BodyData("rectangular_occlusion")


    def add_circle(self, pos: Point2f, vel: Point2f, radius: float=5) -> None:
        pos = self._renderer.to_world_frame(pos)
        vel = (self._renderer.pixels_to_meters(vel[0]), -self._renderer.pixels_to_meters(vel[1]))
        circle = self._world.CreateDynamicBody(position=pos)
        
        fixture_def = Box2D.b2FixtureDef(
            shape=(Box2D.b2CircleShape(radius=radius)), 
            density=1, 
            friction=0, 
            restitution=1
        )

        circle.CreateFixture(fixture_def)
        circle.linearVelocity = vel
        circle.userData = EnvironmentSimulator.BodyData("circle")


    def add_rand_circle(self, mean_vel: float=5000, radius: float=5) -> None:
        position = self.get_rand_pos()
        velocity = get_rand_vel(mean_vel)
        self.add_circle(position, velocity, radius)


    def step(self) -> float:
        frame = self._renderer.get_frame(self._world)
        if self._save:
            self._writer.write(frame)
        if self._save_metadata:
            self._collect_metadata()

        # Make Box2D simulate the physics of our world for one step.
        # Instruct the world to perform a single step of simulation.
        self._world.Step(self._timeStep, 10, 10)

        # Try to keep at the target FPS
        #if self._renderer.is_visible:
        #    self._clock.tick(self._fps)
        #    return self._clock.get_time()

        # self.clock.tick()
        return self._timeStep * 1000


    def run_simulation(self, time_s: float) -> None:
        elapsed_ms = 0.0
        max_time_ms = time_s * 1000
        while elapsed_ms < max_time_ms:
            elapsed_ms += self.step()

        if self._save:
            self._writer.close()


    def reset(self) -> None:
        for body in self._world.bodies:
            self._world.DestroyBody(body)
        self._create_screen_bounding_box()
        if self._save:
            self._writer.close()
        #self._clock = pygame.time.Clock()
        self._renderer.reset()
        self._metadata = []


    def quit(self) -> None:
        #pygame.quit()
        if self._save:
            self._writer.close()

    def _collect_metadata(self) -> None:
        balls_coordinates = []
        for body in self._world.bodies:
            if body.userData.visible:
                # The fixture holds information like density, friction and shape.
                for fixture in body.fixtures:
                    shape = fixture.shape

                    if isinstance(shape, Box2D.b2CircleShape):
                        center = self._renderer.to_screen_frame(body.position)
                        balls_coordinates.append([center[0], center[1]])
        self._metadata.append(balls_coordinates)
