#include "base/stm32f103zet6.h"
#include "base/base.h"
#include "stm32f1xx_hal.h"


static __hal_gpioa_rcc_enable(bool enable) {
    if (enable && !(__HAL_RCC_GPIOA_IS_CLK_ENABLED())) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    } else if ((!enable) && __HAL_RCC_GPIOA_IS_CLK_ENABLED()) {
        __HAL_RCC_GPIOA_CLK_DISABLE();
    }
}

static __hal_gpiob_rcc_enable(bool enable) {
    if (enable && !(__HAL_RCC_GPIOB_IS_CLK_ENABLED())) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    } else if ((!enable) && __HAL_RCC_GPIOB_IS_CLK_ENABLED()) {
        __HAL_RCC_GPIOB_CLK_DISABLE();
    }
}

static __hal_gpioc_rcc_enable(bool enable) {
    if (enable && !(__HAL_RCC_GPIOC_IS_CLK_ENABLED())) {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    } else if ((!enable) && __HAL_RCC_GPIOC_IS_CLK_ENABLED()) {
        __HAL_RCC_GPIOC_CLK_DISABLE();
    }
}

static __hal_gpiod_rcc_enable(bool enable) {
    if (enable && !(__HAL_RCC_GPIOD_IS_CLK_ENABLED())) {
        __HAL_RCC_GPIOD_CLK_ENABLE();
    } else if ((!enable) && __HAL_RCC_GPIOD_IS_CLK_ENABLED()) {
        __HAL_RCC_GPIOD_CLK_DISABLE();
    }
}

static __hal_gpioe_rcc_enable(bool enable) {
    if (enable && !(__HAL_RCC_GPIOE_IS_CLK_ENABLED())) {
        __HAL_RCC_GPIOE_CLK_ENABLE();
    } else if ((!enable) && __HAL_RCC_GPIOE_IS_CLK_ENABLED()) {
        __HAL_RCC_GPIOE_CLK_DISABLE();
    }
}

static __hal_gpiof_rcc_enable(bool enable) {
    if (enable && !(__HAL_RCC_GPIOF_IS_CLK_ENABLED())) {
        __HAL_RCC_GPIOF_CLK_ENABLE();
    } else if ((!enable) && __HAL_RCC_GPIOF_IS_CLK_ENABLED()) {
        __HAL_RCC_GPIOF_CLK_DISABLE();
    }
}

static __hal_gpiog_rcc_enable(bool enable) {
    if (enable && !(__HAL_RCC_GPIOG_IS_CLK_ENABLED())) {
        __HAL_RCC_GPIOG_CLK_ENABLE();
    } else if ((!enable) && __HAL_RCC_GPIOG_IS_CLK_ENABLED()) {
        __HAL_RCC_GPIOG_CLK_DISABLE();
    }
}

void hal_gpio_rcc_enable(uint32_t gpio, bool enable) {
    uint32_t bank = GPIO_BANK(gpio);

    if (bank & GPIO_BANK_A) __hal_gpioa_rcc_enable(enable);
    if (bank & GPIO_BANK_B) __hal_gpiob_rcc_enable(enable);
    if (bank & GPIO_BANK_C) __hal_gpioc_rcc_enable(enable);
    if (bank & GPIO_BANK_D) __hal_gpiod_rcc_enable(enable);
    if (bank & GPIO_BANK_E) __hal_gpioe_rcc_enable(enable);
    if (bank & GPIO_BANK_F) __hal_gpiof_rcc_enable(enable);
    if (bank & GPIO_BANK_G) __hal_gpiog_rcc_enable(enable);
}

uint32_t hal_gpio_bank(uint32_t gpio) {
    uint32_t bank = GPIO_BANK(gpio);

    switch (bank) {
    case GPIO_BANK_A: return GPIOA_BASE;
    case GPIO_BANK_B: return GPIOB_BASE;
    case GPIO_BANK_C: return GPIOC_BASE;
    case GPIO_BANK_D: return GPIOD_BASE;
    case GPIO_BANK_E: return GPIOE_BASE;
    case GPIO_BANK_F: return GPIOF_BASE;
    case GPIO_BANK_G: return GPIOG_BASE;
    default: return 0;
    }
}

