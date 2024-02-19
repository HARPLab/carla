import pickle as pkl

with open("C:\AwarenessManager\DReyeVR-parser\src\cache\shagun_study_11.pkl", "rb") as f:
    data = pkl.load(f)

timestamp_data = {}
print(len(data['TimestampCarla']))
print(len(data['AwarenessData']['VisibleTotal']))
print(len(data['AwarenessData']['UserInput']))
# print(len(data['AwarenessData']['Answer']))
# print(len(data['AwarenessData']['Velocity']))
# print(len(data['AwarenessData']['Location']))

for i in range(len(data['TimestampCarla'])):
    
    # dict_keys(['VisibleTotal', 'UserInput', 'Id', 'Location', 'Velocity', 'Answer'])
    temp_dict = {'VisibleTotal':data['AwarenessData']['VisibleTotal'][i], 'UserInput':data['AwarenessData']['UserInput'][i]}
    timestamp_data[data['TimestampCarla'][i]] = temp_dict
    # if i< 10:
    print(temp_dict)