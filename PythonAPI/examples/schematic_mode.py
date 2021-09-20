#!/usr/bin/env python

from DReyeVR_sensor import DReyeVRSensor
from no_rendering_mode import World, InputControl, HUD
from no_rendering_mode import COLOR_BUTTER_0, COLOR_SCARLET_RED_1, COLOR_WHITE, COLOR_ALUMINIUM_4
from no_rendering_mode import TITLE_INPUT, TITLE_HUD, TITLE_WORLD
from scipy.spatial.transform import Rotation

import carla
import math
import argparse
import random
import logging

try:
    import pygame
except ImportError:
    raise RuntimeError('cannot import pygame, make sure pygame package is installed')


class DReyeVRWorld(World):
    """Class that contains all World information from no_rendering_mode but with overrided DReyeVR functions"""

    def select_hero_actor(self, always_spectator=True):
        """Selects only one hero actor if there are more than one. If there are not any, it will spawn one."""
        print("Selecting spectator for ego-vehicle")
        all_actors = self.world.get_actors()
        # spectator view
        self.player0 = [actor for actor in all_actors
                        if 'spectator' in actor.type_id][0]
        hero_vehicles = [actor for actor in all_actors
                         if 'vehicle' in actor.type_id and actor.attributes['role_name'] == 'hero']
        # for now, will connect to the spectator if there are no other vehicles in the simulator
        if always_spectator is False and len(hero_vehicles) > 0:
            self.hero_actor = random.choice(hero_vehicles)
            self.hero_transform = self.hero_actor.get_transform()
        else:
            # spawns the spectator view
            self._spawn_player0()

    def _spawn_player0(self):
        """Spawns the hero actor when the script runs"""
        # Get a random blueprint.
        all_blueprints = self.world.get_blueprint_library()
        bp = random.choice(all_blueprints.filter(self.args.filter))
        bp.set_attribute('role_name', 'hero')
        if bp.has_attribute('color'):
            color = random.choice(bp.get_attribute('color').recommended_values)
            bp.set_attribute('color', color)
        # Spawn the "player" around the spectator.
        self.hero_actor = self.player0
        # add all the instance variables to not modify other code instances
        self.hero_actor.blueprint = bp
        # dummy functions, not supposed to do much
        self.hero_actor.set_autopilot = lambda x: None
        self.hero_actor.get_speed_limit = lambda: 60  # arbitrary speed limit
        self.hero_actor.apply_control = lambda x: None
        # NOTE: no need to set the bounding box for the actor, defined by the blueprint
        self.hero_transform = self.hero_actor.get_transform()
        # Save it in order to destroy it when closing program
        self.spawned_hero = self.hero_actor

    def render_eye_tracker(self, surface, world_to_pixel):
        if not self.eye_tracker.is_initialized:
            return  # only works if eye tracker has recieved something

        rot = Rotation.from_euler('yxz', self.eye_tracker.hmd_rotation, degrees=True)

        ray_start_locn = self.eye_tracker.hmd_location + rot.apply(self.eye_tracker.eye_origin)
        ray_length = 20  # in carla metres
        ray_end_locn = ray_start_locn + ray_length * rot.apply(self.eye_tracker.gaze_ray)
        gaze_ray_line = [carla.Location(ray_start_locn[0], ray_start_locn[1], ray_start_locn[2]),
                         carla.Location(ray_end_locn[0], ray_end_locn[1], ray_end_locn[2])]
        color = COLOR_SCARLET_RED_1
        # attach to ego-vehicle
        self.player0.get_transform().transform(gaze_ray_line)
        # map gaze data to pixels
        gaze_ray_line = [world_to_pixel(p) for p in gaze_ray_line]
        # draw lines on schematic
        pygame.draw.lines(surface, color, False, gaze_ray_line,
                          width=int(math.ceil(4.0 * self.map_image.scale)))

    def _render_player0(self, surface, player0, world_to_pixel):
        """Renders the player0's bounding boxes"""
        color = COLOR_BUTTER_0
        # Compute bounding box points
        bb = player0.bounding_box.extent
        corners = [carla.Location(x=-bb.x, y=-bb.y),
                   carla.Location(x=bb.x - 0.8, y=-bb.y),
                   carla.Location(x=bb.x, y=0),
                   carla.Location(x=bb.x - 0.8, y=bb.y),
                   carla.Location(x=-bb.x, y=bb.y),
                   carla.Location(x=-bb.x, y=-bb.y)
                   ]
        player0.get_transform().transform(corners)
        corners = [world_to_pixel(p) for p in corners]
        pygame.draw.lines(surface, color, False, corners,
                          int(math.ceil(4.0 * self.map_image.scale)))
        # render the eye gaze on top of the vehicle
        self.render_eye_tracker(surface, world_to_pixel)

    def tick(self, clock, eye_tracker):
        super(DReyeVRWorld, self).tick(clock)
        # can do something here with the eye tracker
        self.eye_tracker = eye_tracker

    def render_actors(self, surface, vehicles, traffic_lights, speed_limits, walkers):
        super(DReyeVRWorld, self).render_actors(surface, vehicles, traffic_lights,
                                                speed_limits, walkers)

        # Player 0
        self._render_player0(surface, self.player0,
                             self.map_image.world_to_pixel)

    def destroy(self):
        """Destroy the hero actor when class instance is destroyed"""
        if self.spawned_hero is not None:
            if 'spectator' in self.spawned_hero.type_id:
                pass  # don't destroy the spectator!
            else:
                self.spawned_hero.destroy()


