#pragma once

#include <cyhal_flash.h>

/* Flash block object. Has to be initialized before use! */
extern cyhal_flash_t flash_obj;
extern cyhal_flash_info_t flash_obj_info;

/* Coredumps flash section start/end addresses. Has to be defined before use! */
extern uint8_t __MfltCoredumpsStart;
extern uint8_t __MfltCoredumpsEnd;
