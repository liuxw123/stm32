#include "user/w25q128.h"
#include "user/logcat.h"
#include "bsp/bsp_spi.h"

#define LOG_TAG "w25q128"
#define DUMMY_BYTE 0xFF
static bsp_handle_t sHandle = 0;

static status_t __get_w25q128_info() {
    uint8_t tx[4] = {0x9F, DUMMY_BYTE, DUMMY_BYTE, DUMMY_BYTE};
    uint8_t rx[4];

    status_t ret = bsp_spi_send_with_recv(tx, rx, 4, sHandle);
    if (rx[1] == 0xEF && rx[2] == 0x40 && rx[3] == 0x18) {
        ALOGD("detected w25q128fv flash");
        return NO_ERROR;
    }

    ALOGE("unknown flash device.%02X%02X%02X", rx[1], rx[2], rx[3]);
    return EUNKNOW;
}

status_t init_w25q128() {
    bsp_spi_params_t param = {
        .enable_it_ = false,
        .work_mode_ = SPI_WORK_MODE1,
    };
    bsp_handle_t handle = bsp_spi_open(BSP_SPI2, &param);
    if (handle != null_handle)
        sHandle = handle;
    else
        return ESTATE;

    return __get_w25q128_info();
}

status_t exit_w25q128() {
    if (sHandle)
        return bsp_spi_close(sHandle);
    return NO_ERROR;
}