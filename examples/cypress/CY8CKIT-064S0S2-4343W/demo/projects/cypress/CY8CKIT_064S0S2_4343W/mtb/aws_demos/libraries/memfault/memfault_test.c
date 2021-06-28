#include <memfault_test.h>

#define MEMFAULT_CHUNK_SIZE 1500

// Triggers an immediate heartbeat capture (instead of waiting for timer
// to expire)
int test_heartbeat(int argc, char *argv[]) {
  memfault_metrics_heartbeat_debug_trigger();
  return 0;
}

int test_trace(int argc, char *argv[]) {
  MEMFAULT_TRACE_EVENT_WITH_LOG(critical_error, "A test error trace!");
  return 0;
}

//! Trigger a user initiated reboot and confirm reason is persisted
int test_reboot(int argc, char *argv[]) {
  memfault_reboot_tracking_mark_reset_imminent(kMfltRebootReason_UserReset, NULL);
  memfault_platform_reboot();
}

//
// Test different crash types where a coredump should be captured
//

int test_assert(int argc, char *argv[]) {
  MEMFAULT_ASSERT(0);
  return -1; // should never get here
}

int test_fault(void) {
  void (*bad_func)(void) = (void *)0xEEEEDEAD;
  bad_func();
  return -1; // should never get here
}

int test_hang(int argc, char *argv[]) {
  while (1) {}
  return -1; // should never get here
}

// Dump Memfault data collected to console
int test_export(int argc, char *argv[]) {
  memfault_data_export_dump_chunks();
  return 0;
}

//
// Send data over selected transport
//

bool try_send_memfault_data(void) {
  // buffer to copy chunk data into
  uint8_t buf[MEMFAULT_CHUNK_SIZE];
  size_t buf_len = sizeof(buf);

  bool data_available = memfault_packetizer_get_chunk(buf, &buf_len);
  if (!data_available ) {
    return false; // no more data to send
  }

  // send payload collected to chunks endpoint
  // user_transport_send_chunk_data(buf, buf_len);
  return true;
}

void send_memfault_data(void) {
  if (memfault_packetizer_data_available()) {
    return; // no new data to send
  }

  // [... user specific logic deciding when & how much data to send
  while (try_send_memfault_data()) { }
  return;
}
