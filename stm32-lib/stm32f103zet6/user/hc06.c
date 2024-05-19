#include "user/hc06.h"
#include "bsp/bsp_uart.h"
#include "user/logcat.h"
#include "base/base.h"

#define LOG_TAG "hc_06"

#define HC06_COMMAND_END "\r\n"
#define HC06_COMMAND_AT "AT"
#define HC06_COMMAND_AT_BAUD "AT+BAUD"
// #define HC06_COMMAND_AT_BAUD_1200 "AT+BAUD1"
// #define HC06_COMMAND_AT_BAUD_2400 "AT+BAUD2"
// #define HC06_COMMAND_AT_BAUD_4800 "AT+BAUD3"
// #define HC06_COMMAND_AT_BAUD_9600 "AT+BAUD4"
// #define HC06_COMMAND_AT_BAUD_19200 "AT+BAUD5"
// #define HC06_COMMAND_AT_BAUD_38400 "AT+BAUD6"
// #define HC06_COMMAND_AT_BAUD_57600 "AT+BAUD7"
// #define HC06_COMMAND_AT_BAUD_115200 "AT+BAUD8"
// #define HC06_COMMAND_AT_BAUD_230400 "AT+BAUD9"
// #define HC06_COMMAND_AT_BAUD_460800 "AT+BAUDA"
// #define HC06_COMMAND_AT_BAUD_921600 "AT+BAUDB"
// #define HC06_COMMAND_AT_BAUD_1382400 "AT+BAUDC"
#define HC06_COMMAND_AT_NAME "AT+NAME"
#define HC06_COMMAND_AT_PIN "AT+PIN"

#define HC06_WORK_AT 0x1000
#define HC06_WORK_PAIR 0x2000

#define HC06_ASSERT_WORK__AT()                            \
    do {                                                  \
        if (sHc06WorkMode != HC06_WORK_AT) return ESTATE; \
    } while (0)

#define HC06_ASSERT_WORK__PAIR()                            \
    do {                                                    \
        if (sHc06WorkMode != HC06_WORK_PAIR) return ESTATE; \
    } while (0)

#define SEND_MAX_CHAR_SIZE 32
#define RECV_MAX_CHAR_SIZE 32

static bsp_handle_t sHandle;
static uint32_t sHc06WorkMode;
static uint8_t sRecv[RECV_MAX_CHAR_SIZE];
static uint8_t sSend[SEND_MAX_CHAR_SIZE];

static void __clear_recv_buffer() {
    memset(sRecv, 0, RECV_MAX_CHAR_SIZE);
}


static uint32_t __at_cmd_str(at_cmds_t cmd, char* param) {
    uint32_t len = 0;
    switch (cmd) {
    case AT: len += snprintf(sSend, SEND_MAX_CHAR_SIZE, "%s", HC06_COMMAND_AT); break;
    case AT_NAME: len += snprintf(sSend, SEND_MAX_CHAR_SIZE, "%s%s", HC06_COMMAND_AT_NAME, param); break;
    case AT_PIN: len += snprintf(sSend, SEND_MAX_CHAR_SIZE, "%s%s", HC06_COMMAND_AT_PIN, param); break;
    case AT_BAUD: len += snprintf(sSend, SEND_MAX_CHAR_SIZE, "%s%c", HC06_COMMAND_AT_BAUD, *param); break;
    default: return 0;
    }
    sSend[len] = '\0';
    ALOGD("len:%d, str: [%s]", len, sSend);

    return len;
}

static status_t __hc06_print_cmd_result() {
    if (sRecv[0] == 'O' && sRecv[1] == 'K') {
        ALOGD("[%s] command test success.", sSend);
        return NO_ERROR;
    } else {
        ALOGE("[%s] command test failed.", sSend);
        return EHW;
    }
}

static status_t __hc06_send_command(at_cmds_t cmd, char* param) {
    __clear_recv_buffer();
    __at_cmd_str(cmd, param);
    bsp_uart_send_with_recv(sSend, sRecv, sHandle);

    ALOGD("Send: [%s], Return: [%s]", sSend, sRecv);
    return __hc06_print_cmd_result();
}

