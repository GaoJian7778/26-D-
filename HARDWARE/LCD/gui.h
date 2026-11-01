/**
 * gui.h - 地面站 GUI 绘图
 *
 * 横屏 480x320: 左侧网格(~360) + 右侧信息栏(~120)
 * 只用16点阵字体
 */
#ifndef __GUI_H
#define __GUI_H

#include "stm32f4xx.h"
#include "lcd.h"

/* ── 网格参数 D题 10×8 ── */
#define GRID_COLS     10
#define GRID_ROWS     8
#define GRID_X0       26
#define GRID_Y0       2
#define GRID_CELL_W   34
#define GRID_CELL_H   36
#define GRID_GAP      2
#define GRID_TOTAL_W  (GRID_COLS*GRID_CELL_W+(GRID_COLS-1)*GRID_GAP)
#define GRID_TOTAL_H  (GRID_ROWS*GRID_CELL_H+(GRID_ROWS-1)*GRID_GAP)
#define INFO_X        (GRID_X0+GRID_TOTAL_W+4)
#define INFO_Y        2

/* ── 全局 ── */
extern uint8_t record_line;

/* ── 函数 ── */
void GUI_DrawGrid(void);
void GUI_MarkNoFly(uint8_t col, uint8_t row, uint8_t is_nofly);
void GUI_MarkTakeoff(uint8_t col, uint8_t row);
void GUI_MarkVisited(uint8_t col, uint8_t row);
void GUI_MarkAnimal(uint8_t col, uint8_t row, uint8_t type);
void GUI_DrawCursor(uint8_t col, uint8_t row, uint8_t visible);
void GUI_UpdateDronePos(uint8_t col, uint8_t row, uint8_t clear_old);
void GUI_ClearInfo(void);
void GUI_ShowMode(const char *mode);
void GUI_ShowStatus(uint8_t status);
void GUI_ShowInfoLine(uint8_t line, const char *text, uint16_t color);
void GUI_ShowAnimalRecord(uint8_t type, uint8_t col, uint8_t row);
void GUI_ShowTitle(void);
void GUI_ShowWaitScreen(void);
void GUI_ShowEmergency(void);
void GUI_ShowSummary(uint16_t counts[3], uint16_t elapsed);
void GUI_DrawGuidePath(uint8_t *path, uint8_t len);
void GUI_SetCellTarget(uint8_t col, uint8_t row, uint16_t color);
void GUI_ShowQRInfo(uint8_t exit_col, uint8_t exit_row);
void GUI_DrawLegend(void);
void GUI_ShowLoadSaveScreen(void);
void GUI_DrawGuideLine(uint8_t c1,uint8_t r1,uint8_t c2,uint8_t r2);

#endif
