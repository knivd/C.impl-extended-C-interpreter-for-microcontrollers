#ifndef PLATFORM_H
#define PLATFORM_H

/*

LOW-LEVEL AND HARDWARE-DEPENDENT FUNCTIONS

platform: PIC32MZ2048EFH064 (TQFP64 and QFN64)

pin  1:	DO/DI/AI
pin  2:	DO/DI/AI			SD Card SEL2#
pin  3: DO/DI/AI			SD Card SEL1#
pin  4: DO/DI/AI			SPI2 SCLK
pin  5: DO/DI/AI			SPI2 MISO
pin  6: DO/DI/AI			SPI2 MOSI
pin  7: GND
pin  8: Vdd (+3.3V)
pin  9: [5V] MCLR#
pin 10:	DO/DI/AI
pin 11:	DO/DI/AI            COM1 Rx
pin 12:	DO/DI/AI
pin 13:	DO/DI/AI            COM1 Tx
pin 14:	DO/DI/AI            COM4 Rx
pin 15:	DO/DI/AI
pin 16:	DO/DI/AI            COM4 Tx
pin 17: PGEC/DO/DI/AI		COM6 Rx (default console + PGC)
pin 18: PGED/DO/DI/AI		COM6 Tx (default console + PGD)
pin 19: AVdd (Fltr +3.3V)
pin 20: AGND (Fltr GND)
pin 21:	DO/DI/AI
pin 22:	DO/DI/AI			SPI3 MISO
pin 23:	DO/DI/AI			SPI3 MOSI
pin 24:	DO/DI/AI
pin 25: GND
pin 26: Vdd (+3.3V)
pin 27:	DO/DI/AI
pin 28:	DO/DI/AI
pin 29:	DO/DI/AI			SPI3 SCLK
pin 30:	DO/DI/AI
pin 31: 24MHz CLK Input
pin 32:	CLK Sleep Control
pin 33: [5V] USB VBUS Input
pin 34: Vusb (+3.3V)
pin 35: GND
pin 36: USB Console D-
pin 37: USB Console D+
pin 38:	DO/DI
pin 39: Vdd (+3.3V)
pin 40: GND
pin 41:	[5V] DO/DI			COM5 Rx / IIC2 SDA
pin 42:	[5V] DO/DI			COM5 Tx / IIC2 SCL
pin 43:	[5V] DO/DI			IIC1 SDA
pin 44:	[5V] DO/DI			IIC1 SCL
pin 45:	[5V] DO/DI
pin 46:	[5V] DO/DI
pin 47:	[5V] DO/DI
pin 48:	[5V] DO/DI			32.768kHz Input
pin 49:	[5V] DO/DI			SPI1 SCLK
pin 50:	[5V] DO/DI			SPI1 MISO
pin 51:	[5V] DO/DI			SPI1 MOSI
pin 52:	[5V] DO/DI          COM2 Rx
pin 53:	[5V] DO/DI          COM2 Tx
pin 54: Vdd (+3.3V)
pin 55: GND
pin 56:	[5V] DO/DI          COM3 Rx
pin 57:	[5V] DO/DI          COM3 Tx
pin 58:	[5V] DO/DI
pin 59: GND
pin 60: Vdd (+3.3V)
pin 61:	[5V] DO/DI
pin 62:	[5V] DO/DI
pin 63:	[5V] DO/DI
pin 64:	DO/DI/AI

*/

#define IFS_DRV_KB 1632     /* IFS drive size in kB (this can be 0 if IFS drive is not required) */
#define RAM_DRV_KB 160      /* RAM drive size in kB (this can be 0 if RAM drive is not required) */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _SUPPRESS_PLIB_WARNING
#define _SUPPRESS_PLIB_WARNING
#endif

#include <xc.h>
#include "plibs/plib.h"

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <limits.h>
#include "../../xmem.h"

#include "disksd.h"
#include "../../fatfs/source/ff.h"
#include "../../fatfs/source/diskio.h"
#include "../../ride/ride.h"

/* these definitions are now placed in ffconf.h */

/* Definitions of physical drive number for each drive */
/* #define DEV_NUL     0   */ /* NULL device */
/* #define DEV_IFS     1   */ /* internal flash disk */
/* #define DEV_RAM     2   */ /* small internal RAM disk for data exchange between files */
/* #define DEV_SD1     3   */ /* external SD card 1 */
/* #define DEV_SD2     4   */ /* external SD card 2 */

