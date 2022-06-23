export CARLA_ROOT=/scratch/abhijatb/Bosch22/carla.harp_p13bd
# export SCENARIO_RUNNER_ROOT=/scratch/abhijatb/Bosch22/scenario_runner_913
export PORT=2000                                                    # change to port that CARLA is running on
export ROUTES=/scratch/abhijatb/Bosch22/LbC_DReyeVR/leaderboard/data/route55.xml         # change to desired route
# export TEAM_AGENT=replay_pilot.py                                     # no need to change
# export TEAM_CONFIG=dreyevr_data_collect                                     # change path to save data
export HAS_DISPLAY=1                                                # set to 0 if you don't want a debug window


export PYTHONPATH=$PYTHONPATH:$CARLA_ROOT/PythonAPI/carla
export PYTHONPATH=$PYTHONPATH:$CARLA_ROOT/PythonAPI/carla/dist/carla-0.9.13-py3.6-linux-x86_64.egg
export PYTHONPATH=$PYTHONPATH:lbc_libs
export PYTHONPATH=$PYTHONPATH:/scratch/abhijatb/Bosch22/LbC_DReyeVR
export PYTHONPATH=$PYTHONPATH:/scratch/abhijatb/Bosch22/LbC_DReyeVR/leaderboard
export PYTHONPATH=$PYTHONPATH:/scratch/abhijatb/Bosch22/LbC_DReyeVR/leaderboard/team_code
export PYTHONPATH=$PYTHONPATH:/scratch/abhijatb/Bosch22/LbC_DReyeVR/scenario_runner


python3 replay_and_sensorout.py \
-f /scratch/abhijatb/Bosch22/dreyevr_recordings/exp_swapnil55.rec \
--route-file=/scratch/abhijatb/Bosch22/LbC_DReyeVR/leaderboard/data/route55.xml \
--scenarios-file=/scratch/abhijatb/Bosch22/LbC_DReyeVR/leaderboard/data/dreyevr/town05_scenarios.json \
--route-id=55 \
--sync-replay