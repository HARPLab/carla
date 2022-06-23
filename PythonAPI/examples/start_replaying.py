#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import glob
import os
import sys
import pathlib
import datetime
from DReyeVR_utils import find_ego_vehicle
from common import COLOR, CONVERTER

try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla

import argparse


import cv2
import numpy as np 

def decode_img(image):
    raw_image = np.array(image.raw_data)
    image_shape = raw_image.reshape((256, 144, 4))
    rgb_value = image_shape[:, :, :3]
    cv2.imshow("", rgb_value)
    cv2.waitKey(1)
    return rgb_value/255.0  # normalize



def main():

    argparser = argparse.ArgumentParser(
        description=__doc__)
    argparser.add_argument(
        '--host',
        metavar='H',
        default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    argparser.add_argument(
        '-o', '--output-dir',
        metavar='O',
        default="/scratch/abhijatb/Bosch22/dreyevr_recordings_sensors/",
        help='output directory to store sensor outputs (/scratch/abhijatb/Bosch22/dreyevr_recordings_sensors)')    
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to (default: 2000)')
    argparser.add_argument(
        '-s', '--start',
        metavar='S',
        default=0.0,
        type=float,
        help='starting time (default: 0.0)')
    argparser.add_argument(
        '-d', '--duration',
        metavar='D',
        default=0.0,
        type=float,
        help='duration (default: 0.0)')
    argparser.add_argument(
        '-f', '--recorder-filename',
        metavar='F',
        default="test1.log",
        help='recorder filename (test1.log)')
    argparser.add_argument(
        '-c', '--camera',
        metavar='C',
        default=0,
        type=int,
        help='camera follows an actor (ex: 82)')
    argparser.add_argument(
        '-x', '--time-factor',
        metavar='X',
        default=1.0,
        type=float,
        help='time factor (default 1.0)')
    argparser.add_argument(
        '-i', '--ignore-hero',
        action='store_true',
        help='ignore hero vehicles')
    argparser.add_argument(
        '--spawn-sensors',
        action='store_true',
        help='spawn sensors in the replayed world')
    args = argparser.parse_args()

    try:

        client = carla.Client(args.host, args.port)
        # client = carla.Client('127.0.0.1', 2000)
        client.set_timeout(60.0)

        # set the time factor for the replayer
        client.set_replayer_time_factor(args.time_factor)

        # set to ignore the hero vehicles or not
        client.set_replayer_ignore_hero(args.ignore_hero)

        # replay the session
        print(client.replay_file(args.recorder_filename, args.start, args.duration, args.camera, args.spawn_sensors))
        
        world = client.get_world() 
        ego_vehicle = find_ego_vehicle(world)
        ego_transform = ego_vehicle.get_transform()

        # get sensors during replay
        imu_bp = world.get_blueprint_library().find('sensor.other.imu')
        imu_location = carla.Location(0,0,0)
        imu_rotation = carla.Rotation(0,0,0)
        imu_transform = carla.Transform(imu_location, imu_rotation)
        imu_bp.set_attribute("sensor_tick",str(3.0))
        ego_imu = world.spawn_actor(imu_bp,imu_transform,attach_to=ego_vehicle, attachment_type=carla.AttachmentType.Rigid)
        def imu_callback(imu):
            print("IMU measure:\n"+str(imu)+'\n')
        ego_imu.listen(lambda imu: imu_callback(imu))        

        # control = carla.VehicleControl()
        # control.steer = 90.0
        # control.throttle = 0.0
        # control.brake = 1.0
        # ego_vehicle.apply_control(control)

        while True: # cv2.waitKey(1) != 'q':
            world_snapshot = world.wait_for_tick()


    finally:
        if ego_vehicle is not None
            if ego_cam is not None:
                ego_cam.stop()
                ego_cam.destroy()
            if ego_imu is not None:
                ego_imu.stop()
                ego_imu.destroy()
            
        pass


if __name__ == '__main__':

    try:
        main()
    except KeyboardInterrupt:
        pass
    finally:
        print('\ndone.')