#define POSCCLK 24000000ul  /* external primary oscillator frequency (Hz) */
#define SOSCCLK 32768ul     /* external secondary oscillator frequency (Hz) */

/* IMPORTANT: */
/*   #define DISABLE_DEBUG  in the project settings will disable the PIC32 debugging mode */

#ifndef BIT
#define BIT(b) (1ull << (b))
#endif

#define SD1_nCS_BIT     BIT(7)      /* RE7 is the CS# of SD1 on the Rittle board */
#define SD2_nCS_BIT     BIT(6)      /* RE6 is the CS# of SD1 on the Rittle board */
#define SD_nCS_LAT      LATE
#define SD_nCS_TRIS     TRISE

#define SD_CARD_CLK     10000000ul  /* desired SD card SPI clock in Hz */

/*
default clocks
IMPORTANT: PBCLK2 and PBCLK3 must be kept 64 MHz at all CPU frequencies
		   PBCLK1, PBCLK4, PBCLK5 must be as high as possible up to 100 MHz
		   PBCLK7 depends on the CPU frequency selection
		   PBCLK8 is never used and remains disabled at all times
*/
#define SYSCLK	192000000ul     /* (max 252) default main system clock */
#define PBCLK1	(SYSCLK / 2)	/* (max 100) peripheral clock for WDT, DeadmanT, Flash */
#define PBCLK2	64000000ul      /* (max 100) peripheral clock for PMP, I2C, UART, SPI */
#define PBCLK3	64000000ul      /* (max 100) peripheral clock for ADC, Comp, Timers, OutCapt, InCapt */
#define PBCLK4	(SYSCLK / 2)	/* (max 200) peripheral clock for Ports */
#define PBCLK5	(SYSCLK / 2)	/* (max 100) peripheral clock for Crypto RNG, USB, CAN, Eth, SQI */
#define PBCLK7	(SYSCLK / 1)	/* (max 252) peripheral clock for CPU, DeadmanT */
#define PBCLK8	(SYSCLK / 2)	/* (max 100) peripheral clock for EBI (NOT USED) */

#define COM_PORTS       6		/* total number of supported COM ports */
#define SPI_PORTS       3		/* total number of SPI ports */
#define IIC_PORTS       1		/* total number of I2C ports */

#define COM_RX_SIZE     256     /* standard buffer size for COM ports */

#define CONSOLE_COM     UART4   /* COM port linked to the serial console (set -1 if none) */
#define CONSOLE_BAUD    115200  /* bardrate for the serial console (protocol fixed at 8N1) */
#define CONSOLE_ECHO    0       /* value above 0 will enable console echo */

#define SYSTEM_SPI      SPI_CHANNEL2    /* SPI channel used for system needs (such as SD card comms, etc.); set -1 if none */

/* hardware platform flags */
#define HWP_ECLK_PRESENT    BIT(0)  /* 24 MHz external clock is present and can be used */
#define HWP_ECLK_WARNED     BIT(1)	/* warning about missing clock has been shown already */

extern unsigned char hwp_flags;     /* hardware flags */
extern unsigned long sys_freq_khz;  /* current system clock frequency in kHz */

int com_buff_size[COM_PORTS];
volatile int com_rx_in[COM_PORTS];
int com_rx_out[COM_PORTS];
byte *com_buff[COM_PORTS];

FATFS FatFs;    /* work area for FatFs */

extern const unsigned char ifs_data[];
extern volatile unsigned long ms_clock; /* counter of the milliseconds since the last system initialisation */
extern volatile unsigned long ss_time;  /* counter of the seconds since Jan 1, 1970, 00:00:00.000 */
extern volatile unsigned long ms_timer; /* countdown timer for delays */

#ifdef BOOTLOADER
/* enter bootloader */
void bootloader(void);
#endif

/* set system clock frequency in kHz and return 0 if successful or 1 in case of invalid parameter */
int set_sysFreq(unsigned long freq_khz);

/* open SPI port */
/* opening with speed 0 will close the port */
/* bits can 8, 16, or 32 bits in a SPI word */
/* will return 0 if it has been successfully executed; 1 otherwise */
int openSPI(unsigned char channel, unsigned char spi_mode, unsigned char bits, unsigned long speed);

// transmit/receive data to/from SPI port
unsigned long xchgSPI(unsigned char channel, unsigned long tx_data);

