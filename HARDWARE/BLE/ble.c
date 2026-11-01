/**
 * ble.c - HC-04 蓝牙驱动
 * USART1: PA9=TX(蓝牙RX) PA10=RX(蓝牙TX)  9600
 */
#include "ble.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>

static RingBuf rx_ring;
static FrameParser rx_parser;
static uint8_t dma_rx_buf[BLE_RX_BUF_SIZE];
volatile uint8_t emergency_flag = 0;

void BLE_Init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;

    /* PA9(TX) PA10(RX) → AF7 */
    GPIOA->MODER &= ~(0xF<<18|0xF<<20);
    GPIOA->MODER |= (2<<18)|(2<<20);
    GPIOA->AFR[1] |= (7<<4)|(7<<8);

    /* USART1: 实际波特率 ~28800 发给 HC-04 9600 → 降3倍 */
    USART1->BRR = (SystemCoreClock/2)/(9600UL*3);
    USART1->CR1 = USART_CR1_UE | USART_CR1_RE | USART_CR1_TE;
    USART1->CR3 = USART_CR3_DMAR;

    /* DMA2 Stream2 Ch4: USART1 RX */
    DMA2_Stream2->CR = 0;
    DMA2_Stream2->PAR = (uint32_t)&USART1->DR;
    DMA2_Stream2->M0AR = (uint32_t)dma_rx_buf;
    DMA2_Stream2->NDTR = BLE_RX_BUF_SIZE;
    DMA2_Stream2->CR = (4<<25)|(1<<16)|(1<<10)|(1<<8)|DMA_SxCR_CIRC|DMA_SxCR_EN;

    /* USART1 空闲中断 */
    USART1->CR1 |= USART_CR1_IDLEIE;
    NVIC_EnableIRQ(USART1_IRQn);
    NVIC_SetPriority(USART1_IRQn, 5);

    memset(&rx_ring, 0, sizeof(rx_ring));
    parser_init(&rx_parser);
}

void BLE_Send(uint8_t *data, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++) {
        while (!(USART1->SR & USART_SR_TXE));
        USART1->DR = data[i];
    }
}

void BLE_SendFrame(Frame *f)
{
    uint8_t buf[70], len;
    proto_frame_to_bytes(f, buf, &len);
    BLE_Send(buf, len);
}

int BLE_RecvByte(void)
{
    if (rx_ring.tail == rx_ring.head) return -1;
    uint8_t b = rx_ring.buffer[rx_ring.tail];
    rx_ring.tail = (rx_ring.tail + 1) % BLE_RX_BUF_SIZE;
    return b;
}

static uint32_t ble_last_rx_tick = 0;

uint8_t BLE_IsConnected(void)
{
    return (xTaskGetTickCount() - ble_last_rx_tick) < 3000;
}

int BLE_ParseFrame(Frame *f)
{
    int b;
    while ((b = BLE_RecvByte()) >= 0)
        if (parser_feed(&rx_parser, (uint8_t)b)) {
            ble_last_rx_tick = xTaskGetTickCount();
            *f = parser_get_frame(&rx_parser);
            return 1;
        }
    return 0;
}

static uint16_t dma_last = 0;

void USART1_IRQHandler(void)
{
    if (USART1->SR & USART_SR_IDLE) {
        (void)USART1->DR;
        uint16_t pos = BLE_RX_BUF_SIZE - DMA2_Stream2->NDTR;
        if (pos != dma_last) {
            for (uint16_t i = dma_last; i != pos; i = (i + 1) % BLE_RX_BUF_SIZE) {
                uint16_t next = (rx_ring.head + 1) % BLE_RX_BUF_SIZE;
                if (next != rx_ring.tail) {
                    rx_ring.buffer[rx_ring.head] = dma_rx_buf[i];
                    rx_ring.head = next;
                }
            }
            dma_last = pos;
        }
    }
}
