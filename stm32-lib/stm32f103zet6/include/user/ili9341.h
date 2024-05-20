#ifndef ILI9341_H
#define ILI9341_H

#include "stm32f1xx_hal.h"

#include "base/base.h"
#include "base/stm32f103zet6.h"

#define WHITE 0xFFFF   // 白色
#define BLACK 0x0000   // 黑色
#define GREY 0xF7DE    // 灰色
#define BLUE 0x001F    // 蓝色
#define BLUE2 0x051F   // 浅蓝色
#define RED 0xF800     // 红色
#define MAGENTA 0xF81F // 红紫色，洋红色
#define GREEN 0x07E0   // 绿色
#define CYAN 0x7FFF    // 蓝绿色，青色
#define YELLOW 0xFFE0  // 黄色

typedef enum {
    FONT_5_8,
    FONT_7_12,
    FONT_11_16,
    FONT_14_20,
    FONT_17_24,
    FONT_MAX,
} lcd_font_t;

typedef enum {
    LEFT_RIGHT_UP_DOWN = 0,
    UP_DOWN_LEF_RIGHT,
    RIGHT_LEFT_UP_DOWN,
    UP_DOWN_RIGHT_LEFT,
    LEFT_RIGHT_DOWN_UP,
    DOWN_UP_LEF_RIGHT,
    RIGHT_LEFT_DOWN_UP,
    DOWN_UP_RIGHT_LEFT,
    PORTRAIT = LEFT_RIGHT_UP_DOWN,
    PORTRAIT_INVERSE = RIGHT_LEFT_DOWN_UP,
    LANDSSCAPE = DOWN_UP_LEF_RIGHT,
    LANDSSCAPE_INVERSE = UP_DOWN_RIGHT_LEFT,
} lcd_scan_direction_t;

typedef struct {
    uint32_t x_;
    uint32_t y_;
} lcd_pixel_point_t;

typedef struct {
    lcd_pixel_point_t cord_;
    uint32_t w_;
    uint32_t h_;
} lcd_rectangle_t;

typedef lcd_rectangle_t lcd_window_t;

status_t init_ili9341();
void set_lcd_screen_direction(lcd_scan_direction_t dir);
void lcd_show_string(lcd_pixel_point_t cord, char* str);
status_t set_lcd_font_size(lcd_font_t font_size);
void clear_lcd_screen(uint16_t color);
void lcd_black_led(bool on);
void set_lcd_color(uint16_t fore_color, uint16_t back_color);
void lcd_set_pixel_color(lcd_pixel_point_t cord);
uint16_t lcd_get_pixel_color(lcd_pixel_point_t cord);

#endif // ILI9341_H