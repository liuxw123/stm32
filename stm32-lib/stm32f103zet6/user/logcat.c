#include "user/logcat.h"
#include "utils/bsp_utils.h"

#define LOG_TAG "logcat"
#define LOGCAT_HW_ENDPOINT_UART1 0x1000
#define LOGCAT_HW_ENDPOINT LOGCAT_HW_ENDPOINT_UART1

static bsp_handle_t sHandle = 0;

#if LOGCAT_HW_ENDPOINT == LOGCAT_HW_ENDPOINT_UART1
#include "bsp/bsp_uart.h"

static status_t __logcat_hw_uart_init() {
    bsp_uart_params_t param = {
        .enable_it_ = true,
        .baud_rate_ = 115200,
        .work_mode_ = UART_MODE_TX_RX,
    };
    bsp_handle_t handle = bsp_uart_open(BSP_UART1, &param);
    if (handle != null_handle) sHandle = handle;
}

static status_t __logcat_hw_uart_exit() {
    return bsp_uart_close(sHandle);
}
#endif


void send_string(char* str) {
#if LOGCAT_HW_ENDPOINT == LOGCAT_HW_ENDPOINT_UART1
    bsp_uart_send(str, sHandle);
#endif
}

static status_t __logcat_hw_init() {
#if LOGCAT_HW_ENDPOINT == LOGCAT_HW_ENDPOINT_UART1
    return __logcat_hw_uart_init();
#else
    error
#endif
}

status_t init_logcat() {
    return __logcat_hw_init();
}

static status_t __logcat_hw_exit() {
#if LOGCAT_HW_ENDPOINT == LOGCAT_HW_ENDPOINT_UART1
    return __logcat_hw_uart_exit();
#else
    error
#endif
}

status_t exit_logcat() {
    return __logcat_hw_exit();
}