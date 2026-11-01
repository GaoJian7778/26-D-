/**
 * protocol.c - 协议实现
 */
#include "protocol.h"
#include <string.h>

uint8_t proto_checksum(Frame *f) {
    uint8_t x=f->dir^f->cmd^f->len;
    for(uint8_t i=0;i<f->len;i++) x^=f->data[i];
    return x;
}

void proto_frame_to_bytes(Frame *f, uint8_t *out, uint8_t *len) {
    out[0]=0xAA; out[1]=0x55; out[2]=f->dir; out[3]=f->cmd; out[4]=f->len;
    for(uint8_t i=0;i<f->len;i++) out[5+i]=f->data[i];
    f->xor_chk=proto_checksum(f); out[5+f->len]=f->xor_chk; *len=6+f->len;
}

void proto_build_nofly(Frame *f, uint8_t *b, uint8_t n)
    { f->dir=DIR_TO_DRONE; f->cmd=CMD_NOFLY_MAP; f->len=n; memcpy(f->data,b,n); }

void proto_build_takeoff(Frame *f)
    { f->dir=DIR_TO_DRONE; f->cmd=CMD_TAKEOFF; f->len=0; }

void proto_build_emergency(Frame *f)
    { f->dir=DIR_TO_DRONE; f->cmd=CMD_EMERGENCY; f->len=0; }

void parser_init(FrameParser *p)
    { p->state=0; p->data_idx=0; p->calc_xor=0; memset(&p->frame,0,sizeof(Frame)); }

int parser_feed(FrameParser *p, uint8_t b) {
    switch(p->state){
    case 0: if(b==0xAA) p->state=1; break;
    case 1: if(b==0x55) p->state=2; else p->state=0; break;
    case 2: p->frame.dir=b; p->calc_xor=b; p->state=3; break;
    case 3: p->frame.cmd=b; p->calc_xor^=b; p->state=4; break;
    case 4: p->frame.len=b; p->calc_xor^=b;
            if(b>FRAME_MAX_DATA){p->state=0;return 0;}
            p->data_idx=0; p->state=b?5:6; break;
    case 5: p->frame.data[p->data_idx]=b; p->calc_xor^=b;
            if(++p->data_idx>=p->frame.len) p->state=6; break;
    case 6: p->frame.xor_chk=b; p->state=0; return(p->calc_xor==b);
    } return 0;
}

Frame parser_get_frame(FrameParser *p) { return p->frame; }
