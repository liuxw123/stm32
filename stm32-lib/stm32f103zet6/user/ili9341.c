#include "user/logcat.h"
#include "fonts/fonts.h"
#include "user/ffile.h"
#include "user/ili9341.h"
#include "bsp/bsp_fsmc.h"

#define LOG_TAG "ili9341"

#define LCD_ILI9341_CMD(cmd) FSMC_CMD(cmd)
#define LCD_ILI9341_Parameter(val) FSMC_Parameter(val)
#define LCD_BL_CTRL __GPIO_PORT__(GPIO_BANK_D, GPIO_PIN13)
#define LCD_RESET __GPIO_PORT__(GPIO_BANK_D, GPIO_PIN11)
#define LCD_BLACK_LED_ON GPIO_PIN_RESET
#define LCD_BLACK_LED_OFF GPIO_PIN_SET

#define LCD_WIDTH 240
#define LCD_HEIGHT 320

typedef struct {
    lcd_scan_direction_t scan_dir_;
    uint32_t width_pixel_;
    uint32_t height_pixel_;
    lcd_font_t font_size_;
    sFONT font_;
    bsp_handle_t font_handle_;
    uint16_t foreground_color_;
    uint16_t background_color_;
} lcd_info_t;

static lcd_info_t sLcdInfo = {
    .scan_dir_ = RIGHT_LEFT_DOWN_UP,
    .width_pixel_ = LCD_WIDTH,
    .height_pixel_ = LCD_HEIGHT,
    .font_size_ = FONT_MAX,
    .font_handle_ = null_handle,
    .foreground_color_ = WHITE,
    .background_color_ = BLACK,
};

static uint8_t DELAY = 0, CMD = 1, EXIT = 2;
static uint16_t LOOP = 0xFFF0;

static uint8_t* __get_font_path(lcd_font_t font) {
    switch (font) {
    case FONT_5_8: return "/sys/fonts/font8.bin";
    case FONT_7_12: return "/sys/fonts/font12.bin";
    case FONT_11_16: return "/sys/fonts/font16.bin";
    case FONT_14_20: return "/sys/fonts/font20.bin";
    case FONT_17_24: return "/sys/fonts/font24.bin";
    default: return null;
    }
}

static uint32_t __get_font_bytes(lcd_font_t font) {
    switch (font) {
    case FONT_5_8: return 8;
    case FONT_7_12: return 12;
    case FONT_11_16: return 32;
    case FONT_14_20: return 40;
    case FONT_17_24: return 72;
    default: return 0;
    }
}

static status_t __lcd_gpio_config() {
    status_t ret = NO_ERROR;
    ret = hal_gpio_init(LCD_BL_CTRL, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH);
    if (ret) return ret;
    ret = hal_gpio_init(LCD_RESET, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH);
    return ret;
}

static void __lcd_reset() {
    HAL_GPIO_WritePin((GPIO_TypeDef*) hal_gpio_bank(LCD_RESET), hal_gpio_pin(LCD_RESET), GPIO_PIN_RESET);
    HAL_Delay(30);
    HAL_GPIO_WritePin((GPIO_TypeDef*) hal_gpio_bank(LCD_RESET), hal_gpio_pin(LCD_RESET), GPIO_PIN_SET);
    HAL_Delay(30);
}

void lcd_black_led(bool on) {
    HAL_GPIO_WritePin(
        (GPIO_TypeDef*) hal_gpio_bank(LCD_BL_CTRL), hal_gpio_pin(LCD_BL_CTRL),
        on ? LCD_BLACK_LED_ON : LCD_BLACK_LED_OFF);
}

static void __lcd_write_memory(uint16_t* datas, uint32_t arr_size) {
    uint32_t index = 0;
    LCD_ILI9341_CMD(0x2C);
    uint16_t cmd = datas[0];
    if (cmd == LOOP) {
        uint16_t msb = datas[1];
        uint16_t lsb = datas[2];
        uint16_t data = datas[3];
        uint32_t count = (((uint32_t) msb) << 16) | lsb;
        while (count--) LCD_ILI9341_Parameter(data);
    } else {
        while (arr_size--) LCD_ILI9341_Parameter(datas[index++]);
    }
}

static void __lcd_write_cmds(uint8_t* datas, uint32_t arr_size) {
    uint32_t index = 0;
    while (index < arr_size) {
        uint8_t cmd = datas[index];
        if (cmd == DELAY) {
            index++;
            HAL_Delay(datas[index]);
            index++;
        } else if (cmd == CMD) {
            index++;
            LCD_ILI9341_CMD(datas[index]);
            index++;
            uint32_t count = datas[index];
            index++;
            while (count--) LCD_ILI9341_Parameter(datas[index++]);
        } else if (cmd == EXIT) {
            index++;
            break;
        }
    }

    if (arr_size != index) ALOGE("index: %d, size: %d", index, arr_size);
}

