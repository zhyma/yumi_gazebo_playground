Urdf & Gazebo files for the ABB YuMi (IRB 14000) robot

This is the modified version of [OrebroUniversity/yumi](https://github.com/OrebroUniversity/yumi) to build for ROS Noetic

### Build

#### Step 1
Install all of these:
```
sudo apt-get install \
        python3-pip \
        protobuf-compiler \
        protobuf-c-compiler \
        ros-$ROS_DISTRO-control-toolbox \
        ros-$ROS_DISTRO-controller-interface \
        ros-$ROS_DISTRO-controller-manager \
        ros-$ROS_DISTRO-effort-controllers \
        ros-$ROS_DISTRO-force-torque-sensor-controller \
        ros-$ROS_DISTRO-gazebo-ros-control \
        ros-$ROS_DISTRO-joint-limits-interface \
        ros-$ROS_DISTRO-joint-state-publisher \
        ros-$ROS_DISTRO-joint-state-controller \
        ros-$ROS_DISTRO-joint-trajectory-controller \
        ros-$ROS_DISTRO-moveit-commander \
        ros-$ROS_DISTRO-moveit-core \
        ros-$ROS_DISTRO-moveit-planners \
        ros-$ROS_DISTRO-moveit-ros-move-group \
        ros-$ROS_DISTRO-moveit-ros-planning \
        ros-$ROS_DISTRO-moveit-ros-visualization \
        ros-$ROS_DISTRO-moveit-simple-controller-manager \
        ros-$ROS_DISTRO-position-controllers \
        ros-$ROS_DISTRO-rqt-joint-trajectory-controller \
        ros-$ROS_DISTRO-transmission-interface \
        ros-$ROS_DISTRO-velocity-controllers 
```

```
    pip3 install --user pyftpdlib
    pip3 install --user --upgrade pyassimp
```
```
    sudo apt install ros-noetic-hector-xacro-tools
```

#### Step 2
Build the abb_driver package from source.
```
# change to the root of the Catkin workspace
cd $HOME/catkin_ws

# retrieve the latest development version of the abb_driver repository. If you'd rather
# use the latest released version, replace 'kinetic-devel' with 'kinetic'
git clone -b melodic-devel https://github.com/ros-industrial/abb_driver.git src/abb_driver

# check for and install missing build dependencies.

# first: update the local database
rosdep update

# now install dependencies, again using rosdep.
# Note: this may install additional packages, depending on the software already present
# on the machine.
# Be sure to change 'kinetic' to whichever ROS version you are using
rosdep install --from-paths src/ --ignore-src --rosdistro noetic

# build the workspace (using catkin_tools)
catkin build
```

#### Step 3
Build the industrial_core package from source.
```
# change to the root of the Catkin workspace
$ cd $HOME/catkin_ws

# retrieve the latest development version of industrial_core. If you'd rather
# use the latest released version, replace 'melodic-devel' with 'melodic'
$ git clone -b melodic-devel https://github.com/ros-industrial/industrial_core.git src/industrial_core

# check build dependencies. Note: this may install additional packages,
# depending on the software installed on the machine
$ rosdep update

# be sure to change 'melodic' to whichever ROS release you are using
$ rosdep install --from-paths src/ --ignore-src --rosdistro noetic

# build the workspace (using catkin_tools)
$ catkin build
```

#### Step 4
Finally, `catkin build` the workspace containing the clone of this package.

### Usage
- `Source ~/devel/setup.bash`
- Use rViz and Gazebo:
    `roslaunch yumi_launch yumi_gazebo_pos_control.launch`
- Use rViz and Moveit! (with default motion planner)
    `roslaunch yumi_moveit_config demo.launch`

## Note
- Disable Gazebo GUI: in the launch file (such as yumi_gazebo_pos_control.launch), `<!--Call Gazebo -->` section, change `<arg name="gui" value="true" />`
    - You can start the GUI by `rosrun gazebo_ros gazebo` manually.