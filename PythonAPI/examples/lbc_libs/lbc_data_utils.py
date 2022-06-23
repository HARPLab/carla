import collections
import queue
import time

import numpy as np
import carla

from PIL import Image, ImageDraw

def get_nearby_lights(vehicle, lights, pixels_per_meter=5.5, size=512, radius=5):
    result = list()

    transform = vehicle.get_transform()
    pos = transform.location
    theta = np.radians(90 + transform.rotation.yaw)
    R = np.array([
        [np.cos(theta), -np.sin(theta)],
        [np.sin(theta),  np.cos(theta)],
        ])

    for light in lights:
        delta = light.get_transform().location - pos

        target = R.T.dot([delta.x, delta.y])
        target *= pixels_per_meter
        target += size // 2

        if min(target) < 0 or max(target) >= size:
            continue

        trigger = light.trigger_volume
        light.get_transform().transform(trigger.location)
        dist = trigger.location.distance(vehicle.get_location())
        a = np.sqrt(
                trigger.extent.x ** 2 +
                trigger.extent.y ** 2 +
                trigger.extent.z ** 2)
        b = np.sqrt(
                vehicle.bounding_box.extent.x ** 2 +
                vehicle.bounding_box.extent.y ** 2 +
                vehicle.bounding_box.extent.z ** 2)

        if dist > a + b:
            continue

        result.append(light)

    return result


def draw_traffic_lights(image, vehicle, lights, pixels_per_meter=5.5, size=512, radius=5):
    image = Image.fromarray(image)
    draw = ImageDraw.Draw(image)

    transform = vehicle.get_transform()
    pos = transform.location
    theta = np.radians(90 + transform.rotation.yaw)
    R = np.array([
        [np.cos(theta), -np.sin(theta)],
        [np.sin(theta),  np.cos(theta)],
        ])

    for light in lights:
        delta = light.get_transform().location - pos

        target = R.T.dot([delta.x, delta.y])
        target *= pixels_per_meter
        target += size // 2

        if min(target) < 0 or max(target) >= size:
            continue

        trigger = light.trigger_volume
        light.get_transform().transform(trigger.location)
        dist = trigger.location.distance(vehicle.get_location())
        a = np.sqrt(
                trigger.extent.x ** 2 +
                trigger.extent.y ** 2 +
                trigger.extent.z ** 2)
        b = np.sqrt(
                vehicle.bounding_box.extent.x ** 2 +
                vehicle.bounding_box.extent.y ** 2 +
                vehicle.bounding_box.extent.z ** 2)

        if dist > a + b:
            continue

        x, y = target
        draw.ellipse(
                (x-radius, y-radius, x+radius, y+radius),
                23 + light.state.real)

    return np.array(image)
