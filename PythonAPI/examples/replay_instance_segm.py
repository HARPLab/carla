#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import configparser
import glob
import os
import sys
import time
import numpy as np
from PIL import Image

try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla

import argparse

try:
    import queue
except ImportError:
    import Queue as queue

from DReyeVR_utils import DReyeVRSensor, find_ego_vehicle, find_ego_sensor

def ReplayStat(s):
    idx = s.find("ReplayStatus")
    if idx == -1:
        return 0
    print(s[idx + len("ReplayStatus=")])
    return (s[idx + len("ReplayStatus=")] == '1')


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
    argparser.add_argument(
        '-n', '--frame-number',
        default=0,
        type=int,
        help='number of frames in the recording')
    argparser.add_argument(
        '-parse', '--recorder-parse',
        metavar='R',
        default="test1.log",
        help='recorder parse file outputted after running show_recorder_file_info.py')
    argparser.add_argument(
        '-config', '--sensor-config',
        default = 'sensor_config.ini',
        help = "sensor configuration deatils for camera orientation"
    )
    args = argparser.parse_args()
    egosensor = None
    instseg_camera = None
    sensor_config = configparser.ConfigParser()
    sensor_config.read(args.sensor_config)

    try:

        client = carla.Client(args.host, args.port)
        client.set_timeout(60.0)

        world = client.get_world()
        
        # Configure carla simulation in sync mode.
        settings = world.get_settings()
        settings.synchronous_mode = True
        settings.fixed_delta_seconds = 0.05
        world.apply_settings(settings)
        world.tick()
        client.set_timeout(60.0)

        # set the time factor for the replayer
        client.set_replayer_time_factor(args.time_factor)

        # replay the session
        replay_info_str = client.replay_file(args.recorder_filename, args.start, args.duration, args.camera, args.spawn_sensors)
    
        blueprint_library = world.get_blueprint_library()
        
        import time
        time.sleep(2)
        
        ego_vehicle = find_ego_vehicle(world)
        egosensor = find_ego_sensor(world)
        ego_queue = queue.Queue()
        egosensor.listen(ego_queue.put)
        ReplayStatus = 0

        for line in replay_info_str.split("\n"):
            if "Total time recorded" in line:
                replay_duration = float(line.split(": ")[-1])
                break
                
        #Assigning the number of replay frames to run through
        total_replay_frames = 200
        if (args.frame_number is not 0):
            total_replay_frames = args.frame_number
        elif args.recorder_parse is not "test1.log":
            with open(args.recorder_parse, 'r') as file:
                for line in file:
                    if "Frames: " in line:
                        framesLine = line.strip()
                        total_replay_frames = int(framesLine.split(": ")[1])
                        print(total_replay_frames)
                        break

        # spawn sensors
        instseg_camera = world.spawn_actor(
            blueprint_library.find('sensor.camera.instance_segmentation'),
            carla.Transform(carla.Location(x=float(sensor_config['rgb']['x']), z=float(sensor_config['rgb']['z'])), carla.Rotation(pitch=float(sensor_config['rgb']['pitch']))),
            attach_to=ego_vehicle)

        instseg_queue = queue.Queue()
        instseg_camera.listen(instseg_queue.put)

        rgb_camera = world.spawn_actor(
            blueprint_library.find('sensor.camera.rgb'),
            carla.Transform(carla.Location(x=float(sensor_config['rgb']['x']), z=float(sensor_config['rgb']['z'])), carla.Rotation(pitch=float(sensor_config['rgb']['pitch']))),
            attach_to=ego_vehicle)
        rgb_queue = queue.Queue()
        rgb_camera.listen(rgb_queue.put)

        world.tick()

        replay_started = 0
        replay_done = False
        ctr = 0
        
        filename = os.path.basename(args.recorder_filename)
        filename = os.path.splitext(filename)[0]
        while not replay_done:
            # replay_frames = set()
            # while not ego_queue.empty():
            #     el = ego_queue.get(2.0)
            #     if el.replaystatus:
            #         replay_frames.add(el.frame)
            # import ipdb; ipdb.set_trace()
            if not instseg_queue.empty():
                image = instseg_queue.get(2.0)
                #image.save_to_disk('%s/images/instance_segmentation_output/%.6d.jpg' % (filename, ctr + 1))
                raw_image = Image.fromarray(np.array(image.raw_data).reshape(image.height, image.width, 4))
                #Image.fromarray(np.array(image.raw_data).reshape(600, 800, 4)).show()
                raw_image.save('%s/images/instance_segmentation_output/%.6d.png' % (filename, ctr + 1))
            if not rgb_queue.empty():
                image = rgb_queue.get(2.0)
                image.save_to_disk('%s/images/rgb_output/%.6d.png' % (filename, ctr + 1))

            world.tick()
            ctr += 1

            if ctr > total_replay_frames:
                replay_done = True
                break
            
        # check if replay has finished
        if replay_done:      
            if egosensor:
                egosensor.destroy()
            if instseg_camera:
                instseg_camera.destroy()
            if rgb_camera:
                rgb_camera.destroy()


    finally:
        pass
       

if __name__ == '__main__':

    try:
        main()
    except KeyboardInterrupt:
        pass
    finally:
        print('\ndone.')