static void __lcd_reg_config(void) {
    // clang-format off
    uint8_t init_reg_arr[] = {
        DELAY, 4, /* 0: delay, 1: cmd, 2: exit*/
        CMD, 0xCF, 3, 0x00, 0x81, 0x30,
        DELAY, 4,
        CMD, 0xED, 4, 0x64, 0x03, 0x12, 0x81,
        DELAY, 4,
        CMD, 0xE8, 3, 0x85, 0x10, 0x78,
        DELAY, 4,
        CMD, 0xCB, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,
        DELAY, 4,
        CMD, 0xF7, 1, 0x20,
        DELAY, 4,
        CMD, 0xEA, 2, 0x00, 0x00,
        DELAY, 4,
        CMD, 0xB1, 2, 0x00, 0x1B,
        DELAY, 4,
        CMD, 0xB6, 2, 0x0A, 0xA2,
        DELAY, 4,
        CMD, 0xC0, 1, 0x35,
        DELAY, 4,
        CMD, 0xC1, 1, 0x11,
        CMD, 0xC5, 2, 0x45, 0x45,
        CMD, 0xC7, 1, 0xA2, 
        CMD, 0xF2, 1, 0x00,
        CMD, 0x26, 1, 0x01,
        DELAY, 4,
        CMD, 0xE0, 15, 0x0F, 0x26, 0x24, 0x0B, 0x0E, 0x09, 0x54, 0xA8, 0x46, 0x0C, 0x17, 0x09, 0x0F, 0x07, 0x00,
        CMD, 0xE1, 15, 0x00, 0x19, 0x1B, 0x04, 0x10, 0x07, 0x2A, 0x47, 0x39, 0x03, 0x06, 0x06, 0x30, 0x38, 0x0F,
        DELAY, 4,
        CMD, 0x36, 1, 0xC8,
        DELAY, 4,
        CMD, 0x2A, 4, 0x00, 0x00, (uint8_t) (((LCD_WIDTH - 1) & 0xFF00) >> 8), (uint8_t) ((LCD_WIDTH - 1) & 0x00FF),
        DELAY, 4,
        CMD, 0x2B, 4, 0x00, 0x00, (uint8_t) (((LCD_HEIGHT - 1) & 0xFF00) >> 8), (uint8_t) ((LCD_HEIGHT - 1) & 0x00FF),
        DELAY, 4,
        CMD, 0x3A, 1, 0x55,
        CMD, 0x11, 0,
        DELAY, 10,
        CMD, 0x29, 0,
        EXIT,
    };
    // clang-format on

    __lcd_write_cmds(init_reg_arr, ARRAY_SIZE(init_reg_arr));
}

void clear_lcd_screen(uint16_t color) {
    uint32_t start = HAL_GetTick();
    // clang-format off
    uint8_t cmds[] = {
        CMD, 0x2A, 4, 0x00, 0x00, (uint8_t) (((sLcdInfo.width_pixel_ - 1) & 0xFF00) >> 8), (uint8_t) ((sLcdInfo.width_pixel_ - 1) & 0x00FF),
        CMD, 0x2B, 4, 0x00, 0x00, (uint8_t) (((sLcdInfo.height_pixel_ - 1) & 0xFF00) >> 8), (uint8_t) ((sLcdInfo.height_pixel_ - 1) & 0x00FF),
    };
    // clang-format on
    __lcd_write_cmds(cmds, ARRAY_SIZE(cmds));

    uint16_t datas[] = {LOOP, (LCD_WIDTH * LCD_HEIGHT) >> 16, (LCD_WIDTH * LCD_HEIGHT) & 0xFFFF, color};
    __lcd_write_memory(datas, ARRAY_SIZE(datas));
    uint32_t end = HAL_GetTick();
    ALOGD("start: %d, end: %d", start, end);
}

