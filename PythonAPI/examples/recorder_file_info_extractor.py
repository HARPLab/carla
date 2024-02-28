import os

path = "/home/srkhuran-local/.config/Epic/CarlaUE4/Saved/"
rec_files = ["exp_nik-pilot_12_05_2023_17_00_59.rec", "exp_pranay-pilot_11_22_2023_16_22_36.rec", "exp_shreeya-pilot_12_05_2023_15_04_50.rec"]

for f in rec_files:
    name = f.split("pilot")
    info_file = name[0] + "pilot.txt"
    os.system("python show_recorder_file_info.py -a -f" +path+f+" > "+info_file)
    print(info_file)

print("Done!")


