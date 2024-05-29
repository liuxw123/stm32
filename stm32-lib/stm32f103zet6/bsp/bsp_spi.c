#include "bsp/bsp_spi.h"
#include "user/logcat.h"

#define LOG_TAG "bsp_spi"

#define SPI1_CS __GPIO_PORT__(GPIO_BANK_A, GPIO_PIN4)
#define SPI1_SCK __GPIO_PORT__(GPIO_BANK_A, GPIO_PIN5)
#define SPI1_MISO __GPIO_PORT__(GPIO_BANK_A, GPIO_PIN6)
#define SPI1_MOSI __GPIO_PORT__(GPIO_BANK_A, GPIO_PIN7)
#define SPI2_CS __GPIO_PORT__(GPIO_BANK_B, GPIO_PIN12)
#define SPI2_SCK __GPIO_PORT__(GPIO_BANK_B, GPIO_PIN13)
#define SPI2_MISO __GPIO_PORT__(GPIO_BANK_B, GPIO_PIN14)
#define SPI2_MOSI __GPIO_PORT__(GPIO_BANK_B, GPIO_PIN15)

#define SPI_CS_ENABLE_STATE GPIO_PIN_RESET
#define SPI_CS_DISABLE_STATE GPIO_PIN_SET

#define NUM_SPI_HW (BSP_SPI_MAX)

static void __msp_spi_init(SPI_HandleTypeDef* hspi);
static void __msp_spi_deinit(SPI_HandleTypeDef* hspi);
static void __spi_cs_enable(uint8_t spi_index, bool enable);

typedef void (*spi_cs_enable)(uint8_t spi_index, bool enable);

typedef struct {
    SPI_HandleTypeDef* handle_;
    bsp_spi_params_t params_;
    bool inited_;
    uint8_t spi_index_;
    spi_cs_enable cs_enable_;
} spi_device_t;


static SPI_HandleTypeDef sSpis[NUM_SPI_HW] = {
    {
        .Instance = SPI1,
        .Init = {
            .Mode = SPI_MODE_MASTER,
            .Direction = SPI_DIRECTION_2LINES,
            .DataSize = SPI_DATASIZE_8BIT,
            .CLKPolarity = SPI_POLARITY_LOW,
            .CLKPhase = SPI_PHASE_1EDGE,
            .NSS = SPI_NSS_SOFT,
            .BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2,
            .FirstBit = SPI_FIRSTBIT_MSB,
            .TIMode = SPI_TIMODE_DISABLE,
            .CRCCalculation = SPI_CRCCALCULATION_DISABLE,
            .CRCPolynomial = 10,
        },
        .MspInitCallback = __msp_spi_init,
        .MspDeInitCallback = __msp_spi_deinit,
    },
    {
        .Instance = SPI2,
        .Init = {
            .Mode = SPI_MODE_MASTER,
            .Direction = SPI_DIRECTION_2LINES,
            .DataSize = SPI_DATASIZE_8BIT,
            .CLKPolarity = SPI_POLARITY_LOW,
            .CLKPhase = SPI_PHASE_1EDGE,
            .NSS = SPI_NSS_SOFT,
            .BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2,
            .FirstBit = SPI_FIRSTBIT_MSB,
            .TIMode = SPI_TIMODE_DISABLE,
            .CRCCalculation = SPI_CRCCALCULATION_DISABLE,
            .CRCPolynomial = 10,
        },
        .MspInitCallback = __msp_spi_init,
        .MspDeInitCallback = __msp_spi_deinit,
    },
};
static spi_device_t sSpiDevice[NUM_SPI_HW] = {
    {
        .handle_ = sSpis,
        .inited_ = false,
        .spi_index_ = BSP_SPI1,
        .cs_enable_ = __spi_cs_enable,
    },
    {
        .handle_ = sSpis + 1,
        .inited_ = false,
        .spi_index_ = BSP_SPI2,
        .cs_enable_ = __spi_cs_enable,
    },
};

static void __spi_cs_enable(uint8_t spi_index, bool enable) {
    switch (spi_index) {
    case BSP_SPI1:
        hal_gpio_set_state(SPI1_CS, enable ? SPI_CS_ENABLE_STATE : SPI_CS_DISABLE_STATE);
        break;
    case BSP_SPI2:
        hal_gpio_set_state(SPI2_CS, enable ? SPI_CS_ENABLE_STATE : SPI_CS_DISABLE_STATE);
        break;
    default:
        break;
    }
}

void __spi_rx_done_callback(SPI_HandleTypeDef* hspi) {
}

void __spi_tx_done_callback(SPI_HandleTypeDef* hspi) {
}

