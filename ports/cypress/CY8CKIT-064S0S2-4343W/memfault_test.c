#include <memfault_test.h>

//! Triggers an immediate heartbeat capture (instead of waiting for timer
//! to expire)
int test_heartbeat(int argc, char *argv[]) {
  memfault_metrics_heartbeat_debug_trigger();
  return 0;
}

//! Triggers a trace event
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
