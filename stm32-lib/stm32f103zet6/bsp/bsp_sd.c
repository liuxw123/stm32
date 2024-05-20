#include "bsp/bsp_sd.h"
#include "user/logcat.h"

#define LOG_TAG "bsp_sd"

#define __DMAx_TxRx_CLK_ENABLE __HAL_RCC_DMA2_CLK_ENABLE
#define SD_DMAx_Tx_INSTANCE DMA2_Channel4
#define SD_DMAx_Rx_INSTANCE DMA2_Channel4
#define SD_DMAx_Tx_IRQn DMA2_Channel4_5_IRQn
#define SD_DMAx_Rx_IRQn DMA2_Channel4_5_IRQn

#define SD_TRANSFER_OK ((uint8_t) 0x00)
#define SD_TRANSFER_BUSY ((uint8_t) 0x01)

static void __msp_sd_init(SD_HandleTypeDef* hsd);
static void __msp_sd_deinit(SD_HandleTypeDef* hsd);
void __sd_rx_done(SD_HandleTypeDef* hsd);
void __sd_tx_done(SD_HandleTypeDef* hsd);

uint8_t SD_DMAConfigRx(SD_HandleTypeDef* hsd);
uint8_t SD_DMAConfigTx(SD_HandleTypeDef* hsd);

static SD_HandleTypeDef uSdHandle = {
    .Instance = SDIO,
    .Init = {
        .ClockEdge = SDIO_CLOCK_EDGE_RISING,
        .ClockBypass = SDIO_CLOCK_BYPASS_DISABLE,
        .ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE,
        .BusWide = SDIO_BUS_WIDE_1B,
        .HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE,
        .ClockDiv = SDIO_TRANSFER_CLK_DIV,
    },
    .MspInitCallback = __msp_sd_init,
    .MspDeInitCallback = __msp_sd_deinit,
    .TxCpltCallback = __sd_tx_done,
    .RxCpltCallback = __sd_rx_done,
};


static volatile bool sDmaTxDone = true;
static volatile bool sDmaRxDone = true;
static bool sInited = false;


#if 0
static void __print_info(uint8_t ret) {
    if (ret != MSD_OK) {
        LCD_DEBUG(0, 20, "sd init failed");
    } else {
        LCD_DEBUG(0, 20, "sd init success");
    }

    if (uSdHandle.SdCard.CardType == CARD_SDHC_SDXC) {
        LCD_DEBUG(0, 40, "Type: MassStorage");
    } else if (uSdHandle.SdCard.CardType == CARD_SDSC) {
        LCD_DEBUG(0, 40, "Type: StandardStorage");
    } else {
        LCD_DEBUG(0, 40, "Type: Invalid");
    }

    if (uSdHandle.SdCard.CardVersion == CARD_V1_X) {
        LCD_DEBUG(0, 60, "card Version: CardV1");
    } else if (uSdHandle.SdCard.CardVersion == CARD_V2_X) {
        LCD_DEBUG(0, 60, "Version: CardV2");
    } else {
        LCD_DEBUG(0, 60, "Version: Invalid");
    }

    double v = 1.0 * uSdHandle.SdCard.LogBlockNbr * uSdHandle.SdCard.LogBlockSize / 1024 / 1024;
    LCD_DEBUG(0, 80, "Volume: %d M", (uint32_t) v);
    LCD_DEBUG(0, 100, "BlockSize: %d", uSdHandle.SdCard.LogBlockSize);
    LCD_DEBUG(0, 120, "BlockNbr: %d", uSdHandle.SdCard.LogBlockNbr);
    LCD_DEBUG(0, 140, "Class: 0x%08X", uSdHandle.SdCard.Class);
    LCD_DEBUG(0, 160, "State: %d", HAL_SD_GetCardState(&uSdHandle));
    LCD_DEBUG(0, 180, "RelCardAdd: 0x%08X", uSdHandle.SdCard.RelCardAdd);
}
#endif

uint8_t BSP_SD_Init(void) {
    uint8_t state = MSD_OK;

    if (sInited) return state;

    if (HAL_SD_Init(&uSdHandle) != HAL_OK) {
        state = MSD_ERROR;
    }

    if (state == MSD_OK) {
        if (HAL_SD_ConfigWideBusOperation(&uSdHandle, SDIO_BUS_WIDE_4B) != HAL_OK) {
            state = MSD_ERROR;
        } else {
            state = MSD_OK;
        }
    }

    sInited = (state == NO_ERROR);
    return state;
}

uint8_t BSP_SD_ReadBlocks(uint32_t* pData, uint32_t ReadAddr, uint32_t NumOfBlocks) {
    if (HAL_SD_ReadBlocks(&uSdHandle, (uint8_t*) pData, ReadAddr, NumOfBlocks, 0xFFFF) != HAL_OK) {
        return MSD_ERROR;
    } else {
        return MSD_OK;
    }
}

uint8_t BSP_SD_WriteBlocks(uint32_t* pData, uint32_t WriteAddr, uint32_t NumOfBlocks) {
    if (HAL_SD_WriteBlocks(&uSdHandle, (uint8_t*) pData, WriteAddr, NumOfBlocks, 0xFFFF) != HAL_OK) {
        return MSD_ERROR;
    } else {
        return MSD_OK;
    }
}

