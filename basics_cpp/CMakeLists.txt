cmake_minimum_required(VERSION 3.22)  # use cmake 3.22 or later
project(basics_cpp)


# Enforce C++20 standard globally for this project
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)  # Use -std=c++20, not -std=gnu++20


# Enable strict compiler warnings
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()


# --- ROS2 dependencies ---
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(turtlesim REQUIRED)


# --- Executable: pose_publisher ---
add_executable(pose_publisher src/pose_publisher.cpp)


# BUILD_INTERFACE: local include/ during build
# INSTALL_INTERFACE: include/basics_cpp/ after installation
# PRIVATE: 对本库的源码可见，但不暴露给依赖本库的其他库
# INTERFACE: 对依赖本库的其他库可见，也对本库的源码可见，但不暴露给本库的源码
# PUBLIC: 对依赖本库的其他库可见，但不暴露给本库的源码
target_include_directories(pose_publisher PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}>)


# Link ROS2 dependencies to the target
ament_target_dependencies(pose_publisher
  rclcpp
  geometry_msgs
  turtlesim)


# --- Install rules ---
install(TARGETS pose_publisher
  DESTINATION lib/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()


# register this package to the ROS2 ecosystem
ament_package()
