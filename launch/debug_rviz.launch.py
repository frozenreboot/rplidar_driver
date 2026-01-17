from launch import LaunchDescription
from launch.actions import (
    AppendEnvironmentVariable as AppendENV,
    DeclareLaunchArgument as ARG,
    IncludeLaunchDescription as INCLUDE,
)
from launch.substitutions import (
    LaunchConfiguration as VAR,
    PathJoinSubstitution as PathJ,
)
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare as FPKG

ARGUMENTS = [
    ARG(name='robot_id', default_value=''),
]

def generate_launch_description():
    rviz = Node(package='rviz2', executable='rviz2', output='screen', 
    namespace=VAR('robot_id'),
    arguments=[
        '-d', PathJ([FPKG('rplidar_ros2_driver'), 'config', 'debug.rviz'])],
    remappings=[
        ('/tf', 'tf'),
        ('/tf_static', 'tf_static'),
        ('/clicked_point', 'clicked_point'),
        ('/goal_pose', 'goal_pose'),
        ('/initialpose', 'initialpose'),
        ('/twist_server', 'twist_server'),
        ('/robot_description', 'robot_description'), ])
    ld = LaunchDescription(ARGUMENTS)
    ld.add_action(rviz)

    return ld
