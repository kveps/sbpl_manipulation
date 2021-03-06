cmake_minimum_required(VERSION 2.8)
project(smpl_test)

set(CMAKE_BUILD_TYPE Release)
list(APPEND CMAKE_CXX_FLAGS "-std=c++11")

find_package(Boost REQUIRED COMPONENTS unit_test_framework)

find_package(catkin
    REQUIRED
    COMPONENTS
        cmake_modules
        geometry_msgs
        leatherman
        moveit_msgs
        moveit_ros_planning
        roscpp
        smpl
        sbpl_collision_checking
        sbpl_kdl_robot_model
        sbpl_pr2_robot_model
        visualization_msgs)

find_package(orocos_kdl REQUIRED)

find_package(Eigen REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${Eigen_INCLUDE_DIRS})
include_directories(${catkin_INCLUDE_DIRS})
include_directories(${orocos_kdl_INCLUDE_DIRS})

catkin_package()

add_executable(callPlanner src/call_planner.cpp)
target_link_libraries(callPlanner ${catkin_LIBRARIES})

add_executable(occupancy_grid_test src/occupancy_grid_test.cpp)
target_link_libraries(occupancy_grid_test ${catkin_LIBRARIES})

add_executable(visualizer_test src/visualizer_test.cpp)
target_link_libraries(visualizer_test ${catkin_LIBRARIES})

add_executable(csv_parser_test src/csv_parser_test.cpp)
target_link_libraries(csv_parser_test ${catkin_LIBRARIES})

add_executable(heap_test src/heap_test.cpp)
target_link_libraries(heap_test ${Boost_LIBRARIES} ${catkin_LIBRARIES})

add_executable(egraph_test src/egraph_test.cpp)
target_link_libraries(egraph_test ${Boost_LIBRARIES} ${catkin_LIBRARIES})

add_executable(xytheta src/xytheta.cpp)
target_link_libraries(xytheta ${catkin_LIBRARIES})

install(
    TARGETS callPlanner
    RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION})

