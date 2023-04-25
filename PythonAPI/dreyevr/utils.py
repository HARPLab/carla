from typing import Optional
import dreyevr  # calls __init__.py which ensures PythonPath contains the necessary files!
import carla
import time


def find_ego_vehicle(world: carla.libcarla.World) -> Optional[carla.libcarla.Vehicle]:
    DReyeVR_vehicles: str = "harplab.dreyevr_vehicle.*"
    ego_vehicles_in_world = list(world.get_actors().filter(DReyeVR_vehicles))
    if len(ego_vehicles_in_world) >= 1:
        print(
            f"Found a DReyeVR EgoVehicle in the world ({ego_vehicles_in_world[0].id})"
        )
        return ego_vehicles_in_world[0]

    DReyeVR_vehicle: Optional[carla.libcarla.Vehicle] = None
    available_ego_vehicles = world.get_blueprint_library().filter(DReyeVR_vehicles)
    if len(available_ego_vehicles) == 1:
        bp = available_ego_vehicles[0]
        print(f'Spawning only available EgoVehicle: "{bp.id}"')
    else:
        print(
            f"Found {len(available_ego_vehicles)} available EgoVehicles. Which one to use?"
        )
        for i, ego in enumerate(available_ego_vehicles):
            print(f"\t[{i}] - {ego.id}")
        print()
        ego_choice = f"Pick EgoVehicle to spawn [0-{len(available_ego_vehicles) - 1}]: "
        i: int = int(input(ego_choice))
        assert 0 <= i < len(available_ego_vehicles)
        bp = available_ego_vehicles[i]
    i: int = 0
    spawn_pts = world.get_map().get_spawn_points()
    while DReyeVR_vehicle is None:
        print(f'Spawning DReyeVR EgoVehicle: "{bp.id}" at {spawn_pts[i]}')
        DReyeVR_vehicle = world.spawn_actor(bp, transform=spawn_pts[i])
        i = (i + 1) % len(spawn_pts)
    return DReyeVR_vehicle


def find_ego_sensor(world: carla.libcarla.World) -> Optional[carla.libcarla.Sensor]:
    def get_world_sensors() -> list:
        return list(world.get_actors().filter("harplab.dreyevr_sensor.*"))

    ego_sensors: list = get_world_sensors()
    if len(ego_sensors) == 0:
        # no EgoSensors found in world, trying to spawn EgoVehicle (which spawns an EgoSensor)
        if find_ego_vehicle(world) is None:  # tries to spawn an EgoVehicle
            raise Exception(
                "No EgoVehicle (nor EgoSensor) found in the world! EgoSensor needs EgoVehicle as parent"
            )
    # in case we had to spawn the EgoVehicle, this effect is not instant and might take some time
    # to account for this, we allow some time (max_wait_sec) to continuously ping the server for
    # an updated actor list with the EgoSensor in it.

    start_t: float = time.time()
    # maximum time to keep checking for EgoSensor spawning after EgoVehicle
    maximum_wait_sec: float = 10.0  # might take a while to spawn EgoVehicle (esp in VR)
    while len(ego_sensors) == 0 and time.time() - start_t < maximum_wait_sec:
        # EgoVehicle should now be present, so we can try again
        ego_sensors = get_world_sensors()
        time.sleep(0.1)  # tick to allow the server some time to breathe
    if len(ego_sensors) == 0:
        raise Exception("Unable to find EgoSensor in the world!")
    assert len(ego_sensors) > 0  # should have spawned with EgoVehicle at least
    if len(ego_sensors) > 1:
        print("[WARN] There are >1 EgoSensors in the world! Defaulting to first")
    return ego_sensors[0]  # always return the first one?
