#!/usr/bin/env python

import argparse
from DReyeVR_sensor import DReyeVRSensor
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
    print("Successfully imported Carla and sourced .egg file")
except Exception as e:
    print("Error:", e)


def to_msg_vec(vector, vec_name, delim, is_Vec2D=False):
    # get variable name from string and add it to msg from data delimited
    s = vec_name + "={"
    s += str(vector.x) + ","
    s += str(vector.y) + ("," if not is_Vec2D else "")
    # note that geom/Vector2D only have an x and y component
    if(not is_Vec2D):
        s += str(vector.z)
    s += "}" + delim
    return s


def to_msg(value, name, delim):
    return name + "=" + str(value) + delim


def CreateDReyeVRMsg(dreyevr: DReyeVRSensor, delim="; "):
    # TODO: need to make this work with custom ROS types
    s = "rosT=" + str(rospy.get_time()) + delim
    # fill in values for the DReyeVREvent msg
    s += to_msg(dreyevr.timestamp_carla, "t_carla", delim)
    s += to_msg(dreyevr.timestamp_sranipal, "t_sranipal", delim)
    s += to_msg(dreyevr.timestamp_carla_stream, "t_carla_stream", delim)
    s += to_msg(dreyevr.framesequence, "sr_frameseq", delim)
    s += to_msg_vec(dreyevr.gaze_ray, "gaze_ray", delim)
    s += to_msg_vec(dreyevr.eye_origin, "eye_origin", delim)
    s += to_msg(dreyevr.vergence, "vergence", delim)
    s += to_msg_vec(dreyevr.hmd_location, "hmd_locn", delim)
    s += to_msg_vec(dreyevr.hmd_rotation, "hmd_rotn", delim)
    s += to_msg_vec(dreyevr.gaze_ray_left, "gaze_ray_l", delim)
    s += to_msg_vec(dreyevr.eye_origin_left, "eye_origin_l", delim)
    s += to_msg_vec(dreyevr.gaze_ray_right, "gaze_ray_r", delim)
    s += to_msg_vec(dreyevr.eye_origin_right, "eye_origin_r", delim)
    s += to_msg(dreyevr.eye_openness_left, "eye_open_l", delim)
    s += to_msg(dreyevr.eye_openness_right, "eye_open_r", delim)
    s += to_msg_vec(dreyevr.pupil_posn_left, "pupil_posn_l", delim, True)
    s += to_msg_vec(dreyevr.pupil_posn_right, "pupil_posn_r", delim, True)
    s += to_msg(dreyevr.pupil_diam_left, "pupil_diam_l", delim)
    s += to_msg(dreyevr.pupil_diam_right, "pupil_diam_r", delim)
    s += to_msg(dreyevr.focus_actor_name, "focus_actor_name", delim)
    s += to_msg(dreyevr.focus_actor_pt, "focus_actor_pt", delim)
    s += to_msg(dreyevr.focus_actor_dist, "focus_actor_dist", delim)
    # boolens for validity
    s += to_msg(dreyevr.gaze_valid, "validity_gaze", delim)
    s += to_msg(dreyevr.gaze_valid_left, "validity_gaze_l", delim)
    s += to_msg(dreyevr.gaze_valid_right, "validity_gaze_r", delim)
    s += to_msg(dreyevr.eye_openness_valid_left, "validity_eye_open_l", delim)
    s += to_msg(dreyevr.eye_openness_valid_right, "validity_eye_open_r", delim)
    s += to_msg(dreyevr.pupil_posn_valid_left, "validity_pupil_posn_l", delim)
    s += to_msg(dreyevr.pupil_posn_valid_right, "validity_pupil_posn_r", delim)
    return String(s)


def init_ros_pub(IP_SELF, IP_ROSMASTER, PORT_ROSMASTER):
    global rospy  # to use with other functions
    import rospy  # need to apt install rospy packages for your linux distro
    global String  # to use with other functions
    from std_msgs.msg import String

    # set the environment variables for ROS_IP
    os.environ["ROS_IP"] = IP_SELF
    os.environ["ROS_MASTER_URI"] = "http://" + IP_ROSMASTER + ":" + str(PORT_ROSMASTER)

    # init ros publisher
    try:
        rospy.set_param('bebop_ip', IP_ROSMASTER)
        rospy.init_node("dreyevr_node")
        return rospy.Publisher("dreyevr_pub", String, queue_size=10)
    except ConnectionRefusedError:
        print("RospyError: Could not initialize rospy connection")
        sys.exit(1)


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
        '-rh', '--roshost',
        metavar='Rh',
        default='192.168.86.33',
        help='IP of the host ROS server (default: 192.168.86.33)')
    argparser.add_argument(
        '-rp', '--rosport',
        metavar='Rp',
        default=11311,
        help='TCP port for ROS server (default: 11311)')
    argparser.add_argument(
        '-sh', '--selfhost',
        metavar='Sh',
        default='192.168.86.123',
        help='IP of the ROS node (this machine) (default: 192.168.86.123)')
    argparser.add_argument(
        '--no-ros', action='store_true',
    )

    args = argparser.parse_args()

    client = carla.Client(args.host, args.port)
    client.set_timeout(10.0)
    sync_mode = False  # synchronous mode
    random.seed(int(time.time()))

    # tunable parameters for your configuration
    IP_SELF = args.selfhost  # where the rosnode is being run (here)
    # NOTE: that IP_SELF may not be the local host if passing main network to VM
    # where the rosmaster (carla roslaunch) is being run
    IP_ROSMASTER = args.roshost
    PORT_ROSMASTER = args.rosport

    if not args.no_ros:
        pub = init_ros_pub(IP_SELF, IP_ROSMASTER, PORT_ROSMASTER)

    world = client.get_world()
    DReyeVR_data = DReyeVRSensor()

    DReyeVR_sensor = DReyeVR_data.spawn(world)

    def publish_and_print(data, will_print=True):
        if not args.no_ros:
            dreyevr = DReyeVR_sensor.update(data)
            msg = CreateDReyeVRMsg(dreyevr)
            pub.publish(msg)  # publish to ros master
        if will_print:
            print(data)

    # subscribe to DReyeVR sensor
    DReyeVR_sensor.listen(publish_and_print)
    try:
        while True:
            if sync_mode:
                world.tick()
            else:
                world.wait_for_tick()
    finally:
        carla.command.DestroyActor(DReyeVR_sensor)
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
