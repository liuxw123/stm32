#ifndef BSP_UART_H
#define BSP_UART_H

#include "base/base.h"
#include "stm32f1xx_hal.h"

#define BSP_UART1 0x0
#define BSP_UART2 0x1
#define BSP_UART_MAX (BSP_UART2 + 1)

#define UART_RX_MAX_BUFFER_SIZE (256)

/**
 * @brief UART pin
 * 
 */
#define UART1_TXD_PIN __GPIO_PORT__(GPIO_BANK_A, GPIO_PIN9)
#define UART1_RXD_PIN __GPIO_PORT__(GPIO_BANK_A, GPIO_PIN10)
#define UART2_TXD_PIN __GPIO_PORT__(GPIO_BANK_A, GPIO_PIN2)
#define UART2_RXD_PIN __GPIO_PORT__(GPIO_BANK_A, GPIO_PIN3)

typedef struct {
    uint32_t baud_rate_;
    uint32_t work_mode_;
    bool enable_it_;
} bsp_uart_params_t;

status_t bsp_uart_send(char* str, bsp_handle_t uart);
status_t bsp_uart_send_it(char* str, bsp_handle_t uart);
status_t bsp_uart_recv_it(uint8_t* data, bsp_handle_t uart);
bsp_handle_t bsp_uart_open(uint32_t which, bsp_uart_params_t* params);
status_t bsp_uart_close(bsp_handle_t which);
status_t bsp_uart_send_with_recv(uint8_t* send, uint8_t* recv, bsp_handle_t uart);

#endif