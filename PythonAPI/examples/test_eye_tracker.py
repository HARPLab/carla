#!/usr/bin/env python

from DReyeVR_sensor import DReyeVRSensor
from test_spawn_debug import DebugHelper, Point as Pt
import argparse
import numpy as np
from numpy import random
import time
import sys
import glob
import os
import copy

python_egg = glob.glob(os.getcwd() + '/../carla/dist/carla-*.egg')
try:
    # sourcing python egg file
    sys.path.append(python_egg[0])
    import carla  # works if the python egg file is properly sourced
    print("Successfully imported Carla and sourced .egg file")
except Exception as e:
    print("Error:", e)


# debugging colours
RED = "\033[1;31m"
BLUE = "\033[1;34m"
GREEN = "\033[0;32m"
RESET = "\033[0;0m"


def eye_tracker_test(world):
    # get carla world debug helper
    debug = DebugHelper(world.debug)

    def cmp_results(latestData, t0: float, t: float, origin: Pt, target: Pt, name: str):
        if(name not in DReyeVRSensor.EYES):
            print("Invalid eye gaze: %s" % name)
            exit(1)
        print("Results for", name)
        # EXPECTED gaze data
        gaze_expected = target - origin
        vergence_exp = np.linalg.norm(gaze_expected)  # magnitude
        # normalizing gaze ray by scaling by 1/magnitude (expected vergence)
        gaze_expected = gaze_expected / vergence_exp
        # debug.draw_arrow([origin, target], 0.01, 0.02, 1.0, None)
        print("Expected: T = [%.4f, %.4f]," % (t0, t), "Gaze =",
              gaze_expected, "& Verg. = %.4f" % vergence_exp, )
        # ACTUAL gaze data
        vergence_act = latestData.vergence
        t = latestData.timestamp_sranipal
        if(name == DReyeVRSensor.COMBO):
            gaze_actual = Pt(latestData.gaze_ray)
        elif(name == DReyeVRSensor.LEFT):
            gaze_actual = Pt(latestData.gaze_ray_left)
        else:
            gaze_actual = Pt(latestData.gaze_ray_right)
        print("Actual: T = %.4f," % t, "Gaze =", gaze_actual, "Verg. = %.4f" % vergence_act, )
        # output errors
        t_err = latestData.timestamp_sranipal - latestData.timestamp_carla
        gaze_err = gaze_actual - gaze_expected
        vergence_err = vergence_act - vergence_exp
        print("%sErrors: " % RED, "T = %.4fs" % t_err, "Gaze =", gaze_err,
              "Verg. = %.4f %s" % (vergence_err, RESET))

    # draw arrows in the 3D world pointing to locations that the user should look at (bottom of arrow)
    # have the user look at them from the default position, and then use this to verify if the data
    # collected is accurate

    def output_results(t0: float, target: Pt):
        # NOTE: vergence = magnitude of gaze ray
        global DReyeVR_data
        latestData = copy.deepcopy(DReyeVR_data)
        from scipy.spatial.transform import Rotation as R
        # NOTE: by default UE4 FRotators are {Pitch, Roll, Yaw} = {Y, X, Z} extrinsic rotations
        rot = R.from_euler('yxz', DReyeVR_data.hmd_rotation, degrees=True)  # in radians
        # compute origin points
        origin = latestData.hmd_location + rot.apply(latestData.eye_origin)
        origin_left = latestData.hmd_location + rot.apply(latestData.eye_origin_left)
        origin_right = latestData.hmd_location + rot.apply(latestData.eye_origin_right)
        # plot the expected and actual gaze data
        t = world.wait_for_tick().timestamp.elapsed_seconds
        cmp_results(latestData, t0, t, Pt(origin), target, DReyeVRSensor.COMBO)
        cmp_results(latestData, t0, t, Pt(origin_left), target, DReyeVRSensor.LEFT)
        cmp_results(latestData, t0, t, Pt(origin_right), target, DReyeVRSensor.RIGHT)
        print("\n")

    # print clean arrays
    np.set_printoptions(precision=4, sign=' ')

    # begin recording data
    print("Press 'r' in the simulator to begin recording data")
    while not DReyeVR_data.is_initialized:
        time.sleep(0.01)
    print("Sensor connected and beginning recording")

    # draw arrows (first sleep 2s)
    time.sleep(0.1)
    reference_pt = Pt([38.3, -3.4, 0.0])

    # height of arrow (1m)
    arrow_height = Pt([0, 0, 1.0])

    # time for an arrow to despawn (s)
    lifetime = 1.5

    pt_front = reference_pt + Pt([5, 0, 0.5])
    t0 = world.wait_for_tick().timestamp.elapsed_seconds
    debug.draw_arrow([pt_front + arrow_height, pt_front], 0.15, 0.2, lifetime, None)
    time.sleep(0.5)  # wait for a bit to let the user look at the arrow
    output_results(t0, pt_front)
    time.sleep(2.0)  # sleep 1s after arrow despawns

    pt_front_right = reference_pt + Pt([5, -1, 0.5])
    t0 = world.wait_for_tick().timestamp.elapsed_seconds
    debug.draw_arrow([pt_front_right + arrow_height,
                      pt_front_right], 0.15, 0.2, lifetime, None)
    time.sleep(0.5)
    output_results(t0, pt_front_right)
    time.sleep(2.0)  # sleep 1s after arrow despawns

    pt_front_left = reference_pt + Pt([5, 1, 0.5])
    t0 = world.wait_for_tick().timestamp.elapsed_seconds
    debug.draw_arrow([pt_front_left + arrow_height,
                      pt_front_left], 0.15, 0.2, lifetime, None)
    time.sleep(0.5)
    output_results(t0, pt_front_left)
    time.sleep(2.0)  # sleep 1s after arrow despawns


def main():
    argparser = argparse.ArgumentParser(
        description=__doc__)
    argparser.add_argument(
        '--host',
        metavar='H',
        default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to (default: 2000)')

    args = argparser.parse_args()

    client = carla.Client(args.host, args.port)
    client.set_timeout(10.0)
    sync_mode = False  # synchronous mode
    random.seed(int(time.time()))

    world = client.get_world()

    global DReyeVR_data
    DReyeVR_data = DReyeVRSensor()

    # spawn a DReyeVR sensor and begin listening
    eye_tracker = DReyeVR_data.spawn(world)

    def update_information(data):
        # is updated on the sensor's listen thread
        if data.vergence >= 0:  # valid data has vergence >= 0
            DReyeVR_data.update(data)

    # subscribe to DReyeVR sensor

    eye_tracker.listen(update_information)

    try:
        eye_tracker_test(world)
    finally:
        # TODO: fix bug where this does not actually remove the sensor from the simulation
        carla.command.DestroyActor(eye_tracker)

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
