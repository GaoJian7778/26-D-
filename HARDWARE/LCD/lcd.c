/**
 * lcd.c - ILI9488 SPI LCD 驱动 (已验证通过版本)
 *
 * 引脚: SCK=PB13 MOSI=PB15 MISO=PB14 CS=PB12 DC=PB10 RST=PB11 LED=PE7
 * 关键设置: 0x3A=0x66(18bit)  0x36=0x68(MX=1,MV=1,BGR=1)
 * 字库: 16点阵 LSB-first (bit0=左端像素)
 */
#include "lcd.h"
#include "stm32f4xx.h"
#include <stdlib.h>

/* ── FONT.H 原厂字库 ── */
#include "FONT.H"

/* ── 全局 ── */
u16 POINT_COLOR = BLACK;
u16 BACK_COLOR  = WHITE;
lcd_dev_t lcddev;

/* ── 软件延时 ── */
static void delay_ms(u32 ms) {
    for (u32 i=0; i<ms*4000; i++) __NOP();
}

/* ── SPI 写字节 (不操作CS) ── */
static void SPI_RW(u8 d) {
    while(!(SPI2->SR & SPI_SR_TXE));
    SPI2->DR = d;
    while(!(SPI2->SR & SPI_SR_RXNE));
    (void)SPI2->DR;
}

/* ── 写命令/数据 (带CS) ── */
void LCD_WR_REG(u8 cmd) { LCD_CS_CLR; LCD_DC_CLR; SPI_RW(cmd); LCD_CS_SET; }
void LCD_WR_DATA(u8 dat) { LCD_CS_CLR; LCD_DC_SET; SPI_RW(dat); LCD_CS_SET; }

/* ── 写16位颜色 (RGB565→3字节) ── */
static void LCD_WriteRAM(u16 color) {
    LCD_WR_DATA((color>>8)&0xF8);
    LCD_WR_DATA((color>>3)&0xFC);
    LCD_WR_DATA(color<<3);
}

/* ── 批量写像素 (CS保持低) ── */
static void LCD_Burst(u16 color, u32 n) {
    u8 r=(color>>8)&0xF8, g=(color>>3)&0xFC, b=color<<3;
    for(u32 i=0;i<n;i++){ SPI_RW(r); SPI_RW(g); SPI_RW(b); }
}

/* ── 硬件初始化 ── */
void LCD_Init(void)
{
    /* 时钟 */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOEEN;

    /* PB10(DC) PB11(RST) PB12(CS) 输出 */
    GPIOB->MODER &= ~(0xF<<20|0xF<<22|0xF<<24);
    GPIOB->MODER |= (1<<20)|(1<<22)|(1<<24);
    GPIOB->OSPEEDR |= (3<<20)|(3<<22)|(3<<24);
    LCD_CS_SET; LCD_DC_SET;

    /* PE7(LED) 输出, 先灭 */
    GPIOE->MODER &= ~(3<<14);
    GPIOE->MODER |= (1<<14);
    GPIOE->OTYPER &= ~(1<<7);
    GPIOE->BSRRH = GPIO_Pin_7;

    /* PB13(SCK) PB14(MISO) PB15(MOSI) AF5 */
    GPIOB->MODER &= ~(0xF<<26|0xF<<28|0xF<<30);
    GPIOB->MODER |= (2<<26)|(2<<28)|(2<<30);
    GPIOB->AFR[1] |= (5<<20)|(5<<24)|(5<<28);

    /* SPI2: /8=5.25MHz */
    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
    SPI2->CR1 = SPI_CR1_MSTR|SPI_CR1_SSM|SPI_CR1_SSI|(3<<3)|SPI_CR1_SPE;

    /* 复位 */
    LCD_RST_CLR; delay_ms(100); LCD_RST_SET; delay_ms(50);

    /* ILI9488 初始化序列 */
    LCD_WR_REG(0xF7); LCD_WR_DATA(0xA9);LCD_WR_DATA(0x51);LCD_WR_DATA(0x2C);LCD_WR_DATA(0x82);
    LCD_WR_REG(0xC0); LCD_WR_DATA(0x11);LCD_WR_DATA(0x09);
    LCD_WR_REG(0xC1); LCD_WR_DATA(0x41);
    LCD_WR_REG(0xC5); LCD_WR_DATA(0x00);LCD_WR_DATA(0x0A);LCD_WR_DATA(0x80);
    LCD_WR_REG(0xB1); LCD_WR_DATA(0xB0);LCD_WR_DATA(0x11);
    LCD_WR_REG(0xB4); LCD_WR_DATA(0x02);
    LCD_WR_REG(0xB6); LCD_WR_DATA(0x02);LCD_WR_DATA(0x42);
    LCD_WR_REG(0xB7); LCD_WR_DATA(0xC6);
    LCD_WR_REG(0xBE); LCD_WR_DATA(0x00);LCD_WR_DATA(0x04);
    LCD_WR_REG(0xE9); LCD_WR_DATA(0x00);
    LCD_WR_REG(0x36); LCD_WR_DATA(0x68); /* MX=1 MY=0 MV=1 BGR=1 */
    LCD_WR_REG(0x3A); LCD_WR_DATA(0x66); /* 18-bit */
    LCD_WR_REG(0xE0);
    LCD_WR_DATA(0x00);LCD_WR_DATA(0x07);LCD_WR_DATA(0x10);LCD_WR_DATA(0x09);
    LCD_WR_DATA(0x17);LCD_WR_DATA(0x0B);LCD_WR_DATA(0x41);LCD_WR_DATA(0x89);
    LCD_WR_DATA(0x4B);LCD_WR_DATA(0x0A);LCD_WR_DATA(0x0C);LCD_WR_DATA(0x0E);
    LCD_WR_DATA(0x18);LCD_WR_DATA(0x1B);LCD_WR_DATA(0x0F);
    LCD_WR_REG(0xE1);
    LCD_WR_DATA(0x00);LCD_WR_DATA(0x17);LCD_WR_DATA(0x1A);LCD_WR_DATA(0x04);
    LCD_WR_DATA(0x0E);LCD_WR_DATA(0x06);LCD_WR_DATA(0x2F);LCD_WR_DATA(0x45);
    LCD_WR_DATA(0x43);LCD_WR_DATA(0x02);LCD_WR_DATA(0x0A);LCD_WR_DATA(0x09);
    LCD_WR_DATA(0x32);LCD_WR_DATA(0x36);LCD_WR_DATA(0x0F);
    LCD_WR_REG(0x11); delay_ms(120);
    LCD_WR_REG(0x29);

    /* 窗口尺寸: 480x320 */
    lcddev.width=480; lcddev.height=320;
    lcddev.setxcmd=0x2A; lcddev.setycmd=0x2B; lcddev.wramcmd=0x2C;

    /* 开背光 + 白屏 */
    LCD_LED_ON;
    LCD_Clear(WHITE);
}

