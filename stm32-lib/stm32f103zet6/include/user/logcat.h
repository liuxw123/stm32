#ifndef LOGCAT_H
#define LOGCAT_H

#include "base/base.h"
#include "stm32f1xx_hal.h"


#define __LOG__(level, fmt, args...)                                                                                    \
    do {                                                                                                                \
        char temp_str[255];                                                                                             \
        uint32_t temp_ticks = HAL_GetTick();                                                                            \
        uint32_t temp_ms = temp_ticks % 1000;                                                                           \
        temp_ticks /= 1000;                                                                                             \
        uint32_t temp_sec = temp_ticks % 60;                                                                            \
        temp_ticks /= 60;                                                                                               \
        uint32_t temp_min = temp_ticks % 60;                                                                            \
        uint32_t temp_hour = temp_ticks / 60;                                                                           \
        uint32_t temp_len = 0;                                                                                          \
        temp_len += snprintf(temp_str, 255, "%02d:%02d:%02d.%03d ", temp_hour, temp_min, temp_sec, temp_ms);            \
        temp_len += snprintf(temp_str + temp_len, 255 - temp_len, "%s %c %s %4d ", LOG_TAG, level, __func__, __LINE__); \
        if (level == 'E') temp_len += snprintf(temp_str + temp_len, 255 - temp_len, "[**ERROR**] ");                    \
        temp_len += snprintf(temp_str + temp_len, 255 - temp_len, fmt, ##args);                                         \
        if (temp_len < 254) {                                                                                           \
            temp_str[temp_len] = '\n';                                                                                  \
            temp_len++;                                                                                                 \
        }                                                                                                               \
        temp_str[temp_len] = '\0';                                                                                      \
        send_string(temp_str);                                                                                          \
    } while (0)
#define ALOGV(fmt, args...) __LOG__('V', fmt, ##args)
#define ALOGD(fmt, args...) __LOG__('D', fmt, ##args)
#define ALOGI(fmt, args...) __LOG__('I', fmt, ##args)
#define ALOGW(fmt, args...) __LOG__('W', fmt, ##args)
#define ALOGE(fmt, args...) __LOG__('E', fmt, ##args)

status_t init_logcat();
status_t exit_logcat();
void send_string(char* str);

#endif