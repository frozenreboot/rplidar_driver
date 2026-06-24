/*
 * RPLIDAR ROS2 DRIVER
 *
 * Copyright (c) 2025 - 2026 frozenreboot
 * Copyright (c) 2009 - 2014 RoboPeak Team
 * Copyright (c) 2014 - 2022 Shanghai Slamtec Co., Ltd.
 *
 * This project is a refactored version of the original rplidar_ros package.
 * The architecture has been redesigned to support ROS2 Lifecycle and
 * Multithreading.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/**
 * @file lidar_driver_wrapper.hpp
 * @brief Abstract interface and concrete implementations for Slamtec RPLIDAR
 * drivers.
 *
 * This header defines a common abstraction layer for Slamtec RPLIDAR devices.
 * The goal is to decouple the vendor-specific Slamtec SDK from higher-level
 * application or ROS 2 node logic.
 *
 * It provides:
 *  - An abstract @ref LidarDriverInterface that describes the required driver
 * API
 *  - A real implementation @ref RealLidarDriver that wraps the Slamtec SDK
 *  - A lightweight @ref DummyLidarDriver used for simulation, testing, or CI
 *
 * By depending only on this interface, the rest of the system can be tested
 * without hardware and can evolve independently from the vendor SDK.
 *
 * @author  frozenreboot (frozenreboot@gmail.com)
 * @date    2025-12-13
 */
#ifndef LIDAR_DRIVER_WRAPPER_HPP
#define LIDAR_DRIVER_WRAPPER_HPP

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "sl_lidar.h"
#include "sl_lidar_driver.h"

namespace rplidar_driver {

/**
 * @enum ProtocolType
 * @brief Describes the protocol / behavior family of the RPLIDAR model.
 *
 * Different RPLIDAR model families have slightly different protocol
 * characteristics and geometric compensation requirements:
 *  - OLD_TYPE : Legacy A-series models, geometric compensation required
 *  - NEW_TYPE : Newer S/C-series models, geometric compensation typically not
 * required
 */
enum class ProtocolType {
  OLD_TYPE, ///< A-series (geometric compensation required)
  NEW_TYPE  ///< S/C-series (geometric compensation usually not required)
};

/**
 * @struct DriverProfile
 * @brief Captures static and runtime characteristics of the connected device.
 *
 * This profile is populated after successful device detection and can be used
 * by higher-level logic to apply model-specific behavior (e.g., range limits
 * or geometric compensation strategies).
 */
struct DriverProfile {
  /// Human-readable model name (e.g., "RPLIDAR A2", "RPLIDAR S3")
  std::string model_name = "Unknown";

  /// Protocol family of the device (legacy A-series vs newer S/C-series)
  ProtocolType protocol = ProtocolType::OLD_TYPE;

  /**
   * @brief Maximum measurable distance supported by the hardware (in meters).
   *
   * This is the physical limit of the device and can be used to clip or
   * validate scan data produced by the driver.
   */
  float hw_max_distance = 12.0f;

  /// Currently active scan mode name as reported by the device.
  std::string active_mode = "Standard";

  /// Currently active motor speed (rpm) if available, 0 if unknown.
  uint16_t active_rpm = 0;

  /**
   * @brief Whether geometric correction should be applied in the driver.
   *
   * For A-series devices this should typically be enabled.
   * For newer S-series devices this can often be disabled.
   */
  bool apply_geometric_correction = true;
};

// ============================================================================
// [Interface Definition]
// ============================================================================

/**
 * @class LidarDriverInterface
 * @brief Abstract interface for RPLIDAR driver implementations.
 *
 * This interface defines the minimal set of operations required from any
 * RPLIDAR driver implementation used by the rest of the system.
 *
 * Implementations are responsible for:
 *  - Managing the underlying Slamtec SDK objects and device connection
 *  - Controlling the motor, scan mode, and scan acquisition
 *  - Reporting device health and handling basic error recovery
 *
 * Higher-level code should depend only on this interface, not on any concrete
 * implementation or on the vendor SDK types directly.
 */
class LidarDriverInterface {
public:
  virtual ~LidarDriverInterface() = default;

  /**
   * @brief Establish a connection to the RPLIDAR device.
   *
   * This method is responsible for opening the serial port, creating the
   * underlying Slamtec driver instance, and performing any initial handshakes
   * required by the SDK.
   *
   * @param port Serial device path (e.g., "/dev/ttyUSB0").
   * @param baudrate Baudrate to use for the serial connection.
   * @param use_geometric_compensation
   *        Whether the driver should enable geometric compensation internally.
   *        Implementations may store this toggle in their internal profile.
   *
   * @return true on success, false if the connection or initialization fails.
   */
  virtual bool connect(const std::string &port, sl_u32 baudrate,
                       bool use_geometric_compensation = true) = 0;