/* ── 设置窗口 ── */
void LCD_SetWindows(u16 xs, u16 ys, u16 xe, u16 ye) {
    LCD_WR_REG(0x2A);
    LCD_WR_DATA(xs>>8);LCD_WR_DATA(xs&0xFF);LCD_WR_DATA(xe>>8);LCD_WR_DATA(xe&0xFF);
    LCD_WR_REG(0x2B);
    LCD_WR_DATA(ys>>8);LCD_WR_DATA(ys&0xFF);LCD_WR_DATA(ye>>8);LCD_WR_DATA(ye&0xFF);
}

/* ── 画点 ── */
void LCD_DrawPoint(u16 x, u16 y) {
    LCD_SetWindows(x,y,x,y);
    LCD_WR_REG(0x2C);
    LCD_WriteRAM(POINT_COLOR);
}

/* ── 填充矩形 ── */
void LCD_FillRect(u16 x, u16 y, u16 w, u16 h, u16 color) {
    if(!w||!h) return;
    LCD_SetWindows(x,y,x+w-1,y+h-1);
    LCD_WR_REG(0x2C);
    LCD_CS_CLR; LCD_DC_SET;
    LCD_Burst(color, (u32)w*h);
    LCD_CS_SET;
}

/* ── 清屏 ── */
void LCD_Clear(u16 color) {
    LCD_FillRect(0, 0, lcddev.width, lcddev.height, color);
}

/* ── 画线 ── */
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2) {
    int dx=(int)x2-(int)x1, dy=(int)y2-(int)y1;
    int steps=abs(dx)>abs(dy)?abs(dx):abs(dy);
    if(!steps){ POINT_COLOR=POINT_COLOR; LCD_DrawPoint(x1,y1); return; }
    float xi=(float)dx/steps, yi=(float)dy/steps, x=x1, y=y1;
    for(int i=0;i<=steps;i++) { LCD_DrawPoint((u16)x,(u16)y); x+=xi; y+=yi; }
}

/* ── 画矩形 ── */
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2) {
    LCD_DrawLine(x1,y1,x2,y1);LCD_DrawLine(x2,y1,x2,y2);
    LCD_DrawLine(x2,y2,x1,y2);LCD_DrawLine(x1,y2,x1,y1);
}

/* ── 显示字符 (16点阵 LSB-first) ── */
void LCD_ShowChar(u16 x, u16 y, u8 ch, u16 size, u16 color, u16 bg) {
    if(ch<' '||ch>'~') ch=' ';
    u8 pos=ch-' ';
    u16 cw=8, ch_h=16;
    if(size==12){ cw=6; ch_h=12; }

    LCD_SetWindows(x, y, x+cw-1, y+ch_h-1);
    LCD_WR_REG(0x2C);
    LCD_CS_CLR; LCD_DC_SET;

    for(u8 row=0; row<ch_h; row++) {
        u8 bits = (size==12) ? asc2_1206[pos][row] : asc2_1608[pos][row];
        for(u8 col=0; col<cw; col++) {
            if(bits&0x01) {
                SPI_RW((color>>8)&0xF8); SPI_RW((color>>3)&0xFC); SPI_RW(color<<3);
            } else {
                SPI_RW((bg>>8)&0xF8); SPI_RW((bg>>3)&0xFC); SPI_RW(bg<<3);
            }
            bits>>=1;
        }
    }
    LCD_CS_SET;
}

/* ── 显示字符串 ── */
void LCD_ShowString(u16 x, u16 y, u16 size, u8 *str, u16 color, u16 bg) {
    u16 cur=x;
    while(*str) {
        if(*str=='\n') { cur=x; y+=18; }
        else { LCD_ShowChar(cur,y,*str,size,color,bg); cur+=(size==12?6:8); }
        str++;
    }
}

/* ── 辅助 ── */
void LCD_WriteRAM_Prepare(void) { LCD_WR_REG(lcddev.wramcmd); }
void LCD_SetCursor(u16 x, u16 y) { LCD_SetWindows(x,y,x,y); LCD_WR_REG(0x2C); }
void LCD_WriteReg(u8 reg, u16 val) { LCD_WR_REG(reg); LCD_WR_DATA(val); }
void LCD_DisplayOn(void) { LCD_LED_ON; }
void LCD_DisplayOff(void) { LCD_LED_OFF; }
u16 LCD_ReadPoint(u16 x, u16 y) { (void)x;(void)y; return 0; }