/* open UART port */
/* opening with buffer size 0 will close the port */
/* will return 0 if it has been successfully executed; 1 otherwise */
int openCOM(unsigned char port, unsigned short buff_size, unsigned long bps,
                UART_LINE_CONTROL_MODE f, UART_CONFIGURATION c);

/* open (with parameter >0) or close (with parameter 0) the USB console */
void openUSB(int state);

/* hardware exception handler function */
void _general_exception_handler(void);

// delays for microseconds and milliseconds
void uSec(unsigned long us);
void mSec(unsigned long ms);

/* offsets from the base address for the port-associated registers */
#define ANSEL      -8
#define ANSELCLR   -7
#define ANSELSET   -6
#define ANSELINV   -5
#define TRIS       -4
#define TRISCLR    -3
#define TRISSET    -2
#define TRISINV    -1
#define PORT        0
#define PORTCLR     1
#define PORTSET     2
#define PORTINV     3
#define LAT         4
#define LATCLR      5
#define LATSET      6
#define LATINV      7
#define ODC         8
#define ODCCLR      9
#define ODCSET      10
#define ODCINV      11
#define CNPU        12
#define CNPUCLR     13
#define CNPUSET     14
#define CNPUINV     15
#define CNPD        16
#define CNPDCLR     17
#define CNPDSET     18
#define CNPDINV     19
#define CNCON       20
#define CNCONCLR    21
#define CNCONSET    22
#define CNCONINV    23
#define CNEN        24
#define CNENCLR     25
#define CNENSET     26
#define CNENINV     27
#define CNSTAT      28
#define CNSTATCLR   29
#define CNSTATSET   30
#define CNSTATINV   31

/* base address of the hardware ports */
#define BASEA  ((volatile unsigned int *) 0xBF860020)
#define BASEB  ((volatile unsigned int *) 0xBF860120)
#define BASEC  ((volatile unsigned int *) 0xBF860220)
#define BASED  ((volatile unsigned int *) 0xBF860220)
#define BASEE  ((volatile unsigned int *) 0xBF860220)
#define BASEF  ((volatile unsigned int *) 0xBF860220)
#define BASEG  ((volatile unsigned int *) 0xBF860220)

// I2C ==========================================================================================

#define I2C_TRIS        TRISD
#define I2C_PORT        PORTD
#define I2C_LAT         LATD
#define I2C_PULLUP      CNPUD

#define SCL             (1 << 10)
#define SDA             (1 << 9)

#define I2C_TIMEOUT     200         /* I2C timeout in multiples of 5usec */
#define I2C_BIT_US      10          /* I2C bit length in microseconds (10us = 100Kbps) */

int i2cInit(int baudrate);
void i2cStart(void);
void i2cRepStart(void);
void i2cStop(void);
int i2cSend(unsigned char data8);
unsigned char i2cRecv(int ack);

// ===================================
// required basic I/O helper functions
// ===================================

clock_t clock(void);
time_t time(time_t *tm);
int kbhit(void);
void UART_tx(unsigned char port, char data);
int UART_rx(unsigned char port);
int UART_peek(unsigned char port);      /* same as UART_rx() but does not remove the character from the buffer */
int UART_buffer(unsigned char port);    /* return the number of bytes in a UART buffer */
int _mon_getc(int blocking);
void _mon_putc(char ch);
void _mon_puts(const char *s);
void _mon_write (const char *s, unsigned int count);

// ===========================
// required platform functions
// ===========================

/* always required platform definitions (even if the system has no capability to handle certain functionalities) */

FATFS FatFs;    /* work area for FatFs */

/* platform initialisation */
void initPlatform(void);

/* warm initialisation of the hardware platform */
void resetPlatform(void);

/* delay in milliseconds */
void mSec(clock_t ms);

/* system beep */
void beep(void);

extern unsigned char *SysMem;       // system memory
extern unsigned long SysMemSize;    // size in bytes of the allocated system memory

extern unsigned char *VidMem;       // video memory
extern unsigned long VidMemSize;    // size in bytes of the allocated video memory

int8_t kbdLayout;
uint16_t Hres, Vres;
uint8_t Vmode;

void initVideo(uint8_t mode);
void clearScreen(int c);
void scrollUp(int vl, int c);
void setPixel(int x, int y, int c);
int getPixel(int x, int y);

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_H */
