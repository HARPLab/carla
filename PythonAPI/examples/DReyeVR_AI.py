#!/usr/bin/env python

import glob
import os
import sys
import time

try:
    sys.path.append(
        glob.glob(
            "PythonAPI/carla/dist/carla-*%d.%d-%s.egg"
            % (
                sys.version_info.major,
                sys.version_info.minor,
                "win-amd64" if os.name == "nt" else "linux-x86_64",
            )
        )[0]
    )
except IndexError:
    pass

import carla

import argparse
from numpy import random


def set_DReyeVR_autopilot(world, traffic_manager):
    DReyeVR_vehicle = None
    for actor in world.get_actors():
        if "dreyevr" in actor.type_id.lower():
            DReyeVR_vehicle = actor
            break
    if DReyeVR_vehicle is None:
        print("Unable to find DReyeVR ego vehicle in world!")
    else:
        DReyeVR_vehicle.set_autopilot(True, traffic_manager.get_port())
        print("Successfully set autopilot on ego vehicle")
    return DReyeVR_vehicle


def spawn_other_vehicles(max_vehicles, world, traffic_manager):
    spawn_points = world.get_map().get_spawn_points()

    blueprints = world.get_blueprint_library().filter("vehicle.*")
    blueprints = sorted(blueprints, key=lambda bp: bp.id)

    other_vehicles = []
    for n, transform in enumerate(spawn_points):
        if n >= max_vehicles:
            break
        blueprint = random.choice(blueprints)
        if blueprint.has_attribute("color"):
            color = random.choice(blueprint.get_attribute("color").recommended_values)
            blueprint.set_attribute("color", color)
        if blueprint.has_attribute("driver_id"):
            driver_id = random.choice(
                blueprint.get_attribute("driver_id").recommended_values
            )
            blueprint.set_attribute("driver_id", driver_id)
        blueprint.set_attribute("role_name", "autopilot")

        new_car = world.spawn_actor(blueprint, transform)
        new_car.set_autopilot(True, traffic_manager.get_port())
        other_vehicles.append(new_car)

    print(f"successfully spawned {len(other_vehicles)} vehicles")
    return other_vehicles


def main():
    argparser = argparse.ArgumentParser(description=__doc__)
    argparser.add_argument(
        "--host",
        metavar="H",
        default="127.0.0.1",
        help="IP of the host server (default: 127.0.0.1)",
    )
    argparser.add_argument(
        "-p",
        "--port",
        metavar="P",
        default=2000,
        type=int,
        help="TCP port to listen to (default: 2000)",
    )
    argparser.add_argument(
        "-n",
        "--number-of-vehicles",
        metavar="N",
        default=10,
        type=int,
        help="number of vehicles (default: 10)",
    )
    argparser.add_argument(
        "--tm-port",
        metavar="P",
        default=8000,
        type=int,
        help="port to communicate with TM (default: 8000)",
    )
    argparser.add_argument(
        "-s", "--seed", metavar="S", type=int, help="Random device seed"
    )
    args = argparser.parse_args()

    client = carla.Client(args.host, args.port)
    client.set_timeout(10.0)
    random.seed(args.seed if args.seed is not None else int(time.time()))

    other_vehicles = []
    try:
        world = client.get_world()

        traffic_manager = client.get_trafficmanager(args.tm_port)
        traffic_manager.set_global_distance_to_leading_vehicle(1.0)
        if args.seed is not None:
            traffic_manager.set_random_device_seed(args.seed)

        ego_vehicle = set_DReyeVR_autopilot(world, traffic_manager)

        # spawn other vehicles
        other_vehicles = spawn_other_vehicles(
            args.number_of_vehicles, world, traffic_manager
        )

        while True:
            world.wait_for_tick()
    finally:

        print("\ndestroying %d vehicles" % len(other_vehicles))
        client.apply_batch([carla.command.DestroyActor(x.id) for x in other_vehicles])

        time.sleep(0.5)


if __name__ == "__main__":

    try:
        main()
    except KeyboardInterrupt:
        pass
    finally:
        print("\ndone.")

