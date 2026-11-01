/**
 * gui.c - 地面站 GUI D题版 10×8网格
 */
#include "gui.h"
#include <stdio.h>

static const char *target_mark[]={"?","R","G","B"};
uint8_t record_line=2;

static u16 cx(uint8_t c){return GRID_X0+1+c*(GRID_CELL_W+GRID_GAP);}
static u16 cy(uint8_t r){return GRID_Y0+1+r*(GRID_CELL_H+GRID_GAP);}

/* ── 10×8网格 ── */
void GUI_DrawGrid(void)
{
    char buf[4];
    LCD_Clear(WHITE);
    LCD_FillRect(GRID_X0,GRID_Y0,GRID_TOTAL_W+2,GRID_TOTAL_H+2,BLACK);
    for(uint8_t r=0;r<GRID_ROWS;r++)
        for(uint8_t c=0;c<GRID_COLS;c++)
            LCD_FillRect(cx(c),cy(r),GRID_CELL_W,GRID_CELL_H,WHITE);
    /* Y坐标 (物理Y7→屏row0, Y0→屏row7) */
    for(uint8_t r=0;r<GRID_ROWS;r++){
        snprintf(buf,4,"Y%d",GRID_ROWS-1-r);
        LCD_ShowString(2,cy(r)+GRID_CELL_H/2-8,12,(u8*)buf,BLACK,WHITE);
    }
    /* X坐标 */
    for(uint8_t c=0;c<GRID_COLS;c++){
        snprintf(buf,4,"X%d",c);
        LCD_ShowString(cx(c)+2,GRID_Y0+GRID_TOTAL_H+4,12,(u8*)buf,BLACK,WHITE);
    }
}

void GUI_MarkNoFly(uint8_t c,uint8_t r,uint8_t f){
    if(c<GRID_COLS&&r<GRID_ROWS)LCD_FillRect(cx(c),cy(r),GRID_CELL_W,GRID_CELL_H,f?GRAY:WHITE);
}
void GUI_MarkTakeoff(uint8_t c,uint8_t r){
    if(c<GRID_COLS&&r<GRID_ROWS)LCD_FillRect(cx(c),cy(r),GRID_CELL_W,GRID_CELL_H,RED);
}
void GUI_MarkVisited(uint8_t c,uint8_t r){
    if(c<GRID_COLS&&r<GRID_ROWS)LCD_FillRect(cx(c),cy(r),GRID_CELL_W,GRID_CELL_H,LIGHTGREEN);
}
void GUI_SetCellTarget(uint8_t c,uint8_t r,uint16_t color){
    if(c<GRID_COLS&&r<GRID_ROWS){
        LCD_FillRect(cx(c),cy(r),GRID_CELL_W,GRID_CELL_H,color);
        POINT_COLOR=BLACK;
        LCD_DrawPoint(cx(c)+GRID_CELL_W/2,cy(r)+GRID_CELL_H/2);
        POINT_COLOR=WHITE;
    }
}
void GUI_MarkAnimal(uint8_t c,uint8_t r,uint8_t type){
    if(type<1||type>3||c>=GRID_COLS||r>=GRID_ROWS)return;
    static const u16 clr[]={WHITE,RED,GREEN,BLUE};
    u16 x=cx(c)+GRID_CELL_W/2-4,y=cy(r)+GRID_CELL_H/2-8;
    LCD_ShowString(x,y,16,(u8*)target_mark[type],clr[type],LIGHTGREEN);
}

