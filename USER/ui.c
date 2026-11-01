/**
 * ui.c - D题搜救飞行器 10×8网格 预载禁飞区
 */
#include "ui.h"
#include "gui.h"
#include "protocol.h"
#include "flash_store.h"
#include "ble.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>

UIMode ui_mode=MODE_MONITOR;
uint8_t nofly_bitmap[80], nofly_count;
uint8_t target_grid[80];  /* 目标位置: 0=无, 1=红, 2=绿, 3=蓝 */
uint8_t guide_path[80];   /* 引导线: col0,row0,col1,row1,... */
uint8_t guide_path_len=0;
uint16_t target_counts[3], mission_elapsed;
uint8_t drone_col, drone_row, drone_status, exit_col=99, exit_row=99;
volatile uint8_t flag_full_redraw=0, flag_cursor_dirty=0, flag_info_dirty=0;

static uint8_t first_draw=1;
static uint8_t last_drone_st=0xFF;

static void bs(uint8_t c,uint8_t r,uint8_t v){if(c<GRID_COLS&&r<GRID_ROWS)nofly_bitmap[r*GRID_COLS+c]=v;}
static void sw(UIMode m){ui_mode=m;flag_full_redraw=1;}

/* D题: 图一灰色禁飞区固定, 预载 */
static void preload_nofly(void){
    /* D题图一灰色密林区 共20格 Y翻转: screen_row=7-Y */
    static const uint8_t gray[]={
        0,5, 0,3, 1,2, 1,0, 2,6, 2,4, 3,6, 3,2, 4,3, 4,0,
        5,7, 5,5, 6,5, 6,2, 6,1, 8,6, 8,5, 8,3, 8,0, 9,6,
    };
    for(int i=0;i<sizeof(gray);i+=2){
        bs(gray[i],gray[i+1],1); nofly_count++;
    }
}

void UI_Init(void){
    memset(nofly_bitmap,0,sizeof(nofly_bitmap));
    memset(target_grid,0,sizeof(target_grid));
    memset(target_counts,0,sizeof(target_counts));
    memset(guide_path,0,sizeof(guide_path));guide_path_len=0;
    nofly_count=0;
    drone_status=STATUS_GROUND;drone_col=99;drone_row=99;exit_col=99;exit_row=99;
    flag_full_redraw=1;first_draw=1;
    preload_nofly();
    if(FlashStore_HasData()){
        ui_mode=MODE_LOADSAVE;
        GUI_ShowLoadSaveScreen();
    } else {
        ui_mode=MODE_MONITOR;
        GUI_ShowTitle();
    }
}

void UI_ProcessKey(KeyEvent *evt){
    if(evt->key==KEY_EMERGENCY&&evt->event==KEY_EVENT_DOWN){emergency_flag=1;return;}
    switch(ui_mode){
    case MODE_LOADSAVE:
        if(evt->event==KEY_EVENT_DOWN){
            if(evt->key==KEY_CONFIRM){PersistentData pd;FlashStore_Load(&pd);
                memcpy(nofly_bitmap,pd.nofly_bitmap,80);
                memcpy(target_grid,pd.target_grid,80);
                memcpy(target_counts,pd.target_counts,sizeof(target_counts));
                mission_elapsed=pd.mission_elapsed;
                exit_col=pd.exit_col;exit_row=pd.exit_row;
                memcpy(guide_path,pd.guide_path,80);guide_path_len=pd.guide_path_len;
                nofly_count=0;for(int i=0;i<80;i++)if(nofly_bitmap[i])nofly_count++;
                sw(MODE_MONITOR);
                /* 重绘引导线 */
                for(int i=1;i<guide_path_len;i++)
                    GUI_DrawGuideLine(guide_path[(i-1)*2],guide_path[(i-1)*2+1],
                                      guide_path[i*2],guide_path[i*2+1]);
            }else if(evt->key==KEY_BACK){FlashStore_Erase();sw(MODE_MONITOR);}
        }
        break;
    case MODE_RESULT:
        if(evt->event==KEY_EVENT_DOWN&&(evt->key==KEY_CONFIRM||evt->key==KEY_BACK))sw(MODE_MONITOR);
        break;
    case MODE_MONITOR:
        if(evt->event==KEY_EVENT_LONG&&evt->key==KEY_BACK){FlashStore_Erase();flag_full_redraw=1;}
        break;
    default:break;
    }
}

