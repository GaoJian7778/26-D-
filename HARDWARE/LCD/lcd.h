/**
 * lcd.h - ILI9488 SPI LCD 驱动
 */
#ifndef __LCD_H
#define __LCD_H

#include "stm32f4xx.h"
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;

#define LCD_W  320
#define LCD_H  480
#define LCD_HORIZONTAL_W  480
#define LCD_HORIZONTAL_H  320

/* ── 引脚宏 ── */
#define LCD_CS_SET   GPIOB->BSRRL = GPIO_Pin_12
#define LCD_CS_CLR   GPIOB->BSRRH = GPIO_Pin_12
#define LCD_DC_SET   GPIOB->BSRRL = GPIO_Pin_10
#define LCD_DC_CLR   GPIOB->BSRRH = GPIO_Pin_10
#define LCD_RST_SET  GPIOB->BSRRL = GPIO_Pin_11
#define LCD_RST_CLR  GPIOB->BSRRH = GPIO_Pin_11
#define LCD_LED_ON   GPIOE->BSRRL = GPIO_Pin_7
#define LCD_LED_OFF  GPIOE->BSRRH = GPIO_Pin_7

/* ── 颜色 ── */
#define WHITE       0xFFFF
#define BLACK       0x0000
#define BLUE        0x001F
#define RED         0xF800
#define GREEN       0x07E0
#define CYAN        0x7FFF
#define YELLOW      0xFFE0
#define MAGENTA     0xF81F
#define GRAY        0x8430
#define LIGHTGRAY   0xEF5B
#define LIGHTGREEN  0x841F
#define DARKGRAY    0x4208
#define LT_BLUE     0x7D7C
#define ORANGE      0xFD20

/* ── LCD 参数结构 ── */
typedef struct {
    u16 width, height, id;
    u8  dir;
    u16 wramcmd, setxcmd, setycmd;
} lcd_dev_t;

extern lcd_dev_t lcddev;
extern u16 POINT_COLOR;
extern u16 BACK_COLOR;

/* ── 函数 ── */
void LCD_Init(void);
void LCD_Clear(u16 color);
void LCD_FillRect(u16 x, u16 y, u16 w, u16 h, u16 color);
void LCD_DrawPoint(u16 x, u16 y);
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2);
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2);
void LCD_SetWindows(u16 xs, u16 ys, u16 xe, u16 ye);
void LCD_SetCursor(u16 x, u16 y);
void LCD_WR_REG(u8 reg);
void LCD_WR_DATA(u8 dat);
void LCD_WriteReg(u8 reg, u16 val);
void LCD_WriteRAM_Prepare(void);
void LCD_DisplayOn(void);
void LCD_DisplayOff(void);
void LCD_ShowChar(u16 x, u16 y, u8 ch, u16 size, u16 color, u16 bg);
void LCD_ShowString(u16 x, u16 y, u16 size, u8 *str, u16 color, u16 bg);
u16  LCD_ReadPoint(u16 x, u16 y);

#endif
