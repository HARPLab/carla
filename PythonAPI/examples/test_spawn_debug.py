#!/usr/bin/env python

import argparse
import threading
import numpy as np
from numpy import random
import time
import sys
import glob
import os

python_egg = glob.glob(os.getcwd() + '/../carla/dist/carla-*.egg')
try:
    # sourcing python egg file
    sys.path.append(python_egg[0])
    import carla  # works if the python egg file is properly sourced
finally:
    print("Successfully imported Carla and sourced .egg file")
    pass


class Point(np.ndarray):
    def __new__(cls, input_array=(0, 0)):
        obj = np.asarray(input_array).view(cls)
        return obj

    @property
    def x(self):
        return self[0]

    @property
    def y(self):
        return self[1]

    @property
    def z(self):
        # could be 2D point, in which case x is 0
        try:
            return self[2]
        except IndexError:
            return 0

    def __eq__(self, other):
        return np.array_equal(self, other)

    def __ne__(self, other):
        return not self.__eq__(other)

    def __iter__(self):
        for x in np.nditer(self):
            yield x.item()

    def dist(self, other):
        return np.linalg.norm(self - other)


class DebugHelper(object):
    """modified from from carla-ros-bridge/debug_helper.py"""

    def __init__(self, carla_debug_helper):
        self.debug = carla_debug_helper

    def draw_arrow(self, points: list, thickness: float, arrow_size: float, lifetime: float, color, debug: bool = False):
        if not len(points) == 2:
            print("Drawing arrow from points requires two points. Received {}".format(
                len(points)))
            return
        start = carla.Location(x=points[0].x,
                               y=-points[0].y,
                               z=points[0].z)
        end = carla.Location(x=points[1].x,
                             y=-points[1].y,
                             z=points[1].z)
        if debug:
            print("Draw Arrow from {} to {} (color: {}, lifetime: {}, thickness: {}, arrow_size: {})".format(
                start, end, color, lifetime, thickness, arrow_size))
        self.debug.draw_arrow(
            start,
            end,
            thickness=thickness,
            # color=color, # need to find the proper type first
            arrow_size=arrow_size,
            life_time=lifetime)

    def draw_points(self, points: list, size: float, lifetime: float, color, debug: bool = False):
        for point in points:
            location = carla.Location(x=point.x, y=-point.y, z=point.z)
            if debug:
                print("Draw Point {} (color: {}, lifetime: {}, size: {})".format(
                    location, color, lifetime, size))
            # could include color
            self.debug.draw_point(location, size=size, life_time=lifetime)

    def draw_line_strips(self, points: list, thickness: float, lifetime: float, color, debug: bool = False):
        if len(points) < 2:
            print(
                "Drawing line-strip requires at least two points. Received {}".format(len(points)))
            return
        last_point = None
        for point in points:
            if last_point:
                start = carla.Location(
                    x=last_point.x, y=-last_point.y, z=last_point.z)
                end = carla.Location(x=point.x, y=-point.y, z=point.z)
                if debug:
                    print("Draw Line from {} to {} (color: {}, lifetime: {}, thickness: {})".format(
                        start, end, color, lifetime, thickness))
                self.debug.draw_line(start,
                                     end,
                                     thickness=thickness,
                                     color=color,
                                     life_time=lifetime)
            last_point = point


