#include "bsp/bsp_uart.h"
#include "base/stm32f103zet6.h"
#include "bsp/bsp_led.h"
#include "user/logcat.h"

#define LOG_TAG "bsp_uart"

typedef struct {
    UART_HandleTypeDef* handle_;
    bsp_uart_params_t params_;
    bool inited_;
    uint8_t* rx_buffer_;
    uint8_t* tx_buffer_;
    uint32_t rx_index_;
} uart_device_t;

static void __msp_uart_init(UART_HandleTypeDef* uart);
static void __msp_uart_deinit(UART_HandleTypeDef* uart);
void __tx_done_callback(UART_HandleTypeDef* uart);
void __rx_done_callback(UART_HandleTypeDef* uart);

static UART_HandleTypeDef sUarts[BSP_UART_MAX] = {
    {
        .Instance = USART1,
        .gState = HAL_UART_STATE_RESET,
        .Init = {
            .BaudRate = 115200,
            .HwFlowCtl = UART_HWCONTROL_NONE,
            .Mode = UART_MODE_TX,
            .WordLength = UART_WORDLENGTH_8B,
            .StopBits = UART_STOPBITS_1,
            .Parity = UART_PARITY_NONE,
            .OverSampling = UART_OVERSAMPLING_16,
        },
        .MspInitCallback = __msp_uart_init,
        .MspDeInitCallback = __msp_uart_deinit,
    },
    {
        .Instance = USART2,
        .gState = HAL_UART_STATE_RESET,
        .Init = {
            .BaudRate = 9600,
            .HwFlowCtl = UART_HWCONTROL_NONE,
            .Mode = UART_MODE_TX,
            .WordLength = UART_WORDLENGTH_8B,
            .StopBits = UART_STOPBITS_1,
            .Parity = UART_PARITY_NONE,
            .OverSampling = UART_OVERSAMPLING_16,
        },
        .MspInitCallback = __msp_uart_init,
        .MspDeInitCallback = __msp_uart_deinit,
    },
};

static uart_device_t sUartDevice[BSP_UART_MAX] = {
    {
        .handle_ = sUarts,
        .inited_ = false,
    },
    {
        .handle_ = sUarts + 1,
        .inited_ = false,
    },
};

static uint8_t sRecvChar;

status_t bsp_uart_send(char* str, bsp_handle_t uart) {
    if (!str || !uart) return EBADPARAM;
    uart_device_t* handle = (uart_device_t*) uart;
    if (!handle->inited_) return ESTATE;

    HAL_StatusTypeDef ret = HAL_UART_Transmit(handle->handle_, (const uint8_t*) str, strlen(str), 0xFFFF);
    return ret == HAL_OK ? NO_ERROR : EUNKNOW;
}

static void __wait_uart_done(uart_device_t* uart, bool rx) {
    if (rx) {
        while (!__HAL_UART_GET_FLAG(uart->handle_, UART_FLAG_IDLE));
        __HAL_UART_CLEAR_IDLEFLAG(uart->handle_);
    } else
        while (!__HAL_UART_GET_FLAG(uart->handle_, UART_FLAG_TXE));
}

status_t bsp_uart_send_it(char* str, bsp_handle_t uart) {
    if (!str || !uart) return EBADPARAM;
    uart_device_t* handle = (uart_device_t*) uart;
    if (!handle->inited_) return ESTATE;

    HAL_StatusTypeDef ret = HAL_UART_Transmit_IT(handle->handle_, (const uint8_t*) str, strlen(str));
    if (ret == HAL_OK) {
        __wait_uart_done(handle, false);
        return NO_ERROR;
    }
    return EHW;
}

status_t bsp_uart_recv_it(uint8_t* data, bsp_handle_t uart) {
    if (!data || !uart) return EBADPARAM;
    uart_device_t* handle = (uart_device_t*) uart;
    if (!handle->inited_) return ESTATE;

    handle->rx_buffer_ = data;
    handle->rx_index_ = 0;
    HAL_StatusTypeDef ret = HAL_UART_Receive_IT(handle->handle_, &sRecvChar, 1);
    if (ret == HAL_OK) {
        __wait_uart_done(handle, true);
        return NO_ERROR;
    }
    return EHW;
}