void UI_Update(void){
    if(flag_full_redraw||first_draw){
        flag_full_redraw=0;flag_info_dirty=0;first_draw=0;
        switch(ui_mode){
        case MODE_RESULT:GUI_ShowSummary(target_counts,mission_elapsed);break;
        case MODE_GUIDE:
            GUI_ClearInfo();GUI_ShowMode("Guide");GUI_DrawLegend();record_line=2;
            break;
        case MODE_MONITOR:
        default:
            GUI_DrawGrid();GUI_MarkTakeoff(0,7);
            for(int i=0;i<80;i++)if(nofly_bitmap[i])GUI_MarkNoFly(i%GRID_COLS,i/GRID_COLS,1);
            for(int i=0;i<80;i++)if(target_grid[i])GUI_MarkAnimal(i%GRID_COLS,i/GRID_COLS,target_grid[i]);
            GUI_ClearInfo();GUI_ShowMode("Monitor");GUI_DrawLegend();record_line=2;
            break;
        }
        return;
    }
    /* 图例持久: MONITOR/GUIDE 模式下每500ms补画 */
    static uint32_t legend_tick=0;
    if(ui_mode==MODE_MONITOR||ui_mode==MODE_GUIDE){
        uint32_t t=xTaskGetTickCount();
        if(t-legend_tick>500){legend_tick=t;GUI_DrawLegend();}
    }
    static uint8_t prev_dc=99,prev_dr=99;
    if(ui_mode==MODE_MONITOR||ui_mode==MODE_GUIDE){
        if(drone_status!=last_drone_st){
            GUI_UpdateDronePos(drone_col,drone_row,0);
            GUI_ShowStatus(drone_status);last_drone_st=drone_status;
        }
        if(drone_status==STATUS_GUIDING
           &&drone_col<GRID_COLS&&drone_row<GRID_ROWS
           &&prev_dc<GRID_COLS&&prev_dr<GRID_ROWS
           &&(drone_col!=prev_dc||drone_row!=prev_dr)){
            GUI_DrawGuideLine(prev_dc,prev_dr,drone_col,drone_row);
        }
        prev_dc=drone_col;prev_dr=drone_row;
    }
}

/* ── BLE回调 ── */
void UI_OnTarget(uint8_t t,uint8_t c,uint8_t r,uint8_t conf){
    (void)conf;
    r=GRID_ROWS-1-r;  /* Y翻转: 物理Y→屏幕行 */
    if(t>=1&&t<=3&&c<GRID_COLS&&r<GRID_ROWS){
        target_grid[r*GRID_COLS+c]=t;
        target_counts[t-1]++;
        flag_full_redraw=1;
    }
}
void UI_OnQR(uint8_t ec,uint8_t er){exit_col=ec;exit_row=GRID_ROWS-1-er;GUI_ShowQRInfo(ec,er);}
void UI_OnStatus(uint8_t c,uint8_t r,uint8_t h,uint8_t s){
    (void)h;drone_col=c;drone_row=GRID_ROWS-1-r;drone_status=s;
    if(s==STATUS_GUIDING && guide_path_len<40){  /* 存引导点 */
        guide_path[guide_path_len*2]=drone_col;
        guide_path[guide_path_len*2+1]=drone_row;
        guide_path_len++;
    }
}
void UI_OnGuidePath(uint8_t *path,uint8_t len){GUI_DrawGuidePath(path,len);}
void UI_OnMissionDone(uint16_t cnt[3],uint16_t el){
    for(int i=0;i<3;i++)target_counts[i]=cnt[i];mission_elapsed=el;
    /* auto-save to flash */
    PersistentData pd;
    memset(&pd,0,sizeof(pd));
    pd.magic=FLASH_STORE_MAGIC;
    memcpy(pd.nofly_bitmap,nofly_bitmap,80);
    memcpy(pd.target_grid,target_grid,80);
    memcpy(pd.target_counts,target_counts,sizeof(target_counts));
    pd.mission_elapsed=el;
    pd.exit_col=exit_col;pd.exit_row=exit_row;
    memcpy(pd.guide_path,guide_path,80);pd.guide_path_len=guide_path_len;
    pd.crc=FlashStore_CRC32((const uint8_t*)&pd,offsetof(PersistentData,crc));
    FlashStore_Save(&pd);
    sw(MODE_RESULT);
}