status_t set_lcd_font_size(lcd_font_t font_size) {
    if (font_size == sLcdInfo.font_size_) return NO_ERROR;

    if (sLcdInfo.font_handle_) {
        ffclose(sLcdInfo.font_handle_);
        sLcdInfo.font_handle_ = null_handle;
    }

    char* font_path = __get_font_path(font_size);
    if (!font_path) return EBADPARAM;

    status_t ret = NO_ERROR;
    uint32_t header_size = sizeof(sLcdInfo.font_) - sizeof(sLcdInfo.font_.table);
    bsp_handle_t file_handle = ffopen(font_path, FA_READ);
    if (!file_handle) return EUNKNOW;
    ret = ffread((uint8_t*) (&(sLcdInfo.font_)), header_size, file_handle);
    if (ret) {
        ALOGE("failed to get font info.");
        sLcdInfo.font_handle_ = null_handle;
        sLcdInfo.font_size_ = FONT_MAX;
    } else {
        ALOGI("font: %d*%d, total size: %d", sLcdInfo.font_.Width, sLcdInfo.font_.Height, sLcdInfo.font_.size);
        sLcdInfo.font_handle_ = file_handle;
        sLcdInfo.font_size_ = font_size;
    }
    return ret;
}

static uint8_t* __get_char_buffer(char ch) {
    if (!sLcdInfo.font_handle_) {
        ALOGE("font file not opened.");
        return null;
    }
    if (ch < ' ') return EBADPARAM;
    uint32_t data_size = __get_font_bytes(sLcdInfo.font_size_);
    if (data_size == 0) return null;

    uint8_t* data_buffer = (uint8_t*) calloc(data_size, 1);
    if (!data_buffer) return null;

    uint32_t header_size = sizeof(sLcdInfo.font_) - sizeof(sLcdInfo.font_.table);
    uint32_t offset = header_size + data_size * (ch - ' ');
    status_t ret = ffseek(offset, sLcdInfo.font_handle_);
    if (ret) return null;

    ret = ffread(data_buffer, data_size, sLcdInfo.font_handle_);
    if (ret) return null;

    return data_buffer;
}

status_t init_ili9341() {
    status_t ret = NO_ERROR;
    ret = __lcd_gpio_config();
    if (ret) {
        ALOGE("__lcd_gpio_config failed.");
        return ret;
    }

    ret = init_fsmc();
    if (ret) {
        ALOGE("init_fsmc failed.");
        return ret;
    }
    __lcd_reset();
    lcd_black_led(true);

    __lcd_reg_config();
    set_lcd_screen_direction(LANDSSCAPE);
    clear_lcd_screen(sLcdInfo.background_color_);

    ret = set_lcd_font_size(FONT_17_24);
    if (ret) {
        ALOGE("set_lcd_font_size failed.");
        return ret;
    }

    lcd_rectangle_t rect = {
        .cord_ = {
            .x_ = (sLcdInfo.width_pixel_ - sLcdInfo.font_.Width * 12) / 2,
            .y_ = (sLcdInfo.height_pixel_ - sLcdInfo.font_.Height) / 2,
        },
        .w_ = 20,
        .h_ = 40,
    };

    lcd_show_string(rect.cord_, "Hello World!");

    set_lcd_color(MAGENTA, BLACK);
    lcd_set_pixel_color(rect.cord_);
    uint16_t pixel_color = lcd_get_pixel_color(rect.cord_);
    ALOGD("Got: 0x%04X, Expected: 0x%04X", pixel_color, MAGENTA);

    return NO_ERROR;
}

void __open_lcd_window(lcd_window_t win) {
    uint32_t x_start = win.cord_.x_;
    uint32_t x_end = win.cord_.x_ + win.w_ - 1;
    uint32_t y_start = win.cord_.y_;
    uint32_t y_end = win.cord_.y_ + win.h_ - 1;

    // clang-format off
    uint8_t cmds[] = {
        CMD, 0x2A, 4, (uint8_t) ((x_start & 0xFF00) >> 8), (uint8_t) (x_start & 0x00FF), (uint8_t) ((x_end & 0xFF00) >> 8), (uint8_t) (x_end & 0x00FF),
        CMD, 0x2B, 4, (uint8_t) ((y_start & 0xFF00) >> 8), (uint8_t) (y_start & 0x00FF), (uint8_t) ((y_end & 0xFF00) >> 8), (uint8_t) (y_end & 0x00FF),
    };
    // clang-format on
    __lcd_write_cmds(cmds, ARRAY_SIZE(cmds));
}

static void __draw_rectangle(lcd_rectangle_t rect, uint16_t color) {
    __open_lcd_window(rect);
    uint16_t datas[] = {LOOP, (rect.w_ * rect.h_) >> 16, (rect.w_ * rect.h_) & 0xFFFF, color};
    __lcd_write_memory(datas, ARRAY_SIZE(datas));
}

