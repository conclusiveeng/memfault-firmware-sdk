#pragma once

#include <memfault/ports/freertos.h>
#include <memfault/components.h>

// Triggers an immediate heartbeat capture (instead of waiting for timer
// to expire)
int test_heartbeat(int argc, char *argv[]);

// Trigger a trace event
int test_trace(int argc, char *argv[]);

// Trigger a user initiated reboot and confirm reason is persisted
int test_reboot(int argc, char *argv[]);

// Test different crash types where a coredump should be captured
int test_assert(int argc, char *argv[]);
int test_fault(void);
int test_hang(int argc, char *argv[]);

// Dump Memfault data collected to console
int test_export(int argc, char *argv[]);

// Send data over selected transport
bool try_send_memfault_data(void);
void send_memfault_data(void);