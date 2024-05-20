#ifndef STM32F103ZET6_H
#define STM32F103ZET6_H

#define GPIO_BANK_A (0x0001 << 0)
#define GPIO_BANK_B (0x0001 << 1)
#define GPIO_BANK_C (0x0001 << 2)
#define GPIO_BANK_D (0x0001 << 3)
#define GPIO_BANK_E (0x0001 << 4)
#define GPIO_BANK_F (0x0001 << 5)
#define GPIO_BANK_G (0x0001 << 6)

#define GPIO_PIN0 (0x0001 << 0)
#define GPIO_PIN1 (0x0001 << 1)
#define GPIO_PIN2 (0x0001 << 2)
#define GPIO_PIN3 (0x0001 << 3)
#define GPIO_PIN4 (0x0001 << 4)
#define GPIO_PIN5 (0x0001 << 5)
#define GPIO_PIN6 (0x0001 << 6)
#define GPIO_PIN7 (0x0001 << 7)
#define GPIO_PIN8 (0x0001 << 8)
#define GPIO_PIN9 (0x0001 << 9)
#define GPIO_PIN10 (0x0001 << 10)
#define GPIO_PIN11 (0x0001 << 11)
#define GPIO_PIN12 (0x0001 << 12)
#define GPIO_PIN13 (0x0001 << 13)
#define GPIO_PIN14 (0x0001 << 14)
#define GPIO_PIN15 (0x0001 << 15)

#define __GPIO_PORT__(bank, pin) ((((uint32_t) bank) << 16) | pin)
#define GPIO_BANK(gpio_port) (gpio_port >> 16)
#define GPIO_PIN(gpio_port) (gpio_port & 0x0000FFFF)


#endif // STM32F103ZET6_H