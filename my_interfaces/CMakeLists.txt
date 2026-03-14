cmake_minimum_required(VERSION 3.8)  # minimum required by ROS2 Jazzy
project(my_interfaces)

# --- ROS2 build system ---
find_package(ament_cmake REQUIRED)

# rosidl_default_generators: code generation toolchain that converts
# .action / .srv / .msg definitions into C++ headers and type support libraries
find_package(rosidl_default_generators REQUIRED)

# Enumerate all interface definition files to be compiled.
# rosidl will generate C++ bindings for each entry.
# Generated namespace: my_interfaces::action::RotateTurtle
rosidl_generate_interfaces(${PROJECT_NAME}
  "action/RotateTurtle.action"
)

# Propagate rosidl_default_runtime as a dependency to downstream packages
# that consume the generated interfaces (e.g. basics_cpp)
ament_export_dependencies(rosidl_default_runtime)

# Finalizes the ament package: registers it with the ROS2 build ecosystem,
# exports package metadata, and generates ament index files used by colcon
ament_package()