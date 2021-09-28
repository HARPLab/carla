#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import glob
import os
import sys
import math

try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla
import argparse
import os
SCENARIO_RUNNER_ROOT = os.environ.get('SCENARIO_RUNNER_ROOT')
import sys
sys.path.append(SCENARIO_RUNNER_ROOT)

from srunner.tools.route_parser import RouteParser
from srunner.tools.route_manipulation import interpolate_trajectory
from agents.navigation.local_planner import RoadOption

def get_3pt_angle(wp, wm, wn):
    '''
    :carla.Location wp:
    :carla.Location wm:
    :carla.Location wn:
    :return: angle subtended at wm by wp and wn
    '''
    import numpy as np

    vec_p = wp - wm
    vec_p = np.array([vec_p.x, vec_p.y])
    uvec_p = vec_p/np.linalg.norm(vec_p)

    vec_n = wn - wm
    vec_n = np.array([vec_n.x, vec_n.y])
    uvec_n = vec_n / np.linalg.norm(vec_n)

    # costheta = np.dot(uvec_p, uvec_n)
    ytan = np.linalg.norm(uvec_n - uvec_p)
    xtan = np.linalg.norm(uvec_n + uvec_p)

    try:
        # angle_bn_3pts = 2 * np.arctan2(ytan, xtan)
        angle_bn_3pts = 2 * np.arctan(ytan/xtan)
    except ValueError:
        print(vec_p, vec_n)
        print(uvec_p, uvec_n)
        # import sys; sys.exit(-1)

    angle_bn_3pts = abs(angle_bn_3pts)
    return angle_bn_3pts

def _draw_all_waypoints(world, waypoints, vertical_shift, persistency=-1):
    """
    Draw a list of waypoints at a certain height given in vertical_shift.
    """
    # TODO: change so only turns are shown
    # Remove all Straight lines?
    draw_skip = 0
    size = 0.1
    color = carla.Color(0, 0, 0)  # black

    for i, w in enumerate(waypoints):
        wm = waypoints[i][0].location + carla.Location(z=vertical_shift)
        size = 0.1
        world.debug.draw_point(wm, size=size, color=color, life_time=persistency)

    # draw start and end in RED and BLUE
    world.debug.draw_point(waypoints[0][0].location + carla.Location(z=vertical_shift), size=size,
                           color=carla.Color(0, 0, 255), life_time=persistency)
    world.debug.draw_point(waypoints[-1][0].location + carla.Location(z=vertical_shift), size=size,
                           color=carla.Color(255, 0, 0), life_time=persistency)

def _draw_waypoints(world, waypoints, vertical_shift, persistency=-1):
    """
    Draw a list of waypoints at a certain height given in vertical_shift.
    """
    # TODO: change so only turns are shown
    # Remove all Straight lines?
    route_curvecalc_skip = 3
    draw_skip = 1
    route_suppress_angle_limit = 1
    size_visible = 0.05
    size = size_visible

    color = carla.Color(0, 0, 0)  # Black

    for i, w in enumerate(waypoints[route_curvecalc_skip + 1:-(route_curvecalc_skip + 1):draw_skip]):
        i = i * draw_skip
        wm = waypoints[i][0].location + carla.Location(z=vertical_shift)
        size = 0.1
        if waypoints[i][1] == RoadOption.LEFT:  # Yellow
            color = carla.Color(255, 255, 0)
        elif waypoints[i][1] == RoadOption.RIGHT:  # Cyan
            color = carla.Color(0, 255, 255)
        elif waypoints[i][1] == RoadOption.CHANGELANELEFT:  # Orange
            color = carla.Color(255, 64, 0)
        elif waypoints[i][1] == RoadOption.CHANGELANERIGHT:  # Dark Cyan
            color = carla.Color(0, 64, 255)
        # elif waypoints[i][1] == RoadOption.STRAIGHT:  # Gray
        #     color = carla.Color(128, 128, 128)
        #     size = 0.0
        else:  # LANEFOLLOW
            color = carla.Color(0, 255, 0)  # Green
            wp = waypoints[i - route_curvecalc_skip - 1][0].location + carla.Location(z=vertical_shift)
            wn = waypoints[i + route_curvecalc_skip + 1][0].location + carla.Location(z=vertical_shift)
            angle = get_3pt_angle(wp, wm, wn)
            angle_deg = (180 / math.pi) * angle

            if (angle < route_suppress_angle_limit) or \
                    (angle_deg > (180 - route_suppress_angle_limit)):
                size = 0.0  # no need to draw this segment
                color = carla.Color(255, 0, 0)
            else:
                size = size_visible
                color = carla.Color(0, 255, 0)  # Green

        color = carla.Color(0, 0, 0)
        world.debug.draw_point(wm, size=size, color=color, life_time=persistency)

    # draw start and end in RED and BLUE
    world.debug.draw_point(waypoints[0][0].location + carla.Location(z=vertical_shift), size=size,
                           color=carla.Color(0, 0, 255), life_time=persistency)
    world.debug.draw_point(waypoints[-1][0].location + carla.Location(z=vertical_shift), size=size,
                           color=carla.Color(255, 0, 0), life_time=persistency)

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
        '-r', '--route',
        metavar='R',
        default=None,
        nargs='+', type=str,
        help='scenario route associated with this recording (default: None)')

    args = argparser.parse_args()

    try:

        client = carla.Client(args.host, args.port)
        client.set_timeout(60.0)

        # set the time factor for the replayer
        client.set_replayer_time_factor(args.time_factor)

        # set to ignore the hero vehicles or not
        client.set_replayer_ignore_hero(args.ignore_hero)

        # overlay the scenario runner route
        routes = args.route[0]
        scenario_file = args.route[1]
        single_route = None
        if len(args.route) > 2:
            single_route = args.route[2]

        route_configurations = RouteParser.parse_routes_file(routes, scenario_file, single_route)

        for config in route_configurations:
            world_annotations = RouteParser.parse_annotations_file(config.scenario_file)
            # prepare route's trajectory (interpolate and add the GPS route)
            world = client.get_world()
            _, route = interpolate_trajectory(world, config.trajectory)
            # potential_scenarios_definitions, _ = RouteParser.scan_route_for_scenarios(config.town, route, world_annotations)
            _draw_waypoints(world, waypoints=route,
                            vertical_shift=0.0, persistency=50000.0)

        # replay the session
        print(client.replay_file(args.recorder_filename,
                                 args.start, args.duration, 2))
    finally:
        pass


if __name__ == '__main__':

    try:
        main()
    except KeyboardInterrupt:
        pass
    finally:
        print('\ndone.')