uint8_t BSP_SD_ReadBlocks_DMA(uint32_t* pData, uint32_t ReadAddr, uint32_t NumOfBlocks) {
    uint8_t state = MSD_OK;

    while (BSP_SD_GetCardState() == SD_TRANSFER_BUSY);

    uSdHandle.hdmatx = NULL;
    state = ((SD_DMAConfigRx(&uSdHandle) == MSD_OK) ? MSD_OK : MSD_ERROR);
    if (state == MSD_OK) {
        state = ((HAL_SD_ReadBlocks_DMA(&uSdHandle, (uint8_t*) pData, ReadAddr, NumOfBlocks) == HAL_OK) ? MSD_OK : MSD_ERROR);
        if (state == MSD_OK) {
            sDmaRxDone = false;
            while (!sDmaRxDone);
        }
    }

    return state;
}

uint8_t BSP_SD_WriteBlocks_DMA(uint32_t* pData, uint32_t WriteAddr, uint32_t NumOfBlocks) {
    uint8_t state = MSD_OK;
    while (BSP_SD_GetCardState() == SD_TRANSFER_BUSY);
    uSdHandle.hdmarx = NULL;

    state = ((SD_DMAConfigTx(&uSdHandle) == MSD_OK) ? MSD_OK : MSD_ERROR);
    if (state == MSD_OK) {
        state = ((HAL_SD_WriteBlocks_DMA(&uSdHandle, (uint8_t*) pData, WriteAddr, NumOfBlocks) == HAL_OK) ? MSD_OK : MSD_ERROR);
        if (state == MSD_OK) {
            sDmaTxDone = false;
            while (!sDmaTxDone);
        }
    }
    return state;
}

uint8_t BSP_SD_Erase(uint32_t StartAddr, uint32_t EndAddr) {
    if (HAL_SD_Erase(&uSdHandle, StartAddr, EndAddr) != HAL_OK) {
        return MSD_ERROR;
    } else {
        return MSD_OK;
    }
}

uint8_t BSP_SD_GetCardState(void) {
    return ((HAL_SD_GetCardState(&uSdHandle) == HAL_SD_CARD_TRANSFER) ? SD_TRANSFER_OK : SD_TRANSFER_BUSY);
}


void BSP_SD_GetCardInfo(HAL_SD_CardInfoTypeDef* CardInfo) {
    HAL_SD_GetCardInfo(&uSdHandle, CardInfo);
}

static void __msp_sd_init(SD_HandleTypeDef* hsd) {
    __HAL_RCC_SDIO_CLK_ENABLE();
    __DMAx_TxRx_CLK_ENABLE();

    hal_gpio_init(__GPIO_PORT__(GPIO_BANK_C, GPIO_PIN8 | GPIO_PIN9 | GPIO_PIN10 | GPIO_PIN11 | GPIO_PIN12), GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH);
    hal_gpio_init(__GPIO_PORT__(GPIO_BANK_D, GPIO_PIN2), GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH);

    HAL_NVIC_SetPriority(SDIO_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(SDIO_IRQn);
}

static void __msp_sd_deinit(SD_HandleTypeDef* hsd) {
}

uint8_t SD_DMAConfigRx(SD_HandleTypeDef* hsd) {
    static DMA_HandleTypeDef hdma_rx;
    HAL_StatusTypeDef status = HAL_ERROR;

    if (hsd->hdmarx == NULL) {
        hdma_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
        hdma_rx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_rx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
        hdma_rx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
        hdma_rx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
        hdma_rx.Init.Mode = DMA_NORMAL;
        hdma_rx.Instance = SD_DMAx_Rx_INSTANCE;

        __HAL_LINKDMA(hsd, hdmarx, hdma_rx);
        HAL_DMA_Abort(&hdma_rx);
        HAL_DMA_DeInit(&hdma_rx);
        status = HAL_DMA_Init(&hdma_rx);

        HAL_NVIC_SetPriority(SD_DMAx_Rx_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(SD_DMAx_Rx_IRQn);
    } else {
        status = HAL_OK;
    }

    return (status != HAL_OK ? MSD_ERROR : MSD_OK);
}

uint8_t SD_DMAConfigTx(SD_HandleTypeDef* hsd) {
    static DMA_HandleTypeDef hdma_tx;
    HAL_StatusTypeDef status;

    if (hsd->hdmatx == NULL) {
        hdma_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
        hdma_tx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_tx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
        hdma_tx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
        hdma_tx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
        hdma_tx.Init.Mode = DMA_NORMAL;
        hdma_tx.Instance = SD_DMAx_Tx_INSTANCE;

        __HAL_LINKDMA(hsd, hdmatx, hdma_tx);
        HAL_DMA_Abort(&hdma_tx);
        HAL_DMA_DeInit(&hdma_tx);

        status = HAL_DMA_Init(&hdma_tx);

        HAL_NVIC_SetPriority(SD_DMAx_Tx_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(SD_DMAx_Tx_IRQn);
    } else {
        status = HAL_OK;
    }

    return (status != HAL_OK ? MSD_ERROR : MSD_OK);
}


void SDIO_IRQHandler(void) {
    HAL_SD_IRQHandler(&uSdHandle);
}

void DMA2_Channel4_5_IRQHandler(void) {
    if (uSdHandle.hdmarx) {
        HAL_DMA_IRQHandler(uSdHandle.hdmarx);
        sDmaRxDone = true;
    } else if (uSdHandle.hdmatx) {
        HAL_DMA_IRQHandler(uSdHandle.hdmatx);
        sDmaTxDone = true;
    }
}

void __sd_tx_done(SD_HandleTypeDef* hsd) {
    sDmaTxDone = 1;
    ALOGD("write done.");
}

void __sd_rx_done(SD_HandleTypeDef* hsd) {
    sDmaRxDone = 1;
    ALOGD("read done.");
}
