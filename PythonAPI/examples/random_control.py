#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# Allows controlling a vehicle with a keyboard. For a simpler and more
# documented example, please take a look at tutorial.py.



from __future__ import print_function


# ==============================================================================
# -- find carla module ---------------------------------------------------------
# ==============================================================================


import glob
import os
import sys

try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass


# ==============================================================================
# -- imports -------------------------------------------------------------------
# ==============================================================================


import carla

from carla import ColorConverter as cc

import argparse
import collections
import datetime
import logging
import math
import random
import re
import weakref
import numpy as np
import cv2
from DReyeVR_utils import find_ego_vehicle


# ==============================================================================
# -- Global functions ----------------------------------------------------------
# ==============================================================================


def find_weather_presets():
    rgx = re.compile('.+?(?:(?<=[a-z])(?=[A-Z])|(?<=[A-Z])(?=[A-Z][a-z])|$)')
    name = lambda x: ' '.join(m.group(0) for m in rgx.finditer(x))
    presets = [x for x in dir(carla.WeatherParameters) if re.match('[A-Z].+', x)]
    return [(getattr(carla.WeatherParameters, x), name(x)) for x in presets]


def main():
    client = carla.Client('127.0.0.1', 2000)
    client.set_timeout(10.0)
    world = client.get_world()
    ego_vehicle = find_ego_vehicle(world)
    ego_transform = ego_vehicle.get_transform()

    # store sensor data to disk
    imu_bp = world.get_blueprint_library().find('sensor.other.imu')
    imu_location = carla.Location(0,0,0)
    imu_rotation = carla.Rotation(0,0,0)
    imu_transform = carla.Transform(imu_location, imu_rotation)
    imu_bp.set_attribute("sensor_tick",str(3.0))
    ego_imu = world.spawn_actor(imu_bp,imu_transform,attach_to=ego_vehicle, attachment_type=carla.AttachmentType.Rigid)
    def imu_callback(imu):
        print("IMU measure:\n"+str(imu)+'\n')
    ego_imu.listen(lambda imu: imu_callback(imu))

    # cam_bp = None
    # cam_bp = world.get_blueprint_library().find('sensor.camera.rgb')
    # cam_location = carla.Location(1.3,0,1.3)
    # cam_rotation = carla.Rotation(0,0,0)
    # cam_transform = carla.Transform(cam_location, cam_rotation)
    # cam_bp.set_attribute("image_size_x", str(256))
    # cam_bp.set_attribute("image_size_y", str(144))
    # cam_bp.set_attribute("fov", str(90))
    # ego_cam = world.spawn_actor(cam_bp,cam_transform,attach_to=ego_vehicle, attachment_type=carla.AttachmentType.Rigid)
    # ego_cam.listen(lambda data: decode_img(data))
    # ego_cam.listen(lambda image: image.save_to_disk('/scratch/Bosch22/tutorial/new_rgb_output/%.6d.jpg' % image.frame))

    # test control
    # control = carla.VehicleControl()
    # control.steer = 90.0
    # control.throttle = 1.0
    # control.brake = 0.0
    # control.hand_brake = False
    # control.manual_gear_shift = False
    # ego_vehicle.apply_control(control)

    while cv2.waitKey(1) != 'q':
        world_snapshot = world.wait_for_tick()

    # --------------
    # Destroy actors
    # --------------
    if ego_vehicle is not None:
        if ego_imu is not None:
            ego_imu.stop()
            ego_imu.destroy()
    print('\nNothing to be done.')

    return


if __name__ == '__main__':

    main()
