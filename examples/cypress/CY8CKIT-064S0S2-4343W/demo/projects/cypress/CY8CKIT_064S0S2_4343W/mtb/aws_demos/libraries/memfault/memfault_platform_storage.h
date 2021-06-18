#pragma once

#include <cyhal_flash.h>

/* Flash block object */
extern cyhal_flash_t flash_obj;
extern cyhal_flash_info_t flash_obj_info;

/* Coredumps section addresses from the linker script. */
extern uint8_t __MfltCoredumpsStart;
extern uint8_t __MfltCoredumpsEnd;
