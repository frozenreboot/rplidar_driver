# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [1.3.0] - 2026-01-18
### Added
- **Scan Interpolation:** Introduced `interpolated_rays` parameter to generate high-density, mathematical line-segment-based measurements (Contribution by @cosmicog).
- **Debugging Tools:** Added `publish_point_cloud` and `intensities_as_angles` parameters for deep analysis.
- **QoS Standardization:** Implemented standard ROS 2 QoS profiles (`SensorDataQoS`, etc.) configurable via `qos_policy`.

### Changed
- **Parameter Overhaul:**
  - Deprecated `scan_processing` in favor of explicit `interpolated_rays`.
  - Removed `inverted` parameter (use TF or `angle_offset` instead).
  - Default `scan_mode` set to "Standard" to prevent ghost points on S-series.

### Fixed
- **Critical Stability Fixes:**
  - Prevented segmentation faults in `dummy_mode` caused by unsafe dynamic casting.
  - Fixed index out-of-bounds crash in the interpolation loop.
  - Resolved `LifecycleNode` activation issues where publishers remained silent after state transition.
  - Fixed a `TypeError` (tuple vs list) in the composition launch file.

## [1.2.0] - 2026-01-09
### Changed
- Updated copyright years to 2026.

### Fixed
- **Breaking:** Fixed scan data mirroring issue (inverted left/right).
  - *Note: Please check your robot's rotation direction after this update.*
- Fixed an issue where `launch arguments` were overriding parameter file settings.

## [1.1.0] - 2026-01-06
### Added
- **ROS 2 Component Support:** Refactored `RPlidarNode` into a `rclcpp_component` to enable zero-copy composition and improve IPC performance.
- **Diagnostics:** Implemented standard system health monitoring (frequency, connection status) via `diagnostic_updater` (REP-107).

### Changed
- **REP-117 Compliance:** Updated scan data output to use `Inf` for out-of-range measurements, strictly adhering to standard ROS REPs.
