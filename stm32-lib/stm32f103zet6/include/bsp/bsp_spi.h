#ifndef BSP_SPI_H
#define BSP_SPI_H

#include "stm32f1xx_hal.h"

#include "base/base.h"
#include "base/stm32f103zet6.h"

#define BSP_SPI1 0x0
#define BSP_SPI2 0x1
#define BSP_SPI_MAX (BSP_SPI2 + 1)

typedef enum {
    SPI_WORK_MODE1,
    SPI_WORK_MODE2,
    SPI_WORK_MODE3,
    SPI_WORK_MODE4,
} spi_work_t;

typedef struct {
    spi_work_t work_mode_;
    bool enable_it_;
} bsp_spi_params_t;

bsp_handle_t bsp_spi_open(uint32_t which, bsp_spi_params_t* params);
status_t bsp_spi_close(bsp_handle_t which);
status_t bsp_spi_send_with_recv(uint8_t* tx, uint8_t* rx, uint16_t size, bsp_handle_t handle);

#endif // BSP_SPI_H