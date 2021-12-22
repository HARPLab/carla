import numpy as np
from typing import Optional
import carla


def find_ego_vehicle(world: carla.libcarla.World) -> Optional[carla.libcarla.Vehicle]:
    DReyeVR_vehicle = None
    ego_vehicles = world.get_actors().filter("vehicle.dreyevr.egovehicle")
    try:
        DReyeVR_vehicle = ego_vehicles[0]  # TODO: support for multiple ego vehicles?
    except IndexError:
        print("Unable to find DReyeVR ego vehicle in world!")
    return DReyeVR_vehicle


def find_eye_tracker(world):
    sensor = None
    eye_trackers = world.get_actors().filter("sensor.dreyevr.dreyevrsensor")
    try:
        sensor = eye_trackers[0]  # TODO: support for multiple eye trackers?
    except IndexError:
        print("Unable to find DReyeVR ego vehicle in world!")
    return sensor


class DReyeVRSensor:

    # just some enums to clear things up later
    LEFT: str = "LEFT"
    COMBO: str = "COMBO"
    RIGHT: str = "RIGHT"

    def __init__(self, world):
        self.eye_tracker = find_eye_tracker(world)
        self.has_data: bool = False
        print("initialized DReyeVRSensor PythonAPI client")

    def update(self, data):
        # all the local variables
        self.has_data = True
        self.timestamp_carla = data.timestamp_carla
        self.timestamp_sranipal = data.timestamp_sranipal
        self.timestamp_carla_stream = data.timestamp_carla_stream
        self.gaze_ray = self.to_np(data.gaze_ray)
        self.eye_origin = self.to_np(data.eye_origin) / 100.0
        self.eye_origin[1] *= -1  # flipped y value to match Carla debugger
        self.vergence = data.vergence
        self.hmd_location = self.to_np(data.hmd_location) / 100.0
        self.hmd_location[1] *= -1  # flipped y value to match Carla debugger
        self.hmd_rotation = self.to_np(data.hmd_rotation)
        self.gaze_ray_left = self.to_np(data.gaze_ray_left)
        self.gaze_ray_left[1] *= -1  # flipped gaze y value to match debugger
        self.eye_origin_left = self.to_np(data.eye_origin_left) / 100.0
        # flipped y value to match Carla debugger
        self.eye_origin_left[1] *= -1
        self.gaze_ray_right = self.to_np(data.gaze_ray_right)
        self.gaze_ray_right[1] *= -1  # flipped gaze y value to match debugger
        self.eye_origin_right = self.to_np(data.eye_origin_right) / 100.0
        # flipped y value to match Carla debugger
        self.eye_origin_right[1] *= -1
        self.eye_openness_left = data.eye_openness_left
        self.eye_openness_right = data.eye_openness_right
        # pupil positions
        self.pupil_posn_left = self.to_np(data.pupil_posn_left)
        self.pupil_posn_right = self.to_np(data.pupil_posn_right)
        # pupil diameters
        self.pupil_diam_left = data.pupil_diam_left
        self.pupil_diam_right = data.pupil_diam_right

    def to_np(self, vector):
        # TODO: there has to be a better way to do this
        try:
            numpy_vec = np.array([vector.x, vector.y, vector.z])
        except:
            numpy_vec = np.array([vector.x, vector.y])
        return numpy_vec

    @classmethod
    def spawn(cls, world):
        # TODO: check if dreyevr sensor already exsists, then use it
        # spawn a DReyeVR sensor and begin listening
        if find_eye_tracker(world) is None:
            bp = [x for x in world.get_blueprint_library().filter("sensor.dreyevr*")]
            try:
                bp = bp[0]
            except IndexError:
                print("no eye tracker in blueprint library?!")
                return None
            ego_vehicle = find_ego_vehicle()
            eye_tracker = world.spawn_actor(
                bp, ego_vehicle.get_transform(), attach_to=ego_vehicle
            )
            print("Spawned DReyeVR sensor: " + eye_tracker.type_id)
        return cls(world)

    def calc_vergence_from_dir(self, L0, R0, LDir, RDir):
        # Calculating shortest line segment intersecting both lines
        # Implementation sourced from http://paulbourke.net/geometry/Ptlineplane/

        L0R0 = L0 - R0  # segment between L origin and R origin
        epsilon = 0.00000001  # small positive real number

        # Calculating dot-product equation to find perpendicular shortest-line-segment
        d1343 = L0R0[0] * RDir[0] + L0R0[1] * RDir[1] + L0R0[2] * RDir[2]
        d4321 = RDir[0] * LDir[0] + RDir[1] * LDir[1] + RDir[2] * LDir[2]
        d1321 = L0R0[0] * LDir[0] + L0R0[1] * LDir[1] + L0R0[2] * LDir[2]
        d4343 = RDir[0] * RDir[0] + RDir[1] * RDir[1] + RDir[2] * RDir[2]
        d2121 = LDir[0] * LDir[0] + LDir[1] * LDir[1] + LDir[2] * LDir[2]
        denom = d2121 * d4343 - d4321 * d4321
        if abs(denom) < epsilon:
            return 1.0  # no intersection, would cause div by 0 err (potentially)
        numer = d1343 * d4321 - d1321 * d4343

        # calculate scalars (mu) that scale the unit direction XDir to reach the desired points
        # variable scale of direction vector for LEFT ray
        muL = numer / denom
        # variable scale of direction vector for RIGHT ray
        muR = (d1343 + d4321 * (muL)) / d4343

        # calculate the points on the respective rays that create the intersecting line
        ptL = L0 + muL * LDir  # the point on the Left ray
        ptR = R0 + muR * RDir  # the point on the Right ray

        # calculate the vector between the middle of the two endpoints and return its magnitude
        # middle point between two endpoints of shortest-line-segment
        ptM = (ptL + ptR) / 2.0
        oM = (L0 + R0) / 2.0  # midpoint between two (L & R) origins
        FinalRay = ptM - oM  # Combined ray between midpoints of endpoints
        # returns the magnitude of the vector (length)
        return np.linalg.norm(FinalRay) / 100.0
