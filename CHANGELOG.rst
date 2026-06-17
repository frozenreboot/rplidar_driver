^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package rplidar_driver
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Forthcoming
-----------

* Added GTest-based test infrastructure, including mock driver tests and lifecycle/publication tests.
* Added driver state and driver sequence documentation.
* Updated README to mark the current release status as beta and document related cautions.
* Clarified that the angle_offset parameter is specified in radians, following `REP-103 <https://www.ros.org/reps/rep-0103.html>`_.
* Removed the angle_offset launch argument and rely on YAML parameters instead.
* Fixed QoS parameter inconsistency by initializing the QoS parameter and applying the configured policy.
* Added governance documentation and maintainer information.
* Added an AI disclosure section for OSRF compliance.
* Contributors: JWJ | frozenreboot, wj, cosmicog

v1.3.0 (2026-01-18)
-------------------

* Added scan interpolation via the `interpolated_rays` parameter to generate high-density, mathematical line-segment-based measurements.
* Added `publish_point_cloud` and `intensities_as_angles` parameters for debugging and analysis.
* Added configurable ROS 2 QoS profiles via the `qos_policy` parameter.
* Deprecated `scan_processing` in favor of the explicit `interpolated_rays` parameter.
* Removed the `inverted` parameter. Use TF or `angle_offset` instead.
* Set the default `scan_mode` to `Standard` to prevent ghost points on S-series devices.
* Fixed segmentation faults in `dummy_mode` caused by unsafe dynamic casting.
* Fixed an index out-of-bounds crash in the interpolation loop.
* Fixed `LifecycleNode` activation issues where publishers remained silent after state transition.
* Fixed a `TypeError` in the composition launch file.
* Contributors: cosmicog, Woojin Jung

v1.2.0 (2026-01-09)
-------------------

* Updated copyright years to 2026.
* Fixed scan data mirroring issue where left and right were inverted.
* Fixed launch arguments overriding parameter file settings.
* Contributors: Woojin Jung

v1.1.0 (2026-01-06)
-------------------

* Refactored `RPlidarNode` into a `rclcpp_component` to enable component composition and improve IPC performance.
* Added diagnostics for frequency and connection status via `diagnostic_updater`.
* Updated scan data output to use `Inf` for out-of-range measurements according to REP-117.
* Contributors: Woojin Jung

v1.0.1 (2026-01-03)
-------------------

* Previous release before the current rosdistro release preparation.
