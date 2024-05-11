#ifndef BSP_UTILS_H
#define BSP_UTILS_H

#include "base/base.h"
#include "stm32f1xx_hal.h"


status_t gpio_rcc_enable(uint32_t gpiox, bool enable);

#endif