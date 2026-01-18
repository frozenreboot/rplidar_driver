from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    # Define arguments
    robot_id_arg = DeclareLaunchArgument(
        name="robot_id",
        default_value="",
        description="Namespace/Robot ID to attach to RViz (e.g. robot1)",
    )

    # Define variables
    robot_id = LaunchConfiguration("robot_id")
    rviz_config_path = PathJoinSubstitution(
        [FindPackageShare("rplidar_ros2_driver"), "config", "debug.rviz"]
    )

    # Define RViz node
    rviz_node = Node(
        package="rviz2",
        executable="rviz2",
        name="rviz2",
        output="screen",
        namespace=robot_id,
        arguments=["-d", rviz_config_path],
        remappings=[
            ("/tf", "tf"),
            ("/tf_static", "tf_static"),
            ("/clicked_point", "clicked_point"),
            ("/goal_pose", "goal_pose"),
            ("/initialpose", "initialpose"),
            ("/twist_server", "twist_server"),
            ("/robot_description", "robot_description"),
        ],
    )

    return LaunchDescription([robot_id_arg, rviz_node])
