#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

'''
This file replays a dreyevr recording and produces intermediate data to train an LbC agent
'''

import glob
import os
import sys
import pathlib
import datetime
import copy

# try:
#     sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
#         sys.version_info.major,
#         sys.version_info.minor,
#         'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
# except IndexError:
#     pass

import carla

import argparse

import cv2
import numpy as np 

from DReyeVR_utils import find_ego_vehicle, DReyeVRSensor, DReyeVRSensorCallBack
from PIL import Image, ImageDraw
from common import COLOR, CONVERTER
from lbc_data_utils import get_nearby_lights, draw_traffic_lights

from leaderboard.utils.route_manipulation import downsample_route, interpolate_trajectory
from leaderboard.autoagents import autonomous_agent
from leaderboard.envs.sensor_interface import SensorInterface, CallBack, SpeedometerReader

from srunner.scenariomanager.carla_data_provider import *
from srunner.tools.route_parser import RouteParser
from srunner.scenariomanager.timer import GameTime

from team_code.planner import RoutePlanner
from agents.navigation.local_planner import RoadOption

class ReplayAgent(object):
    # TODO: inherit from map or other object?
    # this must be done for the high-level commands to be available for me.

    _sensors_list = []

    def __init__(self, args):        
        self.args = args
        self.client = carla.Client(args.host, args.port)
        # client = carla.Client('127.0.0.1', 2000)                 
        self.world = self.client.get_world() 
        self.replay_file = self.args.recorder_filename
        # load the appropriate world map so we can set sync replay if needed
        # see: https://github.com/carla-simulator/carla/issues/4144
        filename = pathlib.Path(self.replay_file)
        town_name = filename.stem[-2] # TODO hacky way to get the town the replay is in
        # other way may be to start the replay async, get the town, then restart in synchronous mode
        self.client.load_world('Town0'+town_name)
        
        CarlaDataProvider.set_client(self.client)
        CarlaDataProvider.set_world(self.world)
        
        self.ego_vehicle = find_ego_vehicle(self.world)

        
        
        self.path_to_dataset = pathlib.Path(args.output_dir) / \
                                pathlib.Path(self.replay_file).stem
        
        now =  str(datetime.datetime.now()).split(' ')
        now_str = now[0] + '_' + now[1]
        
        self.path_to_dataset = pathlib.Path(str(self.path_to_dataset) + '_' + str(now_str))
        (self.path_to_dataset / 'rgb').mkdir(parents=True)
        (self.path_to_dataset / 'rgb_left').mkdir(parents=True)
        (self.path_to_dataset / 'rgb_right').mkdir(parents=True)
        (self.path_to_dataset / 'topdown').mkdir(parents=True)
        (self.path_to_dataset / 'measurements').mkdir(parents=True)

        # init route planners for far and near
        self._read_route() # this calls self.set_global_plan()

        self._command_planner = RoutePlanner(7.5, 25.0, 257)
        self._command_planner.set_route(self._global_plan, True)

        self._waypoint_planner = RoutePlanner(4.0, 50)
        self._waypoint_planner.set_route(self._plan_gps_HACK, True)

        # this data structure will contain all def CARLA sensor data
        self.sensor_interface = SensorInterface()

        # other sensor for the dreyevr data
        self.dreyevr_sensor =  DReyeVRSensor(self.world)
        self.dreyevr_sensor.ego_sensor.listen(DReyeVRSensorCallBack(self.dreyevr_sensor))

        # timing stuff
        self.step = 0
        
        return

    def _destroy(self):
        """
        Remove and destroy all sensors
        """
        for i, _ in enumerate(self._sensors_list):
            if self._sensors_list[i] is not None:
                self._sensors_list[i].stop()
                self._sensors_list[i].destroy()
                self._sensors_list[i] = None
        self._sensors_list = []

    # Sensor interface code    
    def sensors(self):
        if self.args.sensor_config =="LBC":
            sensor_specs = \
            [
                    {
                        'type': 'sensor.camera.rgb',
                        'x': 1.3, 'y': 0.0, 'z': 1.3,
                        'roll': 0.0, 'pitch': 0.0, 'yaw': 0.0,
                        'width': 256, 'height': 144, 'fov': 90,
                        'id': 'rgb'
                        },
                    {
                        'type': 'sensor.camera.rgb',
                        'x': 1.2, 'y': -0.25, 'z': 1.3,
                        'roll': 0.0, 'pitch': 0.0, 'yaw': -45.0,
                        'width': 256, 'height': 144, 'fov': 90,
                        'id': 'rgb_left'
                        },
                    {
                        'type': 'sensor.camera.rgb',
                        'x': 1.2, 'y': 0.25, 'z': 1.3,
                        'roll': 0.0, 'pitch': 0.0, 'yaw': 45.0,
                        'width': 256, 'height': 144, 'fov': 90,
                        'id': 'rgb_right'
                        },
                    {
                        'type': 'sensor.other.imu',
                        'x': 0.0, 'y': 0.0, 'z': 0.0,
                        'roll': 0.0, 'pitch': 0.0, 'yaw': 0.0,
                        'sensor_tick': 0.05,
                        'id': 'imu'
                        },
                    {
                        'type': 'sensor.other.gnss',
                        'x': 0.0, 'y': 0.0, 'z': 0.0,
                        'roll': 0.0, 'pitch': 0.0, 'yaw': 0.0,
                        'sensor_tick': 0.01,
                        'id': 'gps'
                        },
                    {
                        'type': 'sensor.speedometer',
                        'reading_frequency': 20,
                        'id': 'speed'
                        },
                    {
                    'type': 'sensor.camera.semantic_segmentation',
                    'x': 0.0, 'y': 0.0, 'z': 100.0,
                    'roll': 0.0, 'pitch': -90.0, 'yaw': 0.0,
                    'width': 512, 'height': 512, 'fov': 5 * 10.0,
                    'id': 'map'
                        }
            ]
        elif self.args.sensor_config == "CILRS":
            sensor_specs = \
            [		
                {
					'type': 'sensor.camera.rgb',
					'x': 1.3, 'y': 0.0, 'z':2.3,
					'roll': 0.0, 'pitch': 0.0, 'yaw': 0.0,
					'width': 400, 'height': 300, 'fov': 100,
					'id': 'rgb'
					},
				{
					'type': 'sensor.camera.rgb',
					'x': 1.3, 'y': 0.0, 'z':2.3,
					'roll': 0.0, 'pitch': 0.0, 'yaw': -60.0,
					'width': 400, 'height': 300, 'fov': 100,
					'id': 'rgb_left'
					},
				{
					'type': 'sensor.camera.rgb',
					'x': 1.3, 'y': 0.0, 'z':2.3,
					'roll': 0.0, 'pitch': 0.0, 'yaw': 60.0,
					'width': 400, 'height': 300, 'fov': 100,
					'id': 'rgb_right'
					},
				{
					'type': 'sensor.camera.rgb',
					'x': -1.3, 'y': 0.0, 'z':2.3,
					'roll': 0.0, 'pitch': 0.0, 'yaw': -180.0,
					'width': 400, 'height': 300, 'fov': 100,
					'id': 'rgb_rear'
					},
				{
					'type': 'sensor.other.imu',
					'x': 0.0, 'y': 0.0, 'z': 0.0,
					'roll': 0.0, 'pitch': 0.0, 'yaw': 0.0,
					'sensor_tick': 0.05,
					'id': 'imu'
					},
				{
					'type': 'sensor.other.gnss',
					'x': 0.0, 'y': 0.0, 'z': 0.0,
					'roll': 0.0, 'pitch': 0.0, 'yaw': 0.0,
					'sensor_tick': 0.01,
					'id': 'gps'
					},
				{
					'type': 'sensor.speedometer',
					'reading_frequency': 20,
					'id': 'speed'
					},
                {
                    'type': 'sensor.camera.semantic_segmentation',
                    'x': 0.0, 'y': 0.0, 'z': 100.0,
                    'roll': 0.0, 'pitch': -90.0, 'yaw': 0.0,
                    'width': 512, 'height': 512, 'fov': 5 * 10.0,
                    'id': 'map'
                }
            ]
        else:
            print("not a valid sensor config")
            raise NotImplementedError 

        return sensor_specs
    
    def setup_sensors(self, debug_mode=False):
        """
        Create the sensors defined by the user and attach them to the ego-vehicle
        :param vehicle: ego vehicle
        :return:
        """
        bp_library = self.world.get_blueprint_library()
        for sensor_spec in self.sensors():
            # These are the pseudosensors (not spawned)
            if sensor_spec['type'].startswith('sensor.opendrive_map'):
                # The HDMap pseudo sensor is created directly here
                sensor = OpenDriveMapReader(self.ego_vehicle, sensor_spec['reading_frequency'])
            elif sensor_spec['type'].startswith('sensor.speedometer'):
                delta_time = CarlaDataProvider.get_world().get_settings().fixed_delta_seconds
                frame_rate = 1 / delta_time
                sensor = SpeedometerReader(self.ego_vehicle, frame_rate)
            # These are the sensors spawned on the carla world
            else:
                bp = bp_library.find(str(sensor_spec['type']))
                if sensor_spec['type'].startswith('sensor.camera'):
                    # print(sensor_spec['type'])
                    bp.set_attribute('image_size_x', str(sensor_spec['width']))
                    bp.set_attribute('image_size_y', str(sensor_spec['height']))
                    bp.set_attribute('fov', str(sensor_spec['fov']))
                    bp.set_attribute('lens_circle_multiplier', str(3.0))
                    bp.set_attribute('lens_circle_falloff', str(3.0))
                    if sensor_spec['type'].startswith('sensor.camera.rgb'):                         
                        bp.set_attribute('chromatic_aberration_intensity', str(0.5))
                        bp.set_attribute('chromatic_aberration_offset', str(0))

                    sensor_location = carla.Location(x=sensor_spec['x'], y=sensor_spec['y'],
                                                     z=sensor_spec['z'])
                    sensor_rotation = carla.Rotation(pitch=sensor_spec['pitch'],
                                                     roll=sensor_spec['roll'],
                                                     yaw=sensor_spec['yaw'])
                elif sensor_spec['type'].startswith('sensor.lidar'):
                    bp.set_attribute('range', str(85))
                    bp.set_attribute('rotation_frequency', str(10))
                    bp.set_attribute('channels', str(64))
                    bp.set_attribute('upper_fov', str(10))
                    bp.set_attribute('lower_fov', str(-30))
                    bp.set_attribute('points_per_second', str(600000))
                    bp.set_attribute('atmosphere_attenuation_rate', str(0.004))
                    bp.set_attribute('dropoff_general_rate', str(0.45))
                    bp.set_attribute('dropoff_intensity_limit', str(0.8))
                    bp.set_attribute('dropoff_zero_intensity', str(0.4))
                    sensor_location = carla.Location(x=sensor_spec['x'], y=sensor_spec['y'],
                                                     z=sensor_spec['z'])
                    sensor_rotation = carla.Rotation(pitch=sensor_spec['pitch'],
                                                     roll=sensor_spec['roll'],
                                                     yaw=sensor_spec['yaw'])
                elif sensor_spec['type'].startswith('sensor.other.radar'):
                    bp.set_attribute('horizontal_fov', str(sensor_spec['fov']))  # degrees
                    bp.set_attribute('vertical_fov', str(sensor_spec['fov']))  # degrees
                    bp.set_attribute('points_per_second', '1500')
                    bp.set_attribute('range', '100')  # meters

                    sensor_location = carla.Location(x=sensor_spec['x'],
                                                     y=sensor_spec['y'],
                                                     z=sensor_spec['z'])
                    sensor_rotation = carla.Rotation(pitch=sensor_spec['pitch'],
                                                     roll=sensor_spec['roll'],
                                                     yaw=sensor_spec['yaw'])

                elif sensor_spec['type'].startswith('sensor.other.gnss'):
                    bp.set_attribute('noise_alt_stddev', str(0.000005))
                    bp.set_attribute('noise_lat_stddev', str(0.00000005))
                    bp.set_attribute('noise_lon_stddev', str(0.00000005))
                    bp.set_attribute('noise_alt_bias', str(0.0))
                    bp.set_attribute('noise_lat_bias', str(0.0))
                    bp.set_attribute('noise_lon_bias', str(0.0))

                    sensor_location = carla.Location(x=sensor_spec['x'],
                                                     y=sensor_spec['y'],
                                                     z=sensor_spec['z'])
                    sensor_rotation = carla.Rotation()

                elif sensor_spec['type'].startswith('sensor.other.imu'):
                    bp.set_attribute('noise_accel_stddev_x', str(0.001))
                    bp.set_attribute('noise_accel_stddev_y', str(0.001))
                    bp.set_attribute('noise_accel_stddev_z', str(0.015))
                    bp.set_attribute('noise_gyro_stddev_x', str(0.001))
                    bp.set_attribute('noise_gyro_stddev_y', str(0.001))
                    bp.set_attribute('noise_gyro_stddev_z', str(0.001))

                    sensor_location = carla.Location(x=sensor_spec['x'],
                                                     y=sensor_spec['y'],
                                                     z=sensor_spec['z'])
                    sensor_rotation = carla.Rotation(pitch=sensor_spec['pitch'],
                                                     roll=sensor_spec['roll'],
                                                     yaw=sensor_spec['yaw'])
                # create sensor
                sensor_transform = carla.Transform(sensor_location, sensor_rotation)
                sensor = self.world.spawn_actor(bp, sensor_transform, attach_to=self.ego_vehicle, attachment_type=carla.AttachmentType.Rigid)
            # setup callback
            print("Setting up %s %s" %  (sensor_spec['id'], sensor_spec['type']))
            sensor.listen(CallBack(sensor_spec['id'], sensor_spec['type'], sensor, self.sensor_interface))
            self._sensors_list.append(sensor)
        
        # Tick once to spawn the sensors
        self.world.tick()

    def process_sensor_data(self, all_sensors_data):
        self.step+=1
        rgb = cv2.cvtColor(all_sensors_data['rgb'][1][:, :, :3], cv2.COLOR_BGR2RGB)
        rgb_left = cv2.cvtColor(all_sensors_data['rgb_left'][1][:, :, :3], cv2.COLOR_BGR2RGB)
        rgb_right = cv2.cvtColor(all_sensors_data['rgb_right'][1][:, :, :3], cv2.COLOR_BGR2RGB)
        gps = all_sensors_data['gps'][1][:2]
        speed = all_sensors_data['speed'][1]['speed']
        compass = all_sensors_data['imu'][1][-1]

        result = {
                'rgb': rgb,
                'rgb_left': rgb_left,
                'rgb_right': rgb_right,
                'gps': gps,
                'speed': speed,
                'compass': compass
                }

        # modify the semantic segmentation sensor
        _actors = self.world.get_actors()
        traffic_lights = get_nearby_lights(self.ego_vehicle, _actors.filter('*traffic_light*'))

        topdown = all_sensors_data['map'][1][:, :, 2] # input_data['map'][0] is just timestamp
        topdown = draw_traffic_lights(topdown, self.ego_vehicle, traffic_lights)
        result['topdown'] = topdown

        return result
    
    def save(self, far_node, near_command, steer, throttle, brake, target_speed, tick_data):
        """
        Save sensor data
        """
        if self.step < 10:
            return
        frame = self.step // 10
        # print(self.dreyevr_sensor.data)

        print("S/T/B inputs: ", steer, throttle, brake)
        print("High level command: ", near_command.value)
        pos = self._get_position(tick_data['gps'])
        theta = tick_data['compass']
        speed = tick_data['speed']

        data = {
                'x': pos[0],
                'y': pos[1],
                'theta': theta,
                'speed': speed,
                'target_speed': target_speed,
                'x_command': far_node[0],
                'y_command': far_node[1],
                'command': near_command.value,
                'steer': steer,
                'throttle': throttle,
                'brake': brake,
                }        
        
        save_path = self.path_to_dataset
        (save_path / 'measurements' / ('%04d.json' % frame)).write_text(str(data))

        Image.fromarray(tick_data['rgb']).save(save_path / 'rgb' / ('%04d.png' % frame))
        Image.fromarray(tick_data['rgb_left']).save(save_path / 'rgb_left' / ('%04d.png' % frame))
        Image.fromarray(tick_data['rgb_right']).save(save_path / 'rgb_right' / ('%04d.png' % frame))
        Image.fromarray(tick_data['topdown']).save(save_path / 'topdown' / ('%04d.png' % frame))
        return

    def convert_topdown_img(self, topdown_image, path_to_map):
        topdown_array = np.frombuffer(topdown_image.raw_data, dtype=np.dtype("uint8"))
        topdown_array = copy.deepcopy(topdown_array)
        topdown_array = np.reshape(topdown_array, (topdown_image.height, topdown_image.width, 4))
        # TODO why this channel? 
        topdown_slice = topdown_array[:,:,2]
        # _topdown = Image.fromarray(COLOR[CONVERTER[topdown_slice]]) # only for viz
        
        self.actors = self.world.get_actors()
        self._traffic_lights = get_nearby_lights(self.ego_vehicle, self.actors.filter('*traffic_light*'))
        topdown_slice = draw_traffic_lights(topdown_slice, self.ego_vehicle, self._traffic_lights)


        (path_to_map / 'topdown').mkdir(parents=True, exist_ok=True)
        Image.fromarray(topdown_slice).save(path_to_map / 'topdown' / ('%04d.png' % topdown_image.frame))
        # topdown_image.save_to_disk( str(path_to_map / ('/%.6d.jpg' % topdown_image.frame)) )
        return
            
    def start_replay(self):
        self.client.set_replayer_time_factor(self.args.time_factor)
        GameTime.restart()
        # set to ignore the hero vehicles or not
        self.client.set_replayer_ignore_hero(self.args.ignore_hero)
        if self.args.sync_replay:
            self.sync_replay()
            self.replayinit_timestep = self.dreyevr_sensor.data['timestamp']
        else:
            self.async_replay()
        
        self.setup_sensors()        

        return

    def async_replay(self):
        # replay the session
        replay_info_str = self.client.replay_file(self.args.recorder_filename,
         self.args.start, self.args.duration,
         self.args.camera, self.args.spawn_sensors)
        print(replay_info_str)
        for line in replay_info_str.split("\n"):
            if "Total time recorded" in line:
                self.replay_duration = float(line.split(": ")[-1])
                break
        return

    def sync_replay(self):
        # Set synchronous mode settings
        new_settings = self.world.get_settings()
        new_settings.synchronous_mode = True
        new_settings.fixed_delta_seconds = 1. / self.args.desired_fps
        self.world.apply_settings(new_settings) 

        # replay the session
        replay_info_str = self.client.replay_file(self.args.recorder_filename,
                self.args.start, 0,
                0, False)
        print(replay_info_str)
        for line in replay_info_str.split("\n"):
            if "Total time recorded" in line:
                self.replay_duration = float(line.split(": ")[-1])
                break
        
        self.world.tick()
        assert(self.world.get_settings().synchronous_mode)

        return

    def run_step(self):
        timestamp = self._update_timestep()        

        # update carla data
        all_sensors_data = self.sensor_interface.get_data()
        # print(self.ego_vehicle.get_velocity())
        tick_data = self.process_sensor_data(all_sensors_data)

        gps = self._get_position(tick_data['gps'])
        near_node, near_command = self._waypoint_planner.run_step(gps)
        far_node, far_command = self._command_planner.run_step(gps)

        # update dreyevr sensor data
        # TODO: need to make sure that the timing here is correct
        # first, gotta make sure that the dreyevr data is not stale
        # then, gotta make sure that it is the correct dreyevr tick wrt the sim tick
        # (this is esp relevant when we're using sync mode and dreyevr tick behaviour is undefined)
        dreyevr_data = self.dreyevr_sensor.data        
        steer = dreyevr_data['steering_input']
        throttle = dreyevr_data['throttle_input']
        brake = dreyevr_data['brake_input']
        print("EgoSensor spd", dreyevr_data)
        brake = dreyevr_data['brake_input']
        target_speed = dreyevr_data['speed'] # TODO what is this target speed supposed to be in LbC?
        # print("DReyeVR sensor ", dreyevr_data['timestamp'],
        #     dreyevr_data['steering_input'], dreyevr_data['throttle_input'],
        #     dreyevr_data['brake_input'])
        # steer, throttle, brake, target_speed = 1.0, 1.0, 0.0, 1.0        
        # if the replay is over, wxit
        if (dreyevr_data['timestamp'] - self.replayinit_timestep) > \
                self.replay_duration + 1 :
                raise KeyboardInterrupt # start exiting the replay

        if self.step % (self.args.desired_fps / 2.0) ==0: # want to save @ 2Hz
            self.save(far_node, near_command, 
                    steer, throttle, brake,
                    target_speed, tick_data)

        if self.args.sync_replay:            
            self.world.tick()
            # write on top of map
            # make sure all the near/far node stuff is correct

        else: # replay is asynchronous
            world_snapshot = self.world.wait_for_tick()   

        return

    # new internal interface functions
    def _update_timestep(self):
        timestamp = None
        if self.world:
            snapshot = self.world.get_snapshot()
            if snapshot:
                timestamp = snapshot.timestamp
        if timestamp:            
            # Update game time and actor information
            GameTime.on_carla_tick(timestamp)
            CarlaDataProvider.on_carla_tick()
        return timestamp

    def _update_commands(self):
        gps_normed = self._get_position(gps)
        # could replace this with DReyeVR code from replay?
        near_node, near_command = self._waypoint_planner.run_step(gps_normed)
        far_node, far_command = self._command_planner.run_step(gps_normed)

    def _scale_dryevevr_controls(steering_input, brake_input, throttle_input):
        return steering, brake, throttle
    
    # internals borrowed from BaseAgent
    def _get_position(self, gps):
        gps = (gps - self._command_planner.mean) * self._command_planner.scale
        return gps

    # internals borrowed from autonomous_agent.AutonomousAgent
    def _set_global_plan(self, global_plan_gps, global_plan_world_coord):
        """
        Set the plan (route) for the agent
        """
        ds_ids = downsample_route(global_plan_world_coord, 50)
        self._global_plan_world_coord = [(global_plan_world_coord[x][0], global_plan_world_coord[x][1]) for x in ds_ids]
        self._global_plan = [global_plan_gps[x] for x in ds_ids]

        self._plan_HACK = global_plan_world_coord
        self._plan_gps_HACK = global_plan_gps
        return

    def _read_route(self, debug_mode=False):
        """
        Update the input route, i.e. refine waypoint list, and extract possible scenario locations
        """
        route_config = RouteParser.parse_routes_file(self.args.route_file,
                        self.args.scenarios_file, self.args.route_id)
        # for use there's only one route
        route_config = route_config[0]

        # prepare route's trajectory (interpolate and add the GPS route) 
        gps_route, route = interpolate_trajectory(route_config.trajectory)
        # print(route)
        self.route = route

        # this will be the global plan for the replay agent    
        self._set_global_plan(gps_route, self.route)

        # Print route in debug mode
        if self.args.debug_mode:
            self._draw_all_waypoints(vertical_shift=0.1, persistency=50000.0)
        return

    # debug functions
    def _draw_all_waypoints(self, vertical_shift=1, persistency=50000):
        """
        Draw a list of waypoints at a certain height `vertical_shift`.
        """
        # TODO: change so only turns are shown
        world = self.world
        waypoints = self.route
        # Remove all Straight lines?
        for w in waypoints:
            wp = w[0].location + carla.Location(z=vertical_shift)

            size = 0.2
            if w[1] == RoadOption.LEFT:  # Yellow
                color = carla.Color(255, 255, 0)
            elif w[1] == RoadOption.RIGHT:  # Cyan
                color = carla.Color(0, 255, 255)
            elif w[1] == RoadOption.CHANGELANELEFT:  # Orange
                color = carla.Color(255, 64, 0)
            elif w[1] == RoadOption.CHANGELANERIGHT:  # Dark Cyan
                color = carla.Color(0, 64, 255)
            elif w[1] == RoadOption.STRAIGHT:  # Gray
                color = carla.Color(128, 128, 128)
            else:  # LANEFOLLOW
                color = carla.Color(0, 255, 0)  # Green
                size = 0.1  # hide markers when lane following

            world.debug.draw_point(wp, size=size, color=color, life_time=persistency)

        # draw start and end in RED and BLUE
        world.debug.draw_point(waypoints[0][0].location + carla.Location(z=vertical_shift), size=size,
                               color=carla.Color(0, 0, 255), life_time=persistency)
        world.debug.draw_point(waypoints[-1][0].location + carla.Location(z=vertical_shift), size=size,
                               color=carla.Color(255, 0, 0), life_time=persistency)
        return


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
        '-r', '--route-file',
        metavar='R',
        default="/scratch/abhijatb/Bosch22/LbC_DReyeVR/leaderboard/data/route55.xml",
        help='route filename (route55.xml)')
    argparser.add_argument(
        '--sensor-config',
        default="LBC",
        help='sensor spec to use (depends on downstream learning algorithm needs: LBC/CILRS)')
    argparser.add_argument(
        '--scenarios-file',
        metavar='S',
        default="/scratch/abhijatb/Bosch22/LbC_DReyeVR/leaderboard/data/dreyevr/town05_scenarios.json",
        help='scenarios filename (.json)')
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
        '--route-id',
        default="55",
        help='route ID (55)')
    argparser.add_argument(
        '--desired-fps',
        default=20,
        help='desired fps in sync mode (20)')          

    argparser.add_argument(
        '-i', '--ignore-hero',
        action='store_true',
        help='ignore hero vehicles')
    argparser.add_argument(
        '--spawn-sensors',
        action='store_true',
        help='spawn sensors in the replayed world')
    argparser.add_argument(
        '--sync-replay',
        action='store_true',
        help='run the replay in synchronous mode')
    argparser.add_argument(
        '--debug-mode',
        action='store_true',
        help='draw the route waypoints for debug mode')
    args = argparser.parse_args()

    try:
        replayer = ReplayAgent(args)
        
        replayer.start_replay()
        
        while True:
            replayer.run_step()
            # if args.sync_replay:
            #     replayer.world.tick()
            # else:
            #     world_snapshot = replayer.world.wait_for_tick()   

    finally:
        if replayer.ego_vehicle is not None:
            replayer._destroy()            
        pass


if __name__ == '__main__':

    try:
        main()
    except KeyboardInterrupt:
        pass
    finally:
        print('\ndone.')