void set_lcd_screen_direction(lcd_scan_direction_t dir) {
    uint8_t data = (dir << 5) | 0x08;
    // clang-format off
    uint8_t cmds[] = {CMD, 0x36, 1, data,};
    // clang-format on
    __lcd_write_cmds(cmds, ARRAY_SIZE(cmds));

    sLcdInfo.scan_dir_ = dir;
    lcd_window_t win = {
        .cord_ = {
            .x_ = 0,
            .y_ = 0,
        },
    };
    if ((dir == LEFT_RIGHT_DOWN_UP) || (dir == LEFT_RIGHT_UP_DOWN) || (dir == RIGHT_LEFT_DOWN_UP)
        || (dir == RIGHT_LEFT_UP_DOWN)) {
        sLcdInfo.width_pixel_ = LCD_WIDTH;
        sLcdInfo.height_pixel_ = LCD_HEIGHT;
    } else {
        sLcdInfo.width_pixel_ = LCD_HEIGHT;
        sLcdInfo.height_pixel_ = LCD_WIDTH;
    }

    win.w_ = sLcdInfo.width_pixel_;
    win.h_ = sLcdInfo.height_pixel_;

    __open_lcd_window(win);
}

static void __lcd_write_byte(uint8_t data, uint32_t w, uint16_t fore_color, uint16_t back_color, bool msb) {
    if (!msb) data = 0;

    uint32_t index = 0;
    while (w) {
        if (data & (0x01 << (7 - index)))
            LCD_ILI9341_Parameter(fore_color);
        else
            LCD_ILI9341_Parameter(back_color);
        w--;
        index++;
    }
}

static void __lcd_show_char(lcd_pixel_point_t cord, char ch) {
    if (ch < ' ') return;

    uint8_t* data = __get_char_buffer(ch);
    if (!data) {
        ALOGE("__get_char_buffer failed.");
        return;
    }

    uint32_t w = sLcdInfo.font_.Width, h = sLcdInfo.font_.Height;

    lcd_window_t win = {
        .cord_ = {
            .x_ = cord.x_,
            .y_ = cord.y_,
        },
        .w_ = w,
        .h_ = h,
    };
    uint8_t* temp = data;
    __open_lcd_window(win);
    LCD_ILI9341_CMD(0x2C);
    for (uint32_t i = 0; i < h; i++) {
        uint32_t temp_w = w;
        while (temp_w > 8) {
            __lcd_write_byte(*temp++, 8, sLcdInfo.foreground_color_, sLcdInfo.background_color_, true);
            temp_w -= 8;
        }

        __lcd_write_byte(*temp++, temp_w, sLcdInfo.foreground_color_, sLcdInfo.background_color_, true);
    }

    free(data);
}

void lcd_show_string(lcd_pixel_point_t cord, char* str) {
    if (!str) return;

    lcd_pixel_point_t p = {
        .x_ = cord.x_,
        .y_ = cord.y_,
    };

    while (*str) {
        __lcd_show_char(p, *str);
        str++;
        p.x_ += sLcdInfo.font_.Width;
        if (p.x_ + sLcdInfo.font_.Width > sLcdInfo.width_pixel_) {
            p.x_ = 0;
            p.y_ += sLcdInfo.font_.Height;

            if (p.y_ + sLcdInfo.font_.Height > sLcdInfo.height_pixel_) p.y_ = 0;
        }
    }
}

void set_lcd_color(uint16_t fore_color, uint16_t back_color) {
    sLcdInfo.background_color_ = back_color;
    sLcdInfo.foreground_color_ = fore_color;
}

void lcd_set_pixel_color(lcd_pixel_point_t cord) {
    lcd_window_t win = {
        .cord_ = {
            .x_ = cord.x_,
            .y_ = cord.y_,
        },
        .w_ = 1,
        .h_ = 1,
    };
    __open_lcd_window(win);
    uint16_t datas[] = {sLcdInfo.foreground_color_};
    __lcd_write_memory(datas, ARRAY_SIZE(datas));
}

uint16_t lcd_get_pixel_color(lcd_pixel_point_t cord) {
    lcd_window_t win = {
        .cord_ = {
            .x_ = cord.x_,
            .y_ = cord.y_,
        },
        .w_ = 1,
        .h_ = 1,
    };

    __open_lcd_window(win);
    LCD_ILI9341_CMD(0x2E);

    uint16_t R = 0, G = 0, B = 0;

    R = *(__IO uint16_t*) Bank1_LCD_D;
    R = *(__IO uint16_t*) Bank1_LCD_D;
    B = *(__IO uint16_t*) Bank1_LCD_D;
    G = *(__IO uint16_t*) Bank1_LCD_D;

    return (((R >> 11) << 11) | ((G >> 10) << 5) | (B >> 11));
}