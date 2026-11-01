/**
 * Key.c - 7键非阻塞驱动
 */
#include "Key.h"

#define KEY_COUNT 7

typedef struct {
    GPIO_TypeDef *port; uint16_t pin;
} KeyPin;

static const KeyPin kp[KEY_COUNT]={
    {GPIOD,GPIO_Pin_8},{GPIOD,GPIO_Pin_10},{GPIOD,GPIO_Pin_12},{GPIOD,GPIO_Pin_14},
    {GPIOC,GPIO_Pin_6},{GPIOC,GPIO_Pin_8},{GPIOA,GPIO_Pin_8}
};

typedef struct { uint8_t raw,stable,long_fired; uint32_t ticks,rep_tick; } KeyState;
static KeyState ks[KEY_COUNT];
static KeyEvent evq[16];
static uint8_t eh=0,et=0;

int Key_GetEvent(KeyEvent *e){
    if(eh==et) return 0;
    *e=evq[et]; et=(et+1)%16; return 1;
}
static void push(uint8_t k,uint8_t e){
    uint8_t n=(eh+1)%16; if(n==et) return;
    evq[eh].key=k; evq[eh].event=e; eh=n;
}

void Key_Init(void)
{
    RCC->AHB1ENR|=RCC_AHB1ENR_GPIODEN|RCC_AHB1ENR_GPIOCEN|RCC_AHB1ENR_GPIOAEN;

    /* GPIOD:8,10,12,14  GPIOC:6,8  GPIOA:8 上拉输入 */
    GPIOD->MODER&=~(0xF<<16|0xF<<20|0xF<<24|0xF<<28);
    GPIOD->PUPDR|=(1<<16)|(1<<20)|(1<<24)|(1<<28);
    GPIOC->MODER&=~(0xF<<12|0xF<<16);
    GPIOC->PUPDR|=(1<<12)|(1<<16);
    GPIOA->MODER&=~(3<<16);
    GPIOA->PUPDR|=(1<<16);

    /* PA8 急停也走轮询, 不用EXTI (避免误触) */
    memset(ks,0,sizeof(ks));
}

void Key_Scan(void)
{
    for(int i=0;i<KEY_COUNT;i++){
        uint8_t raw=(kp[i].port->IDR & kp[i].pin)?1:0;
        if(raw==ks[i].raw){
            if(raw!=ks[i].stable){
                ks[i].stable=raw;
                if(!raw){ push(i,KEY_EVENT_DOWN); ks[i].ticks=0; ks[i].long_fired=0; ks[i].rep_tick=0; }
                else push(i,KEY_EVENT_UP);
            }else if(!raw){
                ks[i].ticks+=20;
                if(!ks[i].long_fired&&ks[i].ticks>=1000){ push(i,KEY_EVENT_LONG); ks[i].long_fired=1; ks[i].rep_tick=ks[i].ticks; }
                else if(ks[i].long_fired&&ks[i].ticks-ks[i].rep_tick>=200){ push(i,KEY_EVENT_REPEAT); ks[i].rep_tick=ks[i].ticks; }
            }
        }
        ks[i].raw=raw;
    }
}

/* 急停现在走轮询, 在 UI_ProcessKey 中处理 */