status_t bsp_uart_send_with_recv(uint8_t* send, uint8_t* recv, bsp_handle_t uart) {
    if (!send || !recv || !uart) return EBADPARAM;
    uart_device_t* handle = (uart_device_t*) uart;
    if (!handle->inited_) return ESTATE;

    HAL_StatusTypeDef ret = HAL_OK;

    handle->rx_buffer_ = recv;
    handle->rx_index_ = 0;
    ret = HAL_UART_Transmit_IT(handle->handle_, (const uint8_t*) send, strlen(send));
    __wait_uart_done(handle, false);
    ret = HAL_UART_Receive_IT(handle->handle_, &sRecvChar, 1);
#if 0
        uint32_t state0 = __HAL_UART_GET_FLAG(handle->handle_, UART_FLAG_CTS);
        uint32_t state1 = __HAL_UART_GET_FLAG(handle->handle_, UART_FLAG_LBD);
        uint32_t state2 = __HAL_UART_GET_FLAG(handle->handle_, UART_FLAG_TC);
        uint32_t state3 = __HAL_UART_GET_FLAG(handle->handle_, UART_FLAG_TXE);
        uint32_t state4 = __HAL_UART_GET_FLAG(handle->handle_, UART_FLAG_RXNE);
        uint32_t state5 = __HAL_UART_GET_FLAG(handle->handle_, UART_FLAG_IDLE);
        uint32_t state6 = __HAL_UART_GET_FLAG(handle->handle_, UART_FLAG_ORE);
        uint32_t state7 = __HAL_UART_GET_FLAG(handle->handle_, UART_FLAG_NE);
        uint32_t state8 = __HAL_UART_GET_FLAG(handle->handle_, UART_FLAG_FE);
        uint32_t state9 = __HAL_UART_GET_FLAG(handle->handle_, UART_FLAG_PE);

        ALOGD("CTS:%d, LBD:%d, TC:%d, TXE:%d, RXNE:%d, IDLE:%d, ORE:%d, NE:%d, FE:%d, PE:%d", state1, state2, state3, state4, state5, state6, state7, state8, state9);
#endif
    __wait_uart_done(handle, true);
    return ret == HAL_OK ? NO_ERROR : EHW;
}


static void __msp_uart_init(UART_HandleTypeDef* uart) {
    switch ((uint32_t) (uart->Instance)) {
    case USART1_BASE:
        __HAL_RCC_USART1_CLK_ENABLE();
        hal_gpio_init(UART1_RXD_PIN, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH);
        hal_gpio_init(UART1_TXD_PIN, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH);
        if (sUartDevice->params_.enable_it_) {
            HAL_NVIC_SetPriority(USART1_IRQn, 1, 1);
            HAL_NVIC_EnableIRQ(USART1_IRQn);
            sUartDevice->handle_->RxCpltCallback = __rx_done_callback;
            sUartDevice->handle_->TxCpltCallback = __tx_done_callback;
        }
        break;
    case USART2_BASE:
        __HAL_RCC_USART2_CLK_ENABLE();
        hal_gpio_init(UART2_RXD_PIN, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH);
        hal_gpio_init(UART2_TXD_PIN, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH);
        if ((sUartDevice + 1)->params_.enable_it_) {
            HAL_NVIC_SetPriority(USART2_IRQn, 1, 1);
            HAL_NVIC_EnableIRQ(USART2_IRQn);
            (sUartDevice + 1)->handle_->RxCpltCallback = __rx_done_callback;
            (sUartDevice + 1)->handle_->TxCpltCallback = __tx_done_callback;
        }
        break;
    default:
        return;
    }
}