/* ── 光标 ── */
void GUI_DrawCursor(uint8_t c,uint8_t r,uint8_t vis){
    if(c>=GRID_COLS||r>=GRID_ROWS)return;
    u16 x=cx(c),y=cy(r);
    POINT_COLOR=vis?GREEN:BLACK;
    for(uint8_t i=0;i<3;i++){
        LCD_DrawLine(x+i,y+i,x+GRID_CELL_W-1-i,y+i);
        LCD_DrawLine(x+i,y+GRID_CELL_H-1-i,x+GRID_CELL_W-1-i,y+GRID_CELL_H-1-i);
        LCD_DrawLine(x+i,y+i,x+i,y+GRID_CELL_H-1-i);
        LCD_DrawLine(x+GRID_CELL_W-1-i,y+i,x+GRID_CELL_W-1-i,y+GRID_CELL_H-1-i);
    }
}

/* ── 飞机位置 ── */
static uint8_t odc=99,odr=99;
void GUI_UpdateDronePos(uint8_t c,uint8_t r,uint8_t clr){
    if(clr&&odc<GRID_COLS&&odr<GRID_ROWS)GUI_MarkVisited(odc,odr);
    if(c<GRID_COLS&&r<GRID_ROWS){
        u16 x=cx(c)+GRID_CELL_W/2,y=cy(r)+GRID_CELL_H/2;
        POINT_COLOR=RED;
        for(int8_t d=-3;d<=3;d++){LCD_DrawPoint(x+d,y);LCD_DrawPoint(x,y+d);}
        odc=c;odr=r;
    }
}
void GUI_DrawGuideLine(uint8_t c1,uint8_t r1,uint8_t c2,uint8_t r2){
    if(c1<GRID_COLS&&r1<GRID_ROWS&&c2<GRID_COLS&&r2<GRID_ROWS){
        u16 x1=cx(c1)+GRID_CELL_W/2, y1=cy(r1)+GRID_CELL_H/2;
        u16 x2=cx(c2)+GRID_CELL_W/2, y2=cy(r2)+GRID_CELL_H/2;
        POINT_COLOR=BLUE;
        LCD_DrawLine(x1, y1, x2, y2);
    }
}

/* ── 引导路径: 给路径格子画线 ── */
void GUI_DrawGuidePath(uint8_t *path,uint8_t len){
    for(uint8_t i=0;i<len-1;i++){
        uint8_t c1=path[i*2],r1=path[i*2+1],c2=path[i*2+2],r2=path[i*2+3];
        if(c1<GRID_COLS&&r1<GRID_ROWS&&c2<GRID_COLS&&r2<GRID_ROWS){
            u16 x1=cx(c1)+GRID_CELL_W/2,y1=cy(r1)+GRID_CELL_H/2;
            u16 x2=cx(c2)+GRID_CELL_W/2,y2=cy(r2)+GRID_CELL_H/2;
            POINT_COLOR=BLUE;
            LCD_DrawLine(x1,y1,x2,y2);
        }
    }
}

/* ── 信息栏 ── */
void GUI_ClearInfo(void){LCD_FillRect(INFO_X,INFO_Y,480-INFO_X,320,WHITE);}
void GUI_ShowMode(const char *m){
    char b[24];snprintf(b,24,"Mode:%s",m);
    LCD_ShowString(INFO_X,INFO_Y,16,(u8*)b,BLUE,WHITE);
}
void GUI_ShowStatus(uint8_t s){
    static const char *ss[]={"Ground","Takeoff","Cruise","Landing","Landed","ScanQR","Drop","Guide"};
    char b[24];snprintf(b,24,"Stat:%s",s<8?ss[s]:"?");
    LCD_ShowString(INFO_X,22,16,(u8*)b,BLACK,WHITE);
}
void GUI_ShowInfoLine(uint8_t l,const char *t,uint16_t c){
    LCD_ShowString(INFO_X,INFO_Y+l*20,16,(u8*)t,c,WHITE);
}
void GUI_ShowAnimalRecord(uint8_t type,uint8_t c,uint8_t r){
    if(type<1||type>3)return;char b[24];
    snprintf(b,24,"X%dY%d %s",c,r,target_mark[type]);
    GUI_ShowInfoLine(record_line,b,RED);if(++record_line>10)record_line=2;
}

