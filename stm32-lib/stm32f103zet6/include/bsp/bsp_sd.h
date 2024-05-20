#ifndef BSP_SD_H
#define BSP_SD_H

#include "stm32f1xx_hal.h"

#include "base/base.h"
#include "base/stm32f103zet6.h"


#define MSD_OK ((uint8_t) 0x00)
#define MSD_ERROR ((uint8_t) 0x01)


#define SD_DATATIMEOUT 100000000U

#define SD_PRESENT ((uint8_t) 0x01)
#define SD_NOT_PRESENT ((uint8_t) 0x00)

uint8_t BSP_SD_Init(void);
uint8_t BSP_SD_ReadBlocks(uint32_t* pData, uint32_t ReadAddr, uint32_t NumOfBlocks);
uint8_t BSP_SD_WriteBlocks(uint32_t* pData, uint32_t WriteAddr, uint32_t NumOfBlocks);
uint8_t BSP_SD_ReadBlocks_DMA(uint32_t* pData, uint32_t ReadAddr, uint32_t NumOfBlocks);
uint8_t BSP_SD_WriteBlocks_DMA(uint32_t* pData, uint32_t WriteAddr, uint32_t NumOfBlocks);
uint8_t BSP_SD_Erase(uint32_t StartAddr, uint32_t EndAddr);
uint8_t BSP_SD_GetCardState(void);
void BSP_SD_GetCardInfo(HAL_SD_CardInfoTypeDef* CardInfo);

#endif // BSP_SD_H
