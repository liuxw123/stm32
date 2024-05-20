#ifndef BSP_FSMC_LCD_H
#define BSP_FSMC_LCD_H

#include "stm32f1xx_hal.h"

#include "base/base.h"
#include "base/stm32f103zet6.h"

#define Bank1_LCD_C ((uint32_t) 0x6C000000) //Disp Reg ADDR
#define Bank1_LCD_D ((uint32_t) 0x6C000800) //Disp Data ADDR

#define FSMC_CMD(cmd) ((*(__IO uint16_t*) (Bank1_LCD_C)) = ((uint16_t) cmd))
#define FSMC_Parameter(val) ((*(__IO uint16_t*) (Bank1_LCD_D)) = ((uint16_t) (val)))


status_t init_fsmc();
#endif // BSP_FSMC_LCD_H