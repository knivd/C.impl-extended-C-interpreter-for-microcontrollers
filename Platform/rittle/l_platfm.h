/*

Platform-depended library for the C.impl interpreter

platform: ELLO 1A
(C) KnivD, 2020-2021

*/

#ifdef CIMPL
#ifndef LIB_PLATFORM_H
#define LIB_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "platform.h"
#include "../../cimpl/cimpl.h"

extern const sys_const_t platform_const_table[];
extern const sys_func_t platform_func_table[];

#define platform_src \
    "const char const *platform = \"RITTLE\";\r\n" \
    ETXSTR

void pltfm_init(void);
void pltfm_call(void);

void sf_reset(void);        /* void reset(void) */
void sf_delay_ms(void);     /* void delay_ms(unsigned long milliseconds) */
void sf_set_timer(void);    /* void set_timer(unsigned long milliseconds, void (*intHandler)(void)) */

void sf_spiOpen(void);      /* int spiOpen(int channel, int mode, int bits, int baudrate) */
void sf_spiClose(void);     /* int spiClose(int channel) */
void sf_spiByte(void);      /* unsigned char spiByte(int channel, unsigned char data) */
void sf_spiBlock(void);     /* void spiBlock(int channel, unsigned char *buffer, size_t len) */

void sf_comOpen(void);      /* int comOpen(int channel, int mode, int protocol, int baudrate, int rx_buffer_size) */
void sf_comClose(void);     /* int comClose(int channel) */
void sf_comPeek(void);      /* int comPeek(int channel) */
                            /* return -1 if there is nothing in the reception buffer, or the first received byte without removing it */
void sf_comBuff(void);      /* int comBuff(int channel) */
                            /* return the number of bytes in the reception buffer for a channel */
void sf_comTx(void);        /* void comTx(int channel, int bytes, unsigned char *buffer) */
void sf_comRx(void);        /* int comRx(int channel, int bytes, unsigned char *buffer) */
                            /* return the number of actually received bytes */
void sf_comRxCall(void);    /* void comRxCall(int channel, int bytes, void (*intHandler)(void)) */
                            /* call the function every time when there are the specified number or more bytes in the channel reception buffer */

void sf_i2cInit(void);      /* void i2cInit(int baudrate) */
void sf_i2cStart(void);     /* void i2cStart(void) */
void sf_i2cRepStart(void);  /* void i2cRepStart(void) */
void sf_i2cStop(void);      /* void i2cStop(void) */
void sf_i2cSend(void);      /* int i2cSend(unsigned char data8) */
void sf_i2cRecv(void);      /* unsigned char i2cRecv(int ack) */

void sf_setKbdLayout(void); /* void setKbdLayout(char country) */
void sf_getKbdLayout(void); /* char getKbdLayout(void) */
void sf_setBrkCode(void);   /* void setBrkCode(char code) */
void sf_getBrkCode(void);   /* char getBrkCode(void) */

void sf_beep(void);         /* void beep(void) */
void sf_sound(void);        /* void sound(int freq, int vol); sound volume is between 0 and 1000 */

void sf_initVideo(void);    /* void initVideo(int mode) */
void sf_getVmode(void);     /* int getVmode(void) */
void sf_Hres(void);         /* int Hres(void) */
void sf_Vres(void);         /* int Vres(void) */
void sf_clearScreen(void);  /* void clearScreen(int colour) */
void sf_setPixel(void);     /* void setPixel(int x, int y, int c) */
void sf_getPixel(void);     /* int getPixel(int x, int y) */

void sf_setSysFreq(void);   /* int setSysFreq(unsigned lonf khz) */
void sf_getSysFreq(void);   /* int getSysFreq(void) */

#ifdef __cplusplus
}
#endif

#endif  /* LIB_PLATFORM_H */
#endif
