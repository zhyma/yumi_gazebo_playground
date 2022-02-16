cmake_minimum_required(VERSION 3.0.2)
project(yumi_gazebo)

# Check for c++11 / c++0x support
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "-std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "-std=c++0x")
else()
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

# Load catkin and all dependencies required for this package
# cannot have yumi_hw, not sure why
find_package(catkin REQUIRED COMPONENTS
  yumi_description
  yumi_moveit_config
  yumi_support
  roscpp
  gazebo_ros
  std_msgs
  message_generation
)

# Depend on system install of Gazebo
find_package(gazebo REQUIRED)

link_directories(${GAZEBO_LIBRARY_DIRS})
include_directories(${Boost_INCLUDE_DIR} ${catkin_INCLUDE_DIRS} ${GAZEBO_INCLUDE_DIRS})

add_message_files(
  FILES
    CylinderProperties.msg
)

generate_messages(
  DEPENDENCIES
    std_msgs
    geometry_msgs
)

catkin_package(
  DEPENDS
    roscpp
    gazebo_ros
  CATKIN_DEPENDS
    message_runtime
    yumi_description
    yumi_hw
    yumi_moveit_config
    yumi_support
)

add_library(${PROJECT_NAME} src/plugin.cpp)
target_link_libraries(${PROJECT_NAME} ${catkin_LIBRARIES} ${GAZEBO_LIBRARIES})

#############
## Install ##
#############

## Mark cpp header files for installation
install(DIRECTORY launch/
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
)