def game_loop(args):
    """Initialized, Starts and runs all the needed modules for No Rendering Mode"""
    try:
        # Init Pygame
        pygame.init()
        display = pygame.display.set_mode(
            (args.width, args.height),
            pygame.HWSURFACE | pygame.DOUBLEBUF)

        # Place a title to game window
        pygame.display.set_caption(args.description)

        # Show loading screen
        font = pygame.font.Font(pygame.font.get_default_font(), 20)
        text_surface = font.render('Rendering map...', True, COLOR_WHITE)
        display.blit(text_surface, text_surface.get_rect(center=(args.width / 2, args.height / 2)))
        pygame.display.flip()

        # Init
        input_control = InputControl(TITLE_INPUT)
        hud = HUD(TITLE_HUD, args.width, args.height)
        world = DReyeVRWorld(TITLE_WORLD, args, timeout=2.0)

        # DReyeVR eye tracker
        DReyeVR_data = DReyeVRSensor()
        client = carla.Client(args.host, args.port)
        client.set_timeout(10.0)
        eye_tracker = DReyeVR_data.spawn(client.get_world())
        # subscribe to sensor in server
        eye_tracker.listen(DReyeVR_data.update)

        # For each module, assign other modules that are going to be used inside that module
        input_control.start(hud, world)
        hud.start()
        world.start(hud, input_control)

        # Game loop
        clock = pygame.time.Clock()
        while True:
            clock.tick_busy_loop(60)

            # Tick all modules
            world.tick(clock, DReyeVR_data)
            hud.tick(clock)
            input_control.tick(clock)

            # Render all modules
            display.fill(COLOR_ALUMINIUM_4)
            world.render(display)
            hud.render(display)
            input_control.render(display)

            pygame.display.flip()

    except KeyboardInterrupt:
        print('\nCancelled by user. Bye!')

    finally:
        if world is not None:
            world.destroy()

# ==============================================================================
# -- Main --------------------------------------------------------------------
# ==============================================================================


def main():
    """Parses the arguments received from commandline and runs the game loop"""

    # Define arguments that will be received and parsed
    argparser = argparse.ArgumentParser(
        description='DReyeVR+CARLA Schematic Visualizer')
    argparser.add_argument(
        '-v', '--verbose',
        action='store_true',
        dest='debug',
        help='print debug information')
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
        '--res',
        metavar='WIDTHxHEIGHT',
        default='1280x720',
        help='window resolution (default: 1280x720)')
    argparser.add_argument(
        '--filter',
        metavar='PATTERN',
        default='vehicle.tesla.model3',  # for our custom DReyeVR build
        help='actor filter (default: "vehicle.*")')
    argparser.add_argument(
        '--map',
        metavar='TOWN',
        default=None,
        help='start a new episode at the given TOWN')
    argparser.add_argument(
        '--no-rendering',
        action='store_true',
        help='switch off server rendering')
    argparser.add_argument(
        '--show-triggers',
        action='store_true',
        help='show trigger boxes of traffic signs')
    argparser.add_argument(
        '--show-connections',
        action='store_true',
        help='show waypoint connections')
    argparser.add_argument(
        '--show-spawn-points',
        action='store_true',
        help='show recommended spawn points')

    # Parse arguments
    args = argparser.parse_args()
    args.description = argparser.description
    args.width, args.height = [int(x) for x in args.res.split('x')]

    # Print server information
    log_level = logging.DEBUG if args.debug else logging.INFO
    logging.basicConfig(format='%(levelname)s: %(message)s', level=log_level)

    logging.info('listening to server %s:%s', args.host, args.port)
    print(__doc__)

    # Run game loop
    game_loop(args)


if __name__ == '__main__':
    main()