static status_t __hc06_open_uart(bsp_uart_params_t* param) {
    bsp_handle_t handle = bsp_uart_open(BSP_UART2, param);
    if (handle) sHandle = handle;

    return (handle) ? NO_ERROR : EUNKNOW;
}

static status_t __hc06_reopen_uart(uint32_t baud) {
    bsp_uart_params_t param = {
        .enable_it_ = true,
        .baud_rate_ = baud,
        .work_mode_ = UART_MODE_TX_RX,
    };

    status_t ret = exit_hc06();
    if (ret) ALOGE("close hc06 failed. ignore it");
    ret = __hc06_open_uart(&param);
    if (ret) {
        ALOGE("open hc06 failed");
    }
    return ret;
}

status_t hc06_at() {
    __clear_recv_buffer();
    __at_cmd_str(AT, null);
    bsp_uart_send_with_recv(sSend, sRecv, sHandle);

    ALOGD("Send: [%s], Return: [%s]", sSend, sRecv);

    if (sRecv[0] == 'O' && sRecv[1] == 'K') {
        ALOGD("[%s] command test success.", sSend);
        sHc06WorkMode = HC06_WORK_AT;
        return NO_ERROR;
    } else if (sRecv[0] == 'A' && sRecv[1] == 'C' && sRecv[2] == 'K') {
        ALOGD("[%s] command test success.", sSend);
        sHc06WorkMode = HC06_WORK_PAIR;
        return NO_ERROR;
    } else {
        sHc06WorkMode = 0;
        ALOGE("[%s] command test failed.", sSend);
        return EHW;
    }
}

status_t hc06_name(char* name) {
    HC06_ASSERT_WORK__AT();
    if (!name) return EBADPARAM;
    return __hc06_send_command(AT_NAME, name);
}

status_t hc06_pin(char* pin) {
    HC06_ASSERT_WORK__AT();
    if (!pin) return EBADPARAM;
    return __hc06_send_command(AT_PIN, pin);
}

status_t hc06_baud(uint32_t baud) {
    HC06_ASSERT_WORK__AT();
    uint32_t bauds[] = {1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600, 1382400};
    uint32_t index = 0;

    ALOGD("size: %d", ARRAY_SIZE(bauds));
    for (index = 0; index < ARRAY_SIZE(bauds); index++) {
        if (bauds[index] == baud) break;
    }

    if (index >= ARRAY_SIZE(bauds)) return EBADPARAM;

    char code = (index <= 8) ? '1' + index : 'A' + (index - 9);
    status_t ret = __hc06_send_command(AT_BAUD, &code);

    ALOGD("please reset master device to reopen UART device for baud rate changed, new baudrate: %d", bauds[index]);

#if 0
    HAL_Delay(2000);

    ret = __hc06_reopen_uart(bauds[index]);

    if (ret) {
        ALOGE("reset baud to %d failed.", bauds[index]);
        return ret;
    }

    return hc06_at();
#else
    return ret;
#endif
}

status_t init_hc06() {
    bsp_uart_params_t param = {
        .enable_it_ = true,
        .baud_rate_ = 9600,
        .work_mode_ = UART_MODE_TX_RX,
    };

    status_t ret = __hc06_open_uart(&param);

    if (ret) {
        ALOGE("init_hc06 failed.");
        return EHW;
    }

    ret = hc06_at();
    if (ret) return ret;
    if (sHc06WorkMode == HC06_WORK_AT) {
        ret = hc06_name("hc-06");
        if (ret) return ret;
        ret = hc06_baud(9600);
        if (ret) return ret;
        ret = hc06_pin("1234");
        if (ret) return ret;
    }

    ALOGD("[SUCCESS] init_hc06 success.");
    return NO_ERROR;
}

status_t exit_hc06() {
    status_t ret = bsp_uart_close(sHandle);
    sHandle = null_handle;
    return ret;
}