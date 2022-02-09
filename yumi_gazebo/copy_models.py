import os

# os.system('cp -r rod ~/.gazebo/models')


folders = os.listdir('./models')
for f in folders:
    cmd_string = 'cp -r ./models/' + f + ' ~/.gazebo/models'
    os.system(cmd_string)
    print(cmd_string)
