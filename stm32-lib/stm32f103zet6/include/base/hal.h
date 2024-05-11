#ifndef HAL_H
#define HAL_H

void hal_gpio_rcc_enable(uint32_t gpio, bool enable);
uint32_t hal_gpio_bank(uint32_t gpio);
uint32_t hal_gpio_pin(uint32_t gpio);
uint32_t hal_gpio_init(uint32_t gpio, uint32_t mode, uint32_t pull, uint32_t speed);
uint32_t hal_gpio_deinit(uint32_t gpio);

#endif // HAL_H