  /**
   * @brief Close the device connection and release underlying resources.
   *
   * After this call, the device is considered disconnected and further
   * operations (e.g., grabbing scan data) are invalid until @ref connect is
   * called again.
   */
  virtual void disconnect() = 0;

  /**
   * @brief Check whether the driver currently considers itself connected.
   *
   * @return true if a device connection is active, false otherwise.
   */
  virtual bool isConnected() = 0;

  /**
   * @brief Start the motor and configure scan mode/speed.
   *
   * Implementations should:
   *  - Start the motor
   *  - Optionally select a scan mode based on @p user_mode_pref
   *  - Optionally set motor speed based on @p user_rpm_pref
   *
   * If the user preferences are empty/zero, a sensible default mode and rpm
   * should be selected.
   *
   * @param user_mode_pref Optional preferred scan mode name.
   * @param user_rpm_pref Optional preferred motor speed in rpm (0 for default).
   *
   * @return true on success, false if the device cannot enter a scanning state.
   */
  virtual bool start_motor(std::string user_mode_pref = "",
                           uint16_t user_rpm_pref = 0) = 0;

  /**
   * @brief Stop the motor and halt scanning.
   */
  virtual void stop_motor() = 0;

  /**
   * @brief Query the health status of the device.
   *
   * The concrete meaning of the returned integer is defined by the
   * implementation, but it typically propagates or maps from the Slamtec
   * health status structure.
   *
   * @return An integer code representing the device health.
   */
  virtual int getHealth() = 0;

  /**
   * @brief Perform a device reset.
   *
   * Implementations may send a reset command via the SDK and perform any
   * required internal state cleanup to recover from errors.
   */
  virtual void reset() = 0;

  /**
   * @brief Acquire a single scan from the device.
   *
   * Implementations should fill @p nodes with a single scan worth of
   * measurement nodes obtained from the underlying SDK.
   *
   * @param[out] nodes Output container for measurement nodes.
   * @return true on success, false if scan acquisition fails.
   */
  virtual bool grab_scan_data(
      std::vector<sl_lidar_response_measurement_node_hq_t> &nodes) = 0;

  /**
   * @brief Detect device model/protocol and initialize driver strategy.
   *
   * Typical responsibilities include:
   *  - Fetching device info
   *  - Determining protocol type (A-series vs S/C-series)
   *  - Selecting appropriate scan mode and compensation settings
   */
  virtual void detect_and_init_strategy() = 0;

  /**
   * @brief Print a human-readable summary of the current device/profile.
   *
   * This is mainly intended for debugging and logging.
   */
  virtual void print_summary() = 0;

  /**
   * @brief Get the maximum usable distance of the hardware in meters.
   *
   * This value is usually derived from the detected profile.
   *
   * @return Maximum range in meters.
   */
  virtual float get_hw_max_distance() const = 0;

  /**
   * @brief Set the motor speed of the device.
   *
   * Implementations may clamp or ignore unsupported rpm values.
   *
   * @param rpm Target motor speed in rpm.
   * @return true on success, false if the requested speed cannot be applied.
   */
  virtual bool set_motor_speed(uint16_t rpm) = 0;
};

// ============================================================================
// [Real Driver Class Declaration]
// ============================================================================

/**
 * @class RealLidarDriver
 * @brief Concrete driver implementation using the Slamtec RPLIDAR SDK.
 *
 * This class owns an @c sl::ILidarDriver instance and provides a concrete
 * implementation of @ref LidarDriverInterface on top of the Slamtec SDK.
 *
 * Responsibilities:
 *  - Managing the device connection lifecycle
 *  - Detecting model and protocol type (@ref ProtocolType)
 *  - Applying geometric compensation according to @ref DriverProfile
 *  - Handling scan acquisition and basic error reporting
 */
class RealLidarDriver : public LidarDriverInterface {
public:
  /**
   * @brief Construct a RealLidarDriver with no active connection.
   *
   * Use @ref connect to establish a device connection before use.
   */
  RealLidarDriver();

  /**
   * @brief Destructor. Ensures the device is properly disconnected.
   */
  ~RealLidarDriver() override;

  /// @copydoc LidarDriverInterface::connect()
  bool connect(const std::string &port, sl_u32 baudrate,
               bool use_geometric_compensation = true) override;

  /// @copydoc LidarDriverInterface::disconnect()
  void disconnect() override;

  /// @copydoc LidarDriverInterface::isConnected()
  bool isConnected() override;

  /// @copydoc LidarDriverInterface::start_motor()
  bool start_motor(std::string user_mode_pref = "",
                   uint16_t user_rpm_pref = 0) override;

