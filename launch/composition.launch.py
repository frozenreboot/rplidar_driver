#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
RPLIDAR ROS2 DRIVER

Copyright (c) 2025 - 2026 frozenreboot
Copyright (c) 2009 - 2014 RoboPeak Team
Copyright (c) 2014 - 2022 Shanghai Slamtec Co., Ltd.

This project is a refactored version of the original rplidar_ros package.
The architecture has been redesigned to support ROS 2 Lifecycle and
multithreading.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS)
OR BUSINESS INTERRUPTION HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
THE POSSIBILITY OF SUCH DAMAGE.

@file   composition.launch.py
@brief  Launch file for RPLIDAR ROS 2 Component Container.

This launch description:
  - Initializes a ComposableNodeContainer (Component Container)
  - Loads the `RPlidarNode` component into the container via Intra-Process Communication
  - Enables Zero-Copy data transfer for high-performance applications

Note:
  Unlike the standalone launch, this file does NOT automatically configure/activate
  the node by default. It is intended for advanced users or system integrators
  who prefer manual lifecycle management or composition-based deployments.
"""

import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.conditions import IfCondition
from launch_ros.actions import ComposableNodeContainer, LoadComposableNodes
from launch_ros.descriptions import ComposableNode
from launch.actions import DeclareLaunchArgument, GroupAction
from launch.substitutions import LaunchConfiguration, PythonExpression


def generate_launch_description():
    share_dir = get_package_share_directory("rplidar_ros2_driver")
    params_file = os.path.join(share_dir, "param", "rplidar.yaml")

    namespace_arg = DeclareLaunchArgument(
        "namespace",
        default_value="",
        description="Namespace of the ROS node, and its topics, services etc.",
    )
    node_name_arg = DeclareLaunchArgument(
        "node_name",
        default_value="rplidar_node",
        description="Name of the ROS node, can be e.g. front_c1_node",
    )
    output_arg = DeclareLaunchArgument(
        "output",
        default_value="screen",
        description="Output mode of the node.",
    )
    params_fullpath_arg = DeclareLaunchArgument(
        "params_fullpath",
        default_value=[params_file],
        description="OS specific full path of the configuration YAML file",
    )
    provide_a_container_arg = DeclareLaunchArgument(
        "provide_a_container",
        default_value="True",
        description="Whether to use rplidar_container or the user provided "
        "container with the [container_name] param",
    )
    container_name_arg = DeclareLaunchArgument(
        "container_name",
        default_value="rplidar_container",
        description="User provided ComposableNodeContainer name. Send the "
        "[namespace] separately if required. e.g. If running the nav2 "
        "within a container: '/ns/nav2_cont', set this to 'nav2_cont'",
    )
    component_name_arg = DeclareLaunchArgument(
        "component_name",
        default_value="rplidar_node",
        description="Node name of the composable rplidar node",
    )

    provide_a_container = LaunchConfiguration("provide_a_container")
    namespace = LaunchConfiguration("namespace")
    container_name = LaunchConfiguration("container_name")
    component_name = LaunchConfiguration("component_name")
    container_name_full = [namespace, "/", container_name]

    rplidar_component = ComposableNode(
        package="rplidar_ros2_driver",
        plugin="RPlidarNode",  # name of macro-registered c++
        name=component_name,
        namespace=namespace,
        parameters=[LaunchConfiguration("params_fullpath")],
        extra_arguments=[{"use_intra_process_comms": True}],  # Zero-Copy enabled
    )

    container = ComposableNodeContainer(
        namespace=namespace,
        name=container_name,
        package="rclcpp_components",
        executable="component_container",
        composable_node_descriptions=[rplidar_component],
        output="screen",
    )

    load_composable_node_wo_container = GroupAction(
        condition=IfCondition(PythonExpression(["not ", provide_a_container])),
        actions=[
            LoadComposableNodes(
                target_container=container_name_full,
                composable_node_descriptions=[rplidar_component],
            ),
        ],
    )

    load_composable_node_w_container = GroupAction(
        condition=IfCondition(provide_a_container),
        actions=[container],
    )

    ld = LaunchDescription()
    ld.add_action(namespace_arg)
    ld.add_action(node_name_arg)
    ld.add_action(output_arg)
    ld.add_action(params_fullpath_arg)
    ld.add_action(provide_a_container_arg)
    ld.add_action(container_name_arg)
    ld.add_action(component_name_arg)
    ld.add_action(load_composable_node_wo_container)
    ld.add_action(load_composable_node_w_container)

    return ld
