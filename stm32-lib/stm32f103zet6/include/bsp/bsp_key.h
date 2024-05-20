#ifndef BSP_KEY_H
#define BSP_KEY_H

#include "stm32f1xx_hal.h"

#include "base/base.h"
#include "base/stm32f103zet6.h"

status_t init_bsp_key();
status_t exit_bsp_key();
#endif // BSP_KEY_H