  /// @copydoc LidarDriverInterface::stop_motor()
  void stop_motor() override;

  /// @copydoc LidarDriverInterface::getHealth()
  int getHealth() override;

  /// @copydoc LidarDriverInterface::reset()
  void reset() override;

  /// @copydoc LidarDriverInterface::grab_scan_data()
  bool grab_scan_data(
      std::vector<sl_lidar_response_measurement_node_hq_t> &nodes) override;

  /// @copydoc LidarDriverInterface::detect_and_init_strategy()
  void detect_and_init_strategy() override;

  /// @copydoc LidarDriverInterface::print_summary()
  void print_summary() override;

  /// @copydoc LidarDriverInterface::get_hw_max_distance()
  float get_hw_max_distance() const override;

  /// @copydoc LidarDriverInterface::set_motor_speed()
  bool set_motor_speed(uint16_t rpm) override;

  /**
   * @brief Check whether the detected device belongs to the "new" protocol
   * family.
   *
   * This is a convenience helper that allows higher-level code to branch
   * behavior based on hardware generation (e.g., A-series vs S/C-series).
   *
   * @return true if the device is recognized as a new-type model.
   */
  bool is_new_type() const;

  /**
   * @brief Get a human-readable string summarizing the detected device info.
   *
   * @return A concise description including model and firmware information.
   */
  std::string get_device_info_str() const;

private:
  /// Raw pointer to the underlying Slamtec driver instance (owned by this
  /// class).
  sl::ILidarDriver *drv_ = nullptr;

  /// Cached device information obtained from the SDK.
  sl_lidar_response_device_info_t devinfo_{};

  /// Profile describing the connected device and active configuration.
  DriverProfile profile_;

  /// Indicates whether the device has been detected as an S-series model.
  bool is_s_series_detected_ = false;
};

// ============================================================================
// [Dummy Driver Class Declaration]
// ============================================================================

/**
 * @class DummyLidarDriver
 * @brief Lightweight mock driver for simulation, testing, or CI environments.
 *
 * This implementation of @ref LidarDriverInterface does not interact with
 * any real hardware. It can be used to:
 *  - Run higher-level logic in test environments without a physical sensor
 *  - Provide deterministic or synthetic scan data
 *  - Simplify integration tests and continuous integration setups
 */

// Definition Mock driver states
enum class MockDriverState {
  DISCONNECTED, // Init
  CONNECTED,    // Idle
  SCANNING      // Active
};

/**
 * @class DummyLidarDriver
 * @brief Lightweight mock driver for simulation, testing, or CI environments.
 */
class DummyLidarDriver : public LidarDriverInterface {
public:
  /// Construct a dummy driver in a disconnected state.
  DummyLidarDriver() = default;

  /// Destructor (no hardware resources to release).
  ~DummyLidarDriver() override = default;

  /// @copydoc LidarDriverInterface::connect()
  bool connect(const std::string &port, sl_u32 baudrate,
               bool use_geometric_compensation = true) override;

  /// @copydoc LidarDriverInterface::disconnect()
  void disconnect() override;

  /// @copydoc LidarDriverInterface::isConnected()
  bool isConnected() override;

  /// @copydoc LidarDriverInterface::start_motor()
  bool start_motor(std::string user_mode_pref = "",
                   uint16_t user_rpm_pref = 0) override;

  /// @copydoc LidarDriverInterface::stop_motor()
  void stop_motor() override;

  /// @copydoc LidarDriverInterface::getHealth()
  int getHealth() override;

  /// @copydoc LidarDriverInterface::reset()
  void reset() override;

  /// @copydoc LidarDriverInterface::grab_scan_data()
  bool grab_scan_data(
      std::vector<sl_lidar_response_measurement_node_hq_t> &nodes) override;

  /// @copydoc LidarDriverInterface::detect_and_init_strategy()
  void detect_and_init_strategy() override;

  /// @copydoc LidarDriverInterface::print_summary()
  void print_summary() override;

  /// @copydoc LidarDriverInterface::get_hw_max_distance()
  float get_hw_max_distance() const override;

  /**
   * @brief Set motor speed in the dummy implementation.
   *
   * For the dummy driver this method always returns true and does not
   * perform any real hardware operation.
   *
   * @param rpm Target motor speed in rpm (ignored).
   * @return Always true.
   */
  bool set_motor_speed(uint16_t /*rpm*/) override { return true; }

private:
  // Use Enum state
  MockDriverState current_state_ = MockDriverState::DISCONNECTED;
};

} // namespace rplidar_driver

#endif // LIDAR_DRIVER_WRAPPER_HPP