static void __msp_uart_deinit(UART_HandleTypeDef* uart) {
    uint32_t gpiox = 0;
    uint32_t rx_pin, tx_pin;

    switch ((uint32_t) uart->Instance) {
    case USART1_BASE:
        gpiox = hal_gpio_bank(UART1_RXD_PIN);
        rx_pin = hal_gpio_pin(UART1_RXD_PIN);
        tx_pin = hal_gpio_pin(UART1_TXD_PIN);
        __HAL_RCC_USART1_CLK_DISABLE();
        hal_gpio_deinit(UART1_RXD_PIN);
        hal_gpio_deinit(UART1_TXD_PIN);
        if (sUartDevice->params_.enable_it_) {
            HAL_NVIC_DisableIRQ(USART1_IRQn);
            sUartDevice->handle_->RxCpltCallback = null;
            sUartDevice->handle_->TxCpltCallback = null;
        }
        break;
    case USART2_BASE:
        gpiox = hal_gpio_bank(UART2_RXD_PIN);
        rx_pin = hal_gpio_pin(UART2_RXD_PIN);
        tx_pin = hal_gpio_pin(UART2_TXD_PIN);
        __HAL_RCC_USART2_CLK_DISABLE();
        hal_gpio_deinit(UART2_RXD_PIN);
        hal_gpio_deinit(UART2_TXD_PIN);
        if ((sUartDevice + 1)->params_.enable_it_) {
            HAL_NVIC_DisableIRQ(USART2_IRQn);
            (sUartDevice + 1)->handle_->RxCpltCallback = null;
            (sUartDevice + 1)->handle_->TxCpltCallback = null;
        }
        break;
    default:
        return;
    }
}

bsp_handle_t bsp_uart_open(uint32_t which, bsp_uart_params_t* params) {
    if (!params) return null_handle;

    uart_device_t* handle = null;
    switch (which) {
    case BSP_UART1:
    case BSP_UART2:
        handle = sUartDevice + which;
        handle->handle_ = sUarts + which;
        break;
    default: break;
    }
    if (!handle || handle->inited_) return null_handle;

    handle->params_.baud_rate_ = params->enable_it_;
    handle->params_.work_mode_ = params->work_mode_;
    handle->params_.enable_it_ = params->enable_it_;

    handle->handle_->Init.BaudRate = params->baud_rate_;
    handle->handle_->Init.Mode = params->work_mode_;

    HAL_StatusTypeDef ret = HAL_UART_Init(handle->handle_);
    if (ret == HAL_OK) {
        handle->inited_ = true;
        ALOGD("init success. baudrate: %d, mode: %s", handle->handle_->Init.BaudRate, (handle->handle_->Init.Mode == UART_MODE_TX) ? "TX" : (handle->handle_->Init.Mode == UART_MODE_RX) ? "RX"
                                                                                                                                                                                         : "RX_TX");
        return (bsp_handle_t) handle;
    }

    ALOGE("init failed.");
    return null_handle;
}

status_t bsp_uart_close(bsp_handle_t which) {
    if (!which) return EBADPARAM;
    uart_device_t* handle = (uart_device_t*) which;
    if (!handle->inited_) return EALREADY;

    HAL_StatusTypeDef ret = HAL_UART_DeInit(handle->handle_);
    handle->inited_ = false;
    return (ret == HAL_OK) ? NO_ERROR : EHW;
}

void USART1_IRQHandler(void) {
    HAL_UART_IRQHandler(sUartDevice->handle_);
}

void USART2_IRQHandler(void) {
    HAL_UART_IRQHandler((sUartDevice + 1)->handle_);
}

void __tx_done_callback(UART_HandleTypeDef* uart) {
    return;
}

void __rx_done_callback(UART_HandleTypeDef* uart) {
    uart_device_t* handle = null;
    switch ((uint32_t) uart->Instance) {
    case USART1_BASE: handle = sUartDevice; break;
    case USART2_BASE: handle = sUartDevice + 1; break;
    default:
        return;
    }

    handle->rx_buffer_[handle->rx_index_++] = sRecvChar;
    HAL_UART_Receive_IT(handle->handle_, &sRecvChar, 1);
}