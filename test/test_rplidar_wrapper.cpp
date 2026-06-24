#include "lidar_driver_wrapper.hpp"
#include <cmath>
#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <vector>

// ============================================================================
//  Mock Driver Test (Logic & FSM Verification)
// ============================================================================
class MockDriverTest : public ::testing::Test {
protected:
  std::unique_ptr<rplidar_driver::LidarDriverInterface> driver;

  void SetUp() override {
    driver = std::make_unique<rplidar_driver::DummyLidarDriver>();
  }

  void TearDown() override {
    if (driver && driver->isConnected()) {
      driver->disconnect();
    }
  }
};
/**
 * @brief [Happy Path] Verifies data acquisition logic using Mock driver.
 * * Ensures that:
 * 1. The driver transitions to scanning state correctly.
 * 2. Retrieved data is not empty and contains valid sensor readings.
 * 3. Data mimics expected behavior (not frozen/all zeros).
 */
TEST_F(MockDriverTest, ScanDataSanityCheck) {
  // ==========================================================
  // 1. Arrange: Connect to mock port and start motor
  // ==========================================================
  // Mock driver always returns success for connection
  ASSERT_TRUE(driver->connect("/dev/mock_port", 115200));

  // Start motor with standard mode.
  // Expectation: Internal state changes to SCANNING.
  ASSERT_TRUE(driver->start_motor("Standard", 600));

  // ==========================================================
  // 2. Act: Request scan data
  // ==========================================================
  std::vector<sl_lidar_response_measurement_node_hq_t> nodes;
  bool grab_result = driver->grab_scan_data(nodes);

  // ==========================================================
  // 3. Assert: Validate data integrity
  // ==========================================================
  // Check 1: Function returned success
  EXPECT_TRUE(grab_result) << "Driver failed to return data packet.";

  // Check 2: Vector is populated
  EXPECT_GT(nodes.size(), 0) << "Received empty data vector.";

  // Check 3: Data Quality Check (Sanity)
  // Ensures we are not receiving a "dead" signal (all zeros or frozen values)
  bool found_nonzero = false;
  bool variation_detected = false;
  float first_dist = nodes[0].dist_mm_q2;

  for (size_t i = 1; i < nodes.size(); ++i) {
    if (nodes[i].dist_mm_q2 > 0)
      found_nonzero = true;
    if (nodes[i].dist_mm_q2 != first_dist)
      variation_detected = true;
  }

  EXPECT_TRUE(found_nonzero) << "FAIL: All sensor data is zero (Blind Sensor).";
  EXPECT_TRUE(variation_detected)
      << "FAIL: All sensor data is identical (Frozen Data).";

  // Cleanup: Stop motor explicitly to verify state transition
  driver->stop_motor();
  EXPECT_FALSE(driver->grab_scan_data(nodes))
      << "Should not allow data grab after stop.";
}
