#include "bsp/bsp_key.h"
#include "user/logcat.h"

#define LOG_TAG "bsp_key"

/**
 * @brief key buttons gpio
 * 
 */
#define KEY1_BUTTON_PIN __GPIO_PORT__(GPIO_BANK_E, GPIO_PIN2)
#define KEY2_BUTTON_PIN __GPIO_PORT__(GPIO_BANK_E, GPIO_PIN3)
#define KEY3_BUTTON_PIN __GPIO_PORT__(GPIO_BANK_E, GPIO_PIN4)
#define KEY4_BUTTON_PIN __GPIO_PORT__(GPIO_BANK_E, GPIO_PIN5)

#define KEY1_IRQHandler EXTI2_IRQHandler
#define KEY2_IRQHandler EXTI3_IRQHandler
#define KEY3_IRQHandler EXTI4_IRQHandler
#define KEY4_IRQHandler EXTI9_5_IRQHandler

#define KEY_JITTER_MS (500)

typedef enum {
    KEY_RELEASED = GPIO_PIN_SET,
    KEY_PRESSED = GPIO_PIN_RESET,
} bsp_key_state_t;

typedef struct {
    uint32_t gpio_;
    uint32_t hal_pin_;
    uint32_t pressed_ts_;
    bool first_press_done_;
} bsp_key_info_t;

static bsp_key_info_t sKeysInfo[] = {
    {.gpio_ = KEY1_BUTTON_PIN},
    {.gpio_ = KEY2_BUTTON_PIN},
    {.gpio_ = KEY3_BUTTON_PIN},
    {.gpio_ = KEY4_BUTTON_PIN},
};

static status_t __init_key_gpio(uint32_t key) {
    uint32_t ret = hal_gpio_init(key, GPIO_MODE_IT_FALLING, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH);
    uint32_t irq = hal_gpio_exti(key);
    if (ret || !irq) {
        ALOGE("init failed.");
        return ret;
    }
    HAL_NVIC_SetPriority(irq, 2, 0);
    HAL_NVIC_EnableIRQ(irq);

    return NO_ERROR;
}

static status_t __exit_key_gpio(uint32_t key) {
    uint32_t ret = hal_gpio_deinit(key);
    uint32_t irq = hal_gpio_exti(key);
    if (ret || !irq) {
        ALOGE("init failed.");
        return ret;
    }
    HAL_NVIC_DisableIRQ(irq);

    return NO_ERROR;
}

status_t init_bsp_key() {
    for (uint32_t i = 0; i < ARRAY_SIZE(sKeysInfo); i++) {
        sKeysInfo[i].hal_pin_ = hal_gpio_pin(sKeysInfo[i].gpio_);
        __init_key_gpio(sKeysInfo[i].gpio_);
        sKeysInfo[i].first_press_done_ = false;
    }
    return NO_ERROR;
}

status_t exit_bsp_key() {
    for (uint32_t i = 0; i < ARRAY_SIZE(sKeysInfo); i++) {
        __exit_key_gpio(sKeysInfo[i].gpio_);
    }

    return NO_ERROR;
}

void KEY1_IRQHandler() {
    HAL_GPIO_EXTI_IRQHandler(sKeysInfo->hal_pin_);
}

void KEY2_IRQHandler() {
    HAL_GPIO_EXTI_IRQHandler((sKeysInfo + 1)->hal_pin_);
}

void KEY3_IRQHandler() {
    HAL_GPIO_EXTI_IRQHandler((sKeysInfo + 2)->hal_pin_);
}

void KEY4_IRQHandler() {
    HAL_GPIO_EXTI_IRQHandler((sKeysInfo + 3)->hal_pin_);
}

void HAL_GPIO_EXTI_Callback(uint16_t pin) {
    uint8_t which = 0;
    uint32_t ts = HAL_GetTick();
    bsp_key_info_t* key_info = null;

    for (uint32_t i = 0; i < ARRAY_SIZE(sKeysInfo); i++) {
        if (pin == sKeysInfo[i].hal_pin_) {
            which = '1' + i;
            key_info = sKeysInfo + i;
            break;
        }
    }

    if (!key_info) return;
    uint32_t state = hal_gpio_get_state(key_info->gpio_);

    if (state == KEY_PRESSED) {
        if (key_info->pressed_ts_ == 0) key_info->pressed_ts_ = ts;
        if (ts - key_info->pressed_ts_ >= KEY_JITTER_MS || (!key_info->first_press_done_)) {
            key_info->first_press_done_ = true;
            key_info->pressed_ts_ = ts;
            ALOGD("KEY%c pressed.", which);
        }
    }
}
