:: run this from scenario runner root
:: change to where you installed CARLA
SET CARLA_ROOT=C:\AwarenessManager\carla.awareness_public

::clear PYTHONPATH so this doesn't accumulate
SET PYTHONPATH=
SET PYTHONPATH=%PYTHONPATH%;%CARLA_ROOT%\PythonAPI\carla
SET PYTHONPATH=%PYTHONPATH%;%CARLA_ROOT%\PythonAPI\carla\dist\carla-0.9.13-py3.7-win-amd64.egg
:: for DReyeVR_utils
SET PYTHONPATH=%PYTHONPATH%;%CARLA_ROOT%\PythonAPI\examples 

@REM python -i
@REM python .\PythonAPI\examples\generate_traffic.py -n 0 -w 25 --asynch



SET rec_fileroot=C:\Users\HARPlab\AppData\Local\CarlaUE4\Saved\
@REM SET rec_filename=exp_sud_21_02_02_2024_09_33_57.rec
@REM SET rec_filename=exp_pranay_test_21_02_10_2024_21_22_52.rec
SET rec_filename=exp_button-test_02_12_2024_18_39_09.rec

SET rec_txt_outdir=C:\AwarenessManager\recordings_txt\
@REM SET rec_txt_output=exp_sud_21_SA.txt

setlocal enabledelayedexpansion
rem Extract the desired part of the filename and append "_SA.txt"
for /f "tokens=1-3 delims=_" %%a in ("%rec_filename%") do (
    set "desired_part=%%a_%%b_%%c"
)

rem Append _SA.txt to the extracted part
set "rec_txt_output=!desired_part!_SA.txt"
python .\PythonAPI\examples\show_recorder_file_info.py -a -f %rec_fileroot%%rec_filename% >  %rec_txt_outdir%%rec_txt_output%

@REM python .\PythonAPI\examples\start_replaying.py -f %rec_fileroot%%rec_filename%

@REM recording_file = "C:\Users\HARPlab\AppData\Local\CarlaUE4\Saved\exp_sud_21_02_02_2024_09_33_57.rec"
@REM recorder_parse_file = "C:\AwarenessManager\recordings_txt\exp_sud_21_SA.txt" 
@REM python .\PythonAPI\examples\replay_instance_segmentation.py -f %recording_file% -p %recorder_parse_file% -o %rec_txt_outdir%