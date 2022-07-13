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

# data collection options
ROUTENUM=32
SENSORCONFIG="CILRS"

FILES="/scratch/abhijatb/Bosch22/dreyevr_recordings/*$ROUTENUM.rec"
for replay_file in $FILES
do
    python3 replay_agent_SI.py \
    -f $replay_file \
    --route-file=/scratch/abhijatb/Bosch22/LbC_DReyeVR/leaderboard/data/route$ROUTENUM.xml \
    --route-id=$ROUTENUM \
    --sensor-config=$SENSORCONFIG \
    --sync-replay 
    --start 15
  sleep 5
  break
done



# python3 replay_agent_SI.py \
# -f /scratch/abhijatb/Bosch22/dreyevr_recordings/exp_swapnil54.rec \
# --route-file=/scratch/abhijatb/Bosch22/LbC_DReyeVR/leaderboard/data/route54.xml \
# --scenarios-file=/scratch/abhijatb/Bosch22/LbC_DReyeVR/leaderboard/data/dreyevr/town05_scenarios.json \
# --route-id=54 \
# --sync-replay 
#--debug-mode

# python3 replay_agent_SI.py \
# -f /scratch/abhijatb/Bosch22/dreyevr_recordings/exp_swapnil55.rec \
# --route-file=/scratch/abhijatb/Bosch22/LbC_DReyeVR/leaderboard/data/route55.xml \
# --scenarios-file=/scratch/abhijatb/Bosch22/LbC_DReyeVR/leaderboard/data/dreyevr/town05_scenarios.json \
# --route-id=55 \
# --sync-replay \
# --debug-mode

# FILES="/scratch/abhijatb/Bosch22/dreyevr_recordings/*32.rec"
# for replay_file in $FILES
# do
#   echo "Recording $replay_file"
#   # take action on each file. $f store current file name
#   # perform some operation with the file
#   python3 replay_agent_SI.py \
#   -f $replay_file \
#   --route-file=/scratch/abhijatb/Bosch22/LbC_DReyeVR/leaderboard/data/route32.xml \
#   --route-id=32 \
#   --scenarios-file=/scratch/abhijatb/Bosch22/LbC_DReyeVR/leaderboard/data/dreyevr/town03_scenarios.json \
#   --sync-replay

# python3 replay_agent_SI.py \
# -f /scratch/abhijatb/Bosch22/dreyevr_recordings/exp_swapnil32.rec \
# --route-file=/scratch/abhijatb/Bosch22/LbC_DReyeVR/leaderboard/data/route32.xml \
# --route-id=32 \
# --scenarios-file=/scratch/abhijatb/Bosch22/LbC_DReyeVR/leaderboard/data/dreyevr/town03_scenarios.json \
# --sync-replay
# -- debug-mode