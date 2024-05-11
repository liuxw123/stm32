#include "bsp/bsp_led.h"
#include "utils/bsp_utils.h"
#include "user/logcat.h"

#define LOG_TAG "bsp_led"

static void __bsp_led_set_state(uint32_t led, bsp_led_state_t state) {
    HAL_GPIO_WritePin((GPIO_TypeDef*) hal_gpio_bank(led), hal_gpio_pin(led), state);
}

static bsp_led_state_t __bsp_led_get_state(uint32_t led) {
    return HAL_GPIO_ReadPin((GPIO_TypeDef*) hal_gpio_bank(led), hal_gpio_pin(led));
}

static void __bsp_led_toggle(uint32_t led) {
    return HAL_GPIO_TogglePin((GPIO_TypeDef*) hal_gpio_bank(led), hal_gpio_pin(led));
}

status_t bsp_led_on(uint32_t led_flag) {
    led_flag = led_flag & LED_FLAG_MASK;
    if (led_flag & 0x00000001) __bsp_led_set_state(LED1_LIGHT_PIN, LED_STATE_ON);
    if (led_flag & 0x00000002) __bsp_led_set_state(LED2_LIGHT_PIN, LED_STATE_ON);
    if (led_flag & 0x00000004) __bsp_led_set_state(LED3_LIGHT_PIN, LED_STATE_ON);
    if (led_flag & 0x00000008) __bsp_led_set_state(LED4_LIGHT_PIN, LED_STATE_ON);

    return NO_ERROR;
}

status_t bsp_led_off(uint32_t led_flag) {
    led_flag = led_flag & LED_FLAG_MASK;
    if (led_flag & 0x00000001) __bsp_led_set_state(LED1_LIGHT_PIN, LED_STATE_OFF);
    if (led_flag & 0x00000002) __bsp_led_set_state(LED2_LIGHT_PIN, LED_STATE_OFF);
    if (led_flag & 0x00000004) __bsp_led_set_state(LED3_LIGHT_PIN, LED_STATE_OFF);
    if (led_flag & 0x00000008) __bsp_led_set_state(LED4_LIGHT_PIN, LED_STATE_OFF);
    return NO_ERROR;
}

status_t bsp_led_toggle(uint32_t led_flag) {
    led_flag = led_flag & LED_FLAG_MASK;
    if (led_flag & 0x00000001) __bsp_led_toggle(LED1_LIGHT_PIN);
    if (led_flag & 0x00000002) __bsp_led_toggle(LED2_LIGHT_PIN);
    if (led_flag & 0x00000004) __bsp_led_toggle(LED3_LIGHT_PIN);
    if (led_flag & 0x00000008) __bsp_led_toggle(LED4_LIGHT_PIN);

    return NO_ERROR;
}

uint32_t bsp_led_state(uint32_t led_flag) {
    uint32_t state = 0;

    if ((led_flag & 0x00000001) && (__bsp_led_get_state(LED1_LIGHT_PIN) == LED_STATE_ON)) state |= 0x00000001;
    if ((led_flag & 0x00000002) && (__bsp_led_get_state(LED2_LIGHT_PIN) == LED_STATE_ON)) state |= 0x00000002;
    if ((led_flag & 0x00000004) && (__bsp_led_get_state(LED3_LIGHT_PIN) == LED_STATE_ON)) state |= 0x00000004;
    if ((led_flag & 0x00000008) && (__bsp_led_get_state(LED4_LIGHT_PIN) == LED_STATE_ON)) state |= 0x00000008;

    return state;
}

static status_t __init_led_gpio(uint32_t led) {
    status_t ret = hal_gpio_init(led, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH);
    if (ret == NO_ERROR) {
        bsp_led_off(led);
    }
    return ret;
}

static status_t __exit_led_gpio(uint32_t led) {
    return hal_gpio_deinit(led);
}

status_t init_bsp_led() {
    ALOGD("GPIOA: 0x%08X, GPIOB: 0x%08X, GPIOC: 0x%08X, GPIOD: 0x%08X, GPIOE: 0x%08X, GPIOF: 0x%08X, GPIOG: 0x%08X", GPIOA_BASE, GPIOB_BASE, GPIOC_BASE, GPIOD_BASE, GPIOE_BASE, GPIOF_BASE, GPIOG_BASE);
    ALOGD("PIN0: 0x%08X, PIN1: 0x%08X, PIN2: 0x%08X, PIN3: 0x%08X, PIN4: 0x%08X, PIN5: 0x%08X, PIN6: 0x%08X, PIN7: 0x%08X", GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3, GPIO_PIN_4, GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7);
    ALOGD("PIN8: 0x%08X, PIN9: 0x%08X, PIN10: 0x%08X, PIN11: 0x%08X, PIN12: 0x%08X, PIN13: 0x%08X, PIN14: 0x%08X, PIN15: 0x%08X", GPIO_PIN_8, GPIO_PIN_9, GPIO_PIN_10, GPIO_PIN_11, GPIO_PIN_12, GPIO_PIN_13, GPIO_PIN_14, GPIO_PIN_15);
    ALOGD("led1(G6): 0x%08X-0x%08X, led2(A8): 0x%08X-0x%08X, led3(B8): 0x%08X-0x%08X, led4(B9): 0x%08X-0x%08X", hal_gpio_bank(LED1_LIGHT_PIN), hal_gpio_pin(LED1_LIGHT_PIN), hal_gpio_bank(LED2_LIGHT_PIN), hal_gpio_pin(LED2_LIGHT_PIN), hal_gpio_bank(LED3_LIGHT_PIN), hal_gpio_pin(LED3_LIGHT_PIN), hal_gpio_bank(LED4_LIGHT_PIN), hal_gpio_pin(LED4_LIGHT_PIN));

    __init_led_gpio(LED1_LIGHT_PIN);
    __init_led_gpio(LED2_LIGHT_PIN);
    __init_led_gpio(LED3_LIGHT_PIN);
    __init_led_gpio(LED4_LIGHT_PIN);

    /**
     * @brief test api
     * 
     */
    bsp_led_on(0xF);  // all on
    bsp_led_off(0xC); // all off
    // bsp_led_toggle(0xF); // all on

    uint32_t state = bsp_led_state(0xF); // state = 0xF
    if (state == 0x3) {
        ALOGD("[SUCCESS] init led");
        bsp_led_off(0xF);
        return NO_ERROR;
    } else {
        ALOGE("led state error. 0x3 is expected, but got: 0x%02X", state);
        return EHW;
    }
}

status_t exit_bsp_led() {
    __exit_led_gpio(LED1_LIGHT_PIN);
    __exit_led_gpio(LED2_LIGHT_PIN);
    __exit_led_gpio(LED3_LIGHT_PIN);
    __exit_led_gpio(LED4_LIGHT_PIN);
    return NO_ERROR;
}