/* ── 全屏 ── */
void GUI_ShowTitle(void){
    LCD_Clear(WHITE);
    LCD_ShowString(130,80,16,(u8*)"Search & Rescue",BLACK,WHITE);
    LCD_ShowString(160,120,16,(u8*)"2026 TI Cup D",DARKGRAY,WHITE);
    LCD_ShowString(140,170,16,(u8*)"CONFIRM->Start",DARKGRAY,WHITE);
}
void GUI_ShowWaitScreen(void){
    GUI_ClearInfo();GUI_ShowMode("Wait");
    GUI_ShowInfoLine(2,"Config sent.",BLACK);
    GUI_ShowInfoLine(3,"Waiting takeoff...",BLUE);
    GUI_ShowInfoLine(5,"BACK:Cancel",DARKGRAY);
}
void GUI_ShowEmergency(void){GUI_ShowInfoLine(0,"EMERGENCY!",RED);GUI_ShowInfoLine(2,"Return sent!",RED);}
void GUI_ShowQRInfo(uint8_t ec,uint8_t er){
    char b[24];snprintf(b,24,"Exit: X%d Y%d",ec,er);
    GUI_ShowInfoLine(1,b,BLUE);
}

void GUI_DrawLegend(void){
    u16 y=265;
    LCD_FillRect(INFO_X,y,120,55,WHITE);
    /* 红 */
    LCD_FillRect(INFO_X+2,y+2,12,12,RED);
    LCD_ShowString(INFO_X+18,y,12,(u8*)"Red",BLACK,WHITE);
    /* 绿 */
    LCD_FillRect(INFO_X+50,y+2,12,12,GREEN);
    LCD_ShowString(INFO_X+66,y,12,(u8*)"Grn",BLACK,WHITE);
    /* 蓝 */
    LCD_FillRect(INFO_X+2,y+18,12,12,BLUE);
    LCD_ShowString(INFO_X+18,y+16,12,(u8*)"Blu",BLACK,WHITE);
    /* 禁飞灰 */
    LCD_FillRect(INFO_X+50,y+18,12,12,GRAY);
    LCD_ShowString(INFO_X+66,y+16,12,(u8*)"NoFly",BLACK,WHITE);
    /* 引导路径 */
    POINT_COLOR=BLUE;
    LCD_DrawLine(INFO_X+2,y+36,INFO_X+20,y+36);
    LCD_ShowString(INFO_X+24,y+32,12,(u8*)"Path",BLUE,WHITE);
}

void GUI_ShowLoadSaveScreen(void){
    LCD_Clear(WHITE);
    LCD_ShowString(100,80,16,(u8*)"Saved data found!",BLACK,WHITE);
    LCD_ShowString(60,120,16,(u8*)"CONFIRM: Load saved",BLACK,WHITE);
    LCD_ShowString(80,150,16,(u8*)"BACK: Delete & fresh",DARKGRAY,WHITE);
    LCD_ShowString(100,200,16,(u8*)"Press a key...",BLACK,WHITE);
}

void GUI_ShowSummary(uint16_t counts[3],uint16_t el){
    LCD_Clear(WHITE);
    static const char *n[]={"Red","Green","Blue"};
    u16 y=20;
    LCD_ShowString(160,y,16,(u8*)"Mission Summary",BLACK,WHITE);
    y+=30;POINT_COLOR=DARKGRAY;LCD_DrawLine(60,y,420,y);
    for(int i=0;i<3;i++){y+=30;char b[32];snprintf(b,32,"%s: %d",n[i],counts[i]);LCD_ShowString(80,y,16,(u8*)b,BLACK,WHITE);}
    y+=40;char b[32];snprintf(b,32,"Time: %ds",el);LCD_ShowString(160,y,16,(u8*)b,BLACK,WHITE);
    y+=40;LCD_ShowString(120,y,16,(u8*)"CONFIRM->Home",DARKGRAY,WHITE);
}