static void __msp_spi_init(SPI_HandleTypeDef* hspi) {
    switch ((uint32_t) (hspi->Instance)) {
    case SPI1_BASE:
        __HAL_RCC_SPI1_CLK_ENABLE();
        hal_gpio_init(SPI1_CS, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH);
        hal_gpio_init(SPI1_SCK, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH);
        hal_gpio_init(SPI1_MISO, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH);
        hal_gpio_init(SPI1_MOSI, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH);
        if (sSpiDevice->params_.enable_it_) {
            HAL_NVIC_SetPriority(SPI1_IRQn, 1, 1);
            HAL_NVIC_EnableIRQ(SPI1_IRQn);
            sSpiDevice->handle_->RxCpltCallback = __spi_rx_done_callback;
            sSpiDevice->handle_->TxCpltCallback = __spi_tx_done_callback;
        }
        break;
    case SPI2_BASE:
        __HAL_RCC_SPI2_CLK_ENABLE();
        hal_gpio_init(SPI2_CS, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH);
        hal_gpio_init(SPI2_SCK, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH);
        hal_gpio_init(SPI2_MISO, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH);
        hal_gpio_init(SPI2_MOSI, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH);
        if ((sSpiDevice + 1)->params_.enable_it_) {
            HAL_NVIC_SetPriority(SPI2_IRQn, 1, 1);
            HAL_NVIC_EnableIRQ(SPI2_IRQn);
            (sSpiDevice + 1)->handle_->RxCpltCallback = __spi_rx_done_callback;
            (sSpiDevice + 1)->handle_->TxCpltCallback = __spi_tx_done_callback;
        }
        break;
    }
}

static void __msp_spi_deinit(SPI_HandleTypeDef* hspi) {
    switch ((uint32_t) (hspi->Instance)) {
    case SPI1_BASE:
        __HAL_RCC_SPI1_CLK_DISABLE();
        hal_gpio_deinit(SPI1_CS);
        hal_gpio_deinit(SPI1_SCK);
        hal_gpio_deinit(SPI1_MISO);
        hal_gpio_deinit(SPI1_MOSI);
        if (sSpiDevice->params_.enable_it_) {
            HAL_NVIC_DisableIRQ(SPI1_IRQn);
            sSpiDevice->handle_->RxCpltCallback = null;
            sSpiDevice->handle_->TxCpltCallback = null;
        }
        break;
    case SPI2_BASE:
        __HAL_RCC_SPI2_CLK_DISABLE();
        hal_gpio_deinit(SPI2_CS);
        hal_gpio_deinit(SPI2_SCK);
        hal_gpio_deinit(SPI2_MISO);
        hal_gpio_deinit(SPI2_MOSI);
        if ((sSpiDevice + 1)->params_.enable_it_) {
            HAL_NVIC_DisableIRQ(SPI2_IRQn);
            (sSpiDevice + 1)->handle_->RxCpltCallback = null;
            (sSpiDevice + 1)->handle_->TxCpltCallback = null;
        }
        break;
    }
}

bsp_handle_t bsp_spi_open(uint32_t which, bsp_spi_params_t* params) {
    if (!params) return null_handle;

    spi_device_t* handle = null;
    switch (which) {
    case BSP_SPI1:
    case BSP_SPI2:
        handle = sSpiDevice + which;
        handle->handle_ = sSpis + which;
        break;
    default: break;
    }

    if (!handle || handle->inited_) return null_handle;

    handle->params_.work_mode_ = params->work_mode_;
    handle->params_.enable_it_ = params->enable_it_;

    switch (params->work_mode_) {
    case SPI_WORK_MODE1:
        handle->handle_->Init.CLKPolarity = SPI_POLARITY_LOW;
        handle->handle_->Init.CLKPhase = SPI_PHASE_1EDGE;
        break;
    case SPI_WORK_MODE2:
        handle->handle_->Init.CLKPolarity = SPI_POLARITY_HIGH;
        handle->handle_->Init.CLKPhase = SPI_PHASE_1EDGE;
        break;
    case SPI_WORK_MODE3:
        handle->handle_->Init.CLKPolarity = SPI_POLARITY_LOW;
        handle->handle_->Init.CLKPhase = SPI_PHASE_2EDGE;
        break;
    case SPI_WORK_MODE4:
        handle->handle_->Init.CLKPolarity = SPI_POLARITY_HIGH;
        handle->handle_->Init.CLKPhase = SPI_PHASE_2EDGE;
        break;
    default:
        return null_handle;
    }
    HAL_StatusTypeDef ret = HAL_SPI_Init(handle->handle_);
    if (ret == HAL_OK) {
        handle->inited_ = true;
        ALOGD("init success.");
        return (bsp_handle_t) handle;
    }

    ALOGE("init failed.");
    return null_handle;
}

status_t bsp_spi_close(bsp_handle_t which) {
    if (!which) return EBADPARAM;
    spi_device_t* handle = (spi_device_t*) which;
    if (!handle->inited_) return EALREADY;

    HAL_StatusTypeDef ret = HAL_SPI_DeInit(handle->handle_);
    handle->inited_ = false;
    return (ret == HAL_OK) ? NO_ERROR : EHW;
}

status_t bsp_spi_send_with_recv(uint8_t* tx, uint8_t* rx, uint16_t size, bsp_handle_t handle) {
    if (!handle) return EBADPARAM;

    spi_device_t* spi_dev = (spi_device_t*) handle;

    spi_dev->cs_enable_(spi_dev->spi_index_, true);
    HAL_StatusTypeDef ret = HAL_SPI_TransmitReceive(spi_dev->handle_, tx, rx, size, 0x1FFF);
    spi_dev->cs_enable_(spi_dev->spi_index_, false);
    return (ret == HAL_OK) ? NO_ERROR : EHW;
}