def run_test(world):
    # get carla world debug helper
    debug = DebugHelper(world.debug)

    # draw arrows in the 3D world pointing to locations that the user should look at (bottom of arrow)
    # have the user look at them from the default position, and then use this to verify if the data
    # collected is accurate
    def expected(t0: float, p1: Point, p2: Point, draw_arrow: bool = False):
        gaze = p2 - p1
        # NOTE: vergence = magnitude of gaze ray
        exp_vergence = np.sqrt(
            (p2.x - p1.x)**2 + (p2.y - p1.y)**2 + (p2.z - p1.z)**2)
        # normalizing gaze ray by scaling by 1/magnitude (expected vergence)
        exp_gaze_dir_norm = gaze / exp_vergence
        if draw_arrow:
            debug.draw_arrow([p1, p2], 0.05, 0.1, 1.0, None)
        t = world.wait_for_tick().timestamp.elapsed_seconds
        print("T0=%.4f," % t0, "T=%.4f," % t, "Expecting gaze dir =",
              repr(exp_gaze_dir_norm), "and vergence = %.4f" % exp_vergence, )

    # print clean arrays
    np.set_printoptions(precision=4, sign=' ')

    # reference "zero" position of vehicle
    reference_pt = Point([38.3, -3.4, 0.0])

    # vector from vehicle reference to user's VR seat
    head_vec = Point([0.21, 0.0, 1.2])
    # debug.draw_points([reference_pt + head_vec], 1.0, 35.0, None)

    # wait 3s
    time.sleep(3)

    # spawn an arrow directly in front of (+5 in x) the player (spectator)
    p1 = reference_pt + Point([5, 0, 1.5])  # top of arrow is +1.5 from ref pt
    # bottom of arrow (pointing down) is +0.5 from ref pt
    p2 = reference_pt + Point([5, 0, 0.5])
    t0 = world.wait_for_tick().timestamp.elapsed_seconds
    debug.draw_arrow([p1, p2], 0.15, 0.2, 1.0, None)
    expected(t0, reference_pt + head_vec, p2, draw_arrow=True)
    time.sleep(2)  # wait 2s (1s after 1st arrow is drawn)

    # spawn an arrow to the front-right of (+5 in x, +3 in y) the player (spectator)
    p1 = reference_pt + Point([5, -2, 1.5])  # top of arrow is +1.5 from ref pt
    # bottom of arrow (pointing down) is +0.5 from ref pt
    p2 = reference_pt + Point([5, -2, 0.5])
    t0 = world.wait_for_tick().timestamp.elapsed_seconds
    debug.draw_arrow([p1, p2], 0.15, 0.2, 1.0, None)
    expected(t0, reference_pt + head_vec, p2, draw_arrow=True)
    time.sleep(2)  # wait 2s (1s after 1st arrow is drawn)

    # spawn an arrow to the front-left of (+5 in x, -3 in y) the player (spectator)
    p1 = reference_pt + Point([5, 2, 1.5])  # top of arrow is +1.5 from ref pt
    # bottom of arrow (pointing down) is +0.5 from ref pt
    p2 = reference_pt + Point([5, 2, 0.5])
    t0 = world.wait_for_tick().timestamp.elapsed_seconds
    debug.draw_arrow([p1, p2], 0.15, 0.2, 1.0, None)
    expected(t0, reference_pt + head_vec, p2, draw_arrow=True)
    time.sleep(2)  # wait 2s (1s after 1st arrow is drawn)

    # spawn a far away arrow at the start of the brick to the right
    p1 = Point([64, -13.4, 1.5])  # top of arrow is +1.5 from ref pt
    # bottom of arrow (pointing down) is +0.5 from ref pt
    p2 = Point([63, -13.9, 0.5])
    t0 = world.wait_for_tick().timestamp.elapsed_seconds
    debug.draw_arrow([p1, p2], 0.15, 0.2, 1.0, None)
    expected(t0, reference_pt + head_vec, p2, draw_arrow=True)
    time.sleep(2)  # wait 2s (1s after 1st arrow is drawn)
    # finished test


def main():
    argparser = argparse.ArgumentParser(description=__doc__)
    argparser.add_argument('--host',
                           metavar='H',
                           default='127.0.0.1',
                           help='IP of the host server (default: 127.0.0.1)')
    argparser.add_argument('-p', '--port',
                           metavar='P',
                           default=2000,
                           type=int,
                           help='TCP port to listen to (default: 2000)')

    args = argparser.parse_args()

    client = carla.Client(args.host, args.port)
    client.set_timeout(10.0)
    sync_mode = False  # synchronous mode
    random.seed(int(time.time()))

    try:
        world = client.get_world()

        tests = threading.Thread(target=run_test(world))

        # start the test
        tests.start()

    finally:

        if sync_mode:
            settings = world.get_settings()
            settings.synchronous_mode = False
            settings.fixed_delta_seconds = None
            world.apply_settings(settings)


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        pass
    finally:
        print('\ndone.')
