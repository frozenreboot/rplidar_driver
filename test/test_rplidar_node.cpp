#include <future>
#include <gtest/gtest.h>
#include <lifecycle_msgs/msg/state.hpp>
#include <lifecycle_msgs/msg/transition.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>

#include "rplidar_node.hpp"

using lifecycle_msgs::msg::State;
using lifecycle_msgs::msg::Transition;

class RplidarNodeTest : public ::testing::Test {
protected:
  // Initialize ROS 2 runtime once for the entire test suite
  static void SetUpTestCase() { rclcpp::init(0, nullptr); }

  // Shutdown ROS 2 runtime after tests complete
  static void TearDownTestCase() { rclcpp::shutdown(); }
};

/**
 * @brief [Lifecycle Verification]
 * Tests the complete lifecycle state transitions (Configure -> Activate ->
 * Deactivate -> Cleanup). Uses 'dummy_mode' to isolate logic from physical
 * hardware dependencies.
 */
TEST_F(RplidarNodeTest, FullLifecycleTest) {
  // ==========================================================================
  // [Arrange] Setup the node with dummy mode enabled
  // ==========================================================================
  rclcpp::NodeOptions options;
  // Enable dummy driver to simulate hardware behavior
  options.append_parameter_override("dummy_mode", true);
  // Set a dummy serial port (not used in dummy mode, but required for init)
  options.append_parameter_override("serial_port", "/dev/null");

  auto node = std::make_shared<rplidar_driver::RPlidarNode>(options);

  // Verify initial state
  EXPECT_EQ(node->get_current_state().id(), State::PRIMARY_STATE_UNCONFIGURED);

  // ==========================================================================
  // [Act & Assert] 1. Unconfigured -> Inactive (Configure)
  // ==========================================================================
  // This triggers init_parameters() and creates the driver instance.
  auto result_conf = node->trigger_transition(
      rclcpp_lifecycle::Transition(Transition::TRANSITION_CONFIGURE));
  EXPECT_EQ(result_conf.id(), State::PRIMARY_STATE_INACTIVE)
      << "Failed to transition from UNCONFIGURED to INACTIVE.";

  // ==========================================================================
  // [Act & Assert] 2. Inactive -> Active (Activate)
  // ==========================================================================
  // This starts the scan thread and enables publishers.
  auto result_act = node->trigger_transition(
      rclcpp_lifecycle::Transition(Transition::TRANSITION_ACTIVATE));
  EXPECT_EQ(result_act.id(), State::PRIMARY_STATE_ACTIVE)
      << "Failed to transition from INACTIVE to ACTIVE.";

  // ==========================================================================
  // [Act & Assert] 3. Active -> Inactive (Deactivate)
  // ==========================================================================
  // This stops the scan thread and disables publishers.
  auto result_deact = node->trigger_transition(
      rclcpp_lifecycle::Transition(Transition::TRANSITION_DEACTIVATE));
  EXPECT_EQ(result_deact.id(), State::PRIMARY_STATE_INACTIVE)
      << "Failed to transition from ACTIVE to INACTIVE.";

  // ==========================================================================
  // [Act & Assert] 4. Inactive -> Unconfigured (Cleanup)
  // ==========================================================================
  // This destroys the driver instance and releases resources.
  auto result_clean = node->trigger_transition(
      rclcpp_lifecycle::Transition(Transition::TRANSITION_CLEANUP));
  EXPECT_EQ(result_clean.id(), State::PRIMARY_STATE_UNCONFIGURED)
      << "Failed to transition from INACTIVE to UNCONFIGURED.";
}

/**
 * @brief [Integration Test] Scan Topic Publication
 * Verifies that the node actually publishes data to the '/scan' topic
 * when in the ACTIVE state.
 */
TEST_F(RplidarNodeTest, ScanPublicationCheck) {
  // ==========================================================================
  // [Arrange] Setup Node, QoS, and Promise/Future for async validation
  // ==========================================================================
  rclcpp::NodeOptions options;
  options.append_parameter_override("dummy_mode", true);
  options.append_parameter_override("frame_id", "test_laser_link");

  auto node = std::make_shared<rplidar_driver::RPlidarNode>(options);

  // Quickly transition to ACTIVE state
  node->trigger_transition(
      rclcpp_lifecycle::Transition(Transition::TRANSITION_CONFIGURE));
  node->trigger_transition(
      rclcpp_lifecycle::Transition(Transition::TRANSITION_ACTIVATE));

  // Prepare a promise to signal when a message is received
  auto promise = std::make_shared<std::promise<void>>();
  auto future = promise->get_future();
  bool message_received = false;

  // Create a subscriber to listen for the published scan
  auto sub = node->create_subscription<sensor_msgs::msg::LaserScan>(
      "scan", rclcpp::SensorDataQoS(),
      [&](sensor_msgs::msg::LaserScan::SharedPtr msg) {
        // ----------------------------------------------------------------------
        // [Assert] Callback Validation (Inside the Loop)
        // ----------------------------------------------------------------------
        message_received = true;

        // Check 1: Verify frame_id matches configuration
        EXPECT_EQ(msg->header.frame_id, "test_laser_link");

        // Check 2: Ensure data payload is present
        // Since we use the dummy driver, it should generate valid points.
        EXPECT_GT(msg->ranges.size(), 0) << "Received scan message is empty.";

        // Signal completion
        promise->set_value();
      });

  // ==========================================================================
  // [Act] Spin the node to process callbacks
  // ==========================================================================
  rclcpp::executors::SingleThreadedExecutor executor;
  executor.add_node(node->get_node_base_interface());

  auto start_time = std::chrono::steady_clock::now();
  // Wait up to 3 seconds for a message
  while (rclcpp::ok() && !message_received) {
    executor.spin_some();

    if (std::chrono::steady_clock::now() - start_time >
        std::chrono::seconds(3)) {
      break; // Timeout
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  // ==========================================================================
  // [Assert] Final Validation
  // ==========================================================================
  EXPECT_TRUE(message_received)
      << "Timeout: Failed to receive /scan topic within 3 seconds.";

  // Clean shutdown
  node->trigger_transition(
      rclcpp_lifecycle::Transition(Transition::TRANSITION_DEACTIVATE));
}
