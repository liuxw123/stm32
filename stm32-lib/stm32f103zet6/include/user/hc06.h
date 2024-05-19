#ifndef HC06_H
#define HC06_H

#include "base/base.h"
#include "stm32f1xx_hal.h"

typedef enum {
    AT,
    AT_BAUD,
    AT_NAME,
    AT_PIN,
} at_cmds_t;

status_t init_hc06();
status_t exit_hc06();
status_t hc06_at();
status_t hc06_name(char* name);
status_t hc06_pin(char* pin);
status_t hc06_baud(uint32_t baud);

#endif // HC06_H