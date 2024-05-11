#ifndef BSP_LED_H
#define BSP_LED_H

#include "stm32f1xx_hal.h"

#include "base/base.h"
#include "base/stm32f103zet6.h"

#define LED_FLAG_MASK (0x0000000F)

/**
 * @brief LED gpio
 * 
 */
#define LED1_LIGHT_PIN __GPIO_PORT__(GPIO_BANK_G, GPIO_PIN6)
#define LED2_LIGHT_PIN __GPIO_PORT__(GPIO_BANK_A, GPIO_PIN8)
#define LED3_LIGHT_PIN __GPIO_PORT__(GPIO_BANK_B, GPIO_PIN8)
#define LED4_LIGHT_PIN __GPIO_PORT__(GPIO_BANK_B, GPIO_PIN9)

typedef enum {
    LED_STATE_ON = GPIO_PIN_RESET,
    LED_STATE_OFF = GPIO_PIN_SET,
} bsp_led_state_t;

status_t bsp_led_on(uint32_t led_flag);
status_t bsp_led_off(uint32_t led_flag);
status_t bsp_led_toggle(uint32_t led_flag);
uint32_t bsp_led_state(uint32_t led_flag);
status_t init_bsp_led();
status_t exit_bsp_led();
#endif
