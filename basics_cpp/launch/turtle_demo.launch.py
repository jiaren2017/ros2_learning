# turtle_demo.launch.py
# Launches the complete turtle demo pipeline:
# turtlesim → pose_publisher → pose_subscriber → turtle_arc_server

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():

    # Declare a configurable argument: target rotation angle for the arc server
    # Can be overridden at launch time:
    # ros2 launch basics_cpp turtle_demo.launch.py linear_speed:=2.0
    linear_speed_arg = DeclareLaunchArgument(
        "linear_speed",
        default_value="1.0",
        description="Linear speed for turtle_mover (rad/s)"
    )

    angular_speed_arg = DeclareLaunchArgument(
        "angular_speed",
        default_value="1.0",
        description="Angular speed for turtle_mover (rad/s)"
    )

    # LaunchConfiguration reads the argument value at launch time
    linear_speed  = LaunchConfiguration("linear_speed")
    angular_speed = LaunchConfiguration("angular_speed")

    # --- Node definitions ---

    turtlesim_node = Node(
        package="turtlesim",
        executable="turtlesim_node",
        name="turtlesim",
        output="screen"
    )

    pose_publisher_node = Node(
        package="basics_cpp",
        executable="pose_publisher",
        name="pose_publisher",
        output="screen"
    )

    pose_subscriber_node = Node(
        package="basics_cpp",
        executable="pose_subscriber",
        name="pose_subscriber",
        output="screen"
    )

    turtle_mover_node = Node(
        package="basics_cpp",
        executable="turtle_mover",
        name="turtle_mover",
        output="screen",
        # Pass Parameters directly into the node at launch time
        parameters=[{
            "linear_speed":  linear_speed,
            "angular_speed": angular_speed,
        }]
    )

    turtle_arc_server_node = Node(
        package="basics_cpp",
        executable="turtle_arc_server",
        name="turtle_arc_server",
        output="screen"
    )

    return LaunchDescription([
        # Arguments must be declared before nodes that use them
        linear_speed_arg,
        angular_speed_arg,
        turtlesim_node,
        pose_publisher_node,
        pose_subscriber_node,
        turtle_mover_node,
        turtle_arc_server_node,
    ])