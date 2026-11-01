/**
 * ui.h - UI 状态机
 */
#ifndef __UI_H
#define __UI_H
#include "stm32f4xx.h"
#include "Key.h"

typedef enum { MODE_MONITOR=0, MODE_GUIDE, MODE_RESULT, MODE_LOADSAVE } UIMode;

extern UIMode  ui_mode;
extern uint8_t nofly_bitmap[80], nofly_count;
extern uint8_t target_grid[80];
extern uint16_t target_counts[3], mission_elapsed;
extern uint8_t drone_col, drone_row, drone_status;
extern uint8_t exit_col, exit_row;

/* 增量更新标志 */
extern volatile uint8_t flag_full_redraw;
extern volatile uint8_t flag_cursor_dirty;
extern volatile uint8_t flag_info_dirty;

void UI_Init(void);
void UI_ProcessKey(KeyEvent *evt);
void UI_Update(void);
void UI_OnTarget(uint8_t type, uint8_t col, uint8_t row, uint8_t conf);
void UI_OnQR(uint8_t ec, uint8_t er);
void UI_OnStatus(uint8_t col, uint8_t row, uint8_t h, uint8_t status);
void UI_OnGuidePath(uint8_t *path, uint8_t len);
void UI_OnMissionDone(uint16_t counts[3], uint16_t elapsed);
#endif
