//! @file
//!
//! Copyright (c) Memfault, Inc.
//! See License.txt for details
// Logging depends on how your configuration does logging. See
// https://docs.memfault.com/docs/embedded/self-serve/#implement-logging-dependency

#pragma once

#include <memfault/core/platform/debug_log.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MEMFAULT_PRINT_RESET_INFO(...) MEMFAULT_LOG_INFO(__VA_ARGS__)

#ifdef __cplusplus
}
#endif
