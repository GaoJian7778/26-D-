/**
 * protocol.h - HC-04 蓝牙通信协议
 * 帧: AA 55 | dir(1B) | cmd(1B) | len(1B) | data(NB) | xor(1B)
 */
#ifndef __PROTOCOL_H
#define __PROTOCOL_H
#include "stm32f4xx.h"

#define DIR_TO_DRONE     0x00
#define DIR_FROM_DRONE   0x01
#define CMD_NOFLY_MAP    0x01
#define CMD_TAKEOFF      0x02
#define CMD_EMERGENCY    0x03
#define CMD_ANIMAL       0x11
#define CMD_STATUS       0x12
#define CMD_MISSION_DONE 0x13

/* D题: 三色目标 */
#define TARGET_RED       0x01
#define TARGET_GREEN     0x02
#define TARGET_BLUE      0x03

#define STATUS_GROUND    0x00
#define STATUS_TAKEOFF   0x01
#define STATUS_CRUISING  0x02
#define STATUS_LANDING   0x03
#define STATUS_LANDED    0x04
#define STATUS_QR_SCAN   0x05
#define STATUS_DROPPING  0x06
#define STATUS_GUIDING   0x07

#define FRAME_MAX_DATA   90  /* D题 80字节位图 */

typedef struct {
    uint8_t dir, cmd, len;
    uint8_t data[FRAME_MAX_DATA];
    uint8_t xor_chk;
} Frame;

typedef struct {
    uint8_t state, data_idx, calc_xor;
    Frame frame;
} FrameParser;

void  proto_build_nofly(Frame *f, uint8_t *bitmap, uint8_t len);
void  proto_build_takeoff(Frame *f);
void  proto_build_emergency(Frame *f);
void  parser_init(FrameParser *p);
int   parser_feed(FrameParser *p, uint8_t byte);
Frame parser_get_frame(FrameParser *p);
uint8_t proto_checksum(Frame *f);
void  proto_frame_to_bytes(Frame *f, uint8_t *out, uint8_t *out_len);
#endif