uint32_t hal_gpio_pin(uint32_t gpio) {
    uint32_t pin = GPIO_PIN(gpio);
    uint32_t res = 0;
    if (pin & GPIO_PIN0) res |= GPIO_PIN_0;
    if (pin & GPIO_PIN1) res |= GPIO_PIN_1;
    if (pin & GPIO_PIN2) res |= GPIO_PIN_2;
    if (pin & GPIO_PIN3) res |= GPIO_PIN_3;
    if (pin & GPIO_PIN4) res |= GPIO_PIN_4;
    if (pin & GPIO_PIN5) res |= GPIO_PIN_5;
    if (pin & GPIO_PIN6) res |= GPIO_PIN_6;
    if (pin & GPIO_PIN7) res |= GPIO_PIN_7;
    if (pin & GPIO_PIN8) res |= GPIO_PIN_8;
    if (pin & GPIO_PIN9) res |= GPIO_PIN_9;
    if (pin & GPIO_PIN10) res |= GPIO_PIN_10;
    if (pin & GPIO_PIN11) res |= GPIO_PIN_11;
    if (pin & GPIO_PIN12) res |= GPIO_PIN_12;
    if (pin & GPIO_PIN13) res |= GPIO_PIN_13;
    if (pin & GPIO_PIN14) res |= GPIO_PIN_14;
    if (pin & GPIO_PIN15) res |= GPIO_PIN_15;

    return res;
}

bool hal_gpio_compare_pin(uint32_t gpio, uint32_t pin) {
    return GPIO_PIN(gpio) == pin;
}

uint32_t hal_gpio_exti(uint32_t gpio) {
    uint32_t pin = GPIO_PIN(gpio);

    switch (pin) {
    case GPIO_PIN0: return EXTI0_IRQn;
    case GPIO_PIN1: return EXTI1_IRQn;
    case GPIO_PIN2: return EXTI2_IRQn;
    case GPIO_PIN3: return EXTI3_IRQn;
    case GPIO_PIN4: return EXTI4_IRQn;
    case GPIO_PIN5:
    case GPIO_PIN6:
    case GPIO_PIN7:
    case GPIO_PIN8:
    case GPIO_PIN9: return EXTI9_5_IRQn;
    case GPIO_PIN10:
    case GPIO_PIN11:
    case GPIO_PIN12:
    case GPIO_PIN13:
    case GPIO_PIN14:
    case GPIO_PIN15: return EXTI15_10_IRQn;
    default: return 0;
    }
}

void hal_gpio_toggle_state(uint32_t gpio) {
    uint32_t bank = hal_gpio_bank(gpio);
    uint32_t pin = hal_gpio_pin(gpio);

    if ((!gpio) || (!pin)) return EBADPARAM;
    HAL_GPIO_TogglePin((GPIO_TypeDef*) bank, pin);
}

void hal_gpio_set_state(uint32_t gpio, uint32_t state) {
    uint32_t bank = hal_gpio_bank(gpio);
    uint32_t pin = hal_gpio_pin(gpio);

    if ((!gpio) || (!pin)) return EBADPARAM;
    HAL_GPIO_WritePin((GPIO_TypeDef*) bank, pin, state);
}

uint32_t hal_gpio_get_state(uint32_t gpio) {
    uint32_t bank = hal_gpio_bank(gpio);
    uint32_t pin = hal_gpio_pin(gpio);

    if ((!gpio) || (!pin)) return EBADPARAM;
    return HAL_GPIO_ReadPin((GPIO_TypeDef*) bank, pin);
}

uint32_t hal_gpio_init(uint32_t gpio, uint32_t mode, uint32_t pull, uint32_t speed) {
    uint32_t bank = hal_gpio_bank(gpio);
    uint32_t pin = hal_gpio_pin(gpio);

    if ((!gpio) || (!pin)) return EBADPARAM;

    GPIO_InitTypeDef init = {
        .Pin = pin,
        .Mode = mode,
        .Pull = pull,
        .Speed = speed,
    };

    hal_gpio_rcc_enable(gpio, true);
    HAL_GPIO_Init((GPIO_TypeDef*) bank, &init);

    return NO_ERROR;
}

uint32_t hal_gpio_deinit(uint32_t gpio) {
    uint32_t bank = hal_gpio_bank(gpio);
    uint32_t pin = hal_gpio_pin(gpio);

    if ((!gpio) || (!pin)) return EBADPARAM;
    hal_gpio_rcc_enable(gpio, false);
    HAL_GPIO_DeInit((GPIO_TypeDef*) bank, pin);

    return NO_ERROR;
}