#ifndef PLATFORM_H
#define	PLATFORM_H

#ifndef _SUPPRESS_PLIB_WARNING
#define _SUPPRESS_PLIB_WARNING
#endif

#ifdef	__cplusplus
extern "C" {
#endif

#include <xc.h>
#include <plib.h>

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <limits.h>
#include "../../xmem.h"

#include "disksd.h"
#include "../../fatfs/source/ff.h"
#include "../../fatfs/source/diskio.h"

#ifndef BIT
#define BIT(b) (1ull << (b))
#endif

#define _XTAL_FREQ      50000000ul  // oscillator frequency as defined in the configuration bits
#define _PB_FREQ        50000000ul  // peripheral bus frequency as defined in the configuration bits

#define IFS_DRV_KB      62          // IFS drive size in kB (this can be 0 if IFS drive is not required)
#define RAM_DRV_KB      0           // RAM drive size in kB (this can be 0 if RAM drive is not required)

#define SERIAL_BAUDRATE 38400       // serial console baudrate; protocol 8N1
#define SERIAL_TX       LATBbits.LATB7      // serial console Tx line (Rx is fixed at RB5)
#define SERIAL_TX_TRIS  TRISBbits.TRISB7

extern const unsigned char ifs_data[]; // the IFS: drive area


// SYSTEM DEFINITIONS ===========================================================================

void _general_exception_handler(void);
void reset(void);
void initPlatform(void);
void resetPlatform(void);
void empty(void);

unsigned char *SysMem;      // system memory
unsigned long SysMemSize;   // size in bytes of the allocated system memory

extern const void * const ELLO[];

// VIDEO GENERATOR AND HARDWARE-DEPENDENT BASIC VIDEO FUNCTIONS =================================

unsigned char *VidMem;      // video memory
unsigned long VidMemSize;   // size in bytes of the allocated video memory

extern unsigned int VideoParams[1][8];

uint32_t VpageAddr;     // VM memory address where the video page starts
uint16_t Hres, Vres;    // video resolution in pixels
uint8_t Vmode;          // video mode

void initVideo(uint8_t mode);
void clearScreen(int c);
void scrollUp(int vl, int c);
void setPixel(int x, int y, int c);
int getPixel(int x, int y);

void _mon_putc(char ch);
void _mon_puts(const char *s);


// PS/2 KEYBOARD ================================================================================

#define CON_BUFFER_SIZE     32  // size of the input console buffer
#define DEFAULT_KBD_FLAGS   0   // Scroll / Num / Caps flags

#ifndef MSK_WAIT_MS
#define MSK_WAIT_MS     	20  // give this many milliseconds time for the next key in escape sequences
#endif

volatile uint8_t kbdFlags;      // keyboard LED flags: [.0] ScrollLock; [.1] NumLock; [.2] CapsLock
volatile int16_t keyDown;       // -1 or key code of a pressed key
volatile uint32_t msKbdTimer;   // self-resettable millisecond countdown timer

void setKbdLEDs(uint8_t flags);
void initKeyboard(void);

int _mon_getc(int blocking);
int kbhit(void);
char getch(void);


// USB ==========================================================================================

#ifdef MULTI_CLASS_DEVICE
uint8_t cdc_interfaces[];
uint8_t msc_interfaces[];
#endif

void doUSB(void);


// TIMING =======================================================================================

volatile unsigned long ms_timer;    /* internal countdown timer used by mSec() */
volatile unsigned long ms_clock;    /* counter of the milliseconds since the last system initialisation */
volatile unsigned long ss_time;     /* counter of the seconds since Jan 1, 1970, 00:00:00.000 */

// delays for microseconds and milliseconds
void uSec(unsigned long us);
void mSec(unsigned long ms);

clock_t clock(void);
time_t time(time_t *t);


// SYSTEM SPI AND FILE SYSTEM ===================================================================

#define SYSTEM_SPI      SPI_CHANNEL1    /* openSPI() assumes only SPI1 in ELLO 1A */

#define SD1_nCS_BIT     BIT(1)          /* RA1 is the CS# of SD1 card */
#define SD2_nCS_BIT     BIT(0)          /* RA0 is used as CS# for an externally connected SD2 card */
#define SD_nCS_LAT      LATA
#define SD_nCS_TRIS     TRISA

#define SD_CARD_CLK     10000000ul      /* desired SD card SPI clock in Hz */

int openSPI(char channel, unsigned char spi_mode, unsigned char bits, unsigned long speed);
unsigned long xchgSPI(char channel, unsigned long tx_data);

FATFS FatFs;    /* work area for FatFs */

// USB ==========================================================================================

void checkUSB(void);


// SPECIAL VERSION OF NVMProgram() ADAPTED FOR PIC32MX1xx/2xx ===================================

#ifdef PAGE_SIZE
#undef PAGE_SIZE
#undef BYTE_PAGE_SIZE
#undef ROW_SIZE
#undef BYTE_ROW_SIZE
#undef NUM_ROWS_PAGE
#endif

#define PAGE_SIZE       256             // # of 32-bit Instructions per Page
#define BYTE_PAGE_SIZE  (4 * PAGE_SIZE) // Page size in Bytes
#define ROW_SIZE        32              // # of 32-bit Instructions per Row
#define BYTE_ROW_SIZE   (4 * ROW_SIZE)  // # Row size in Bytes
#define NUM_ROWS_PAGE   8               // Number of Rows per Page

unsigned int NVMProgramMX1(unsigned char *address, unsigned char *data, unsigned int size, unsigned char *pagebuff);


// SOUND ========================================================================================

void sound(int freq, int vol);      /* frequency in Hz, sound volume between 0 and 1000 */
void beep(void);                    /* default system beep */


// COM ==========================================================================================

#define COM_PORTS       1           /* total number of supported COM ports */

int com_buff_size[COM_PORTS];
volatile int com_rx_in[COM_PORTS];
int com_rx_out[COM_PORTS];
byte *com_buff[COM_PORTS];

/* open UART port */
/* opening with buffer size 0 will close the port */
/* will return 0 if it has been successfully executed; 1 otherwise */
int openCOM(unsigned char port, unsigned short buff_size, unsigned long bps,
                UART_LINE_CONTROL_MODE f, UART_CONFIGURATION c);

void UART_tx(unsigned char port, char data);
int UART_rx(unsigned char port);
int UART_peek(unsigned char port);      /* same as UART_rx() but does not remove the character from the buffer */
int UART_buffer(unsigned char port);    /* return the number of bytes in a UART buffer */


// I2C ==========================================================================================

#define I2C_TRIS        TRISB
#define I2C_PORT        PORTB
#define I2C_LAT         LATB
#define I2C_PULLUP      CNPUB

#define SCL             (1 << 8)
#define SDA             (1 << 9)

#define I2C_TIMEOUT     200         /* I2C timeout in multiples of 5usec */
#define I2C_BIT_US      10          /* I2C bit length in microseconds (10us = 100Kbps) */

int i2cInit(int baudrate);
void i2cStart(void);
void i2cRepStart(void);
void i2cStop(void);
int i2cSend(unsigned char data8);
unsigned char i2cRecv(int ack);


// RTC ==========================================================================================

#define I2C_DS3231      0xD0

int i2cGetTime(struct tm *t);
int i2cSetTime(struct tm *t);


// I/O ports ====================================================================================

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
#define BASEA  ((volatile unsigned int *) 0xBF886020)
#define BASEB  ((volatile unsigned int *) 0xBF886120)
#define BASEC  ((volatile unsigned int *) 0xBF886220)
#define BASED  ((volatile unsigned int *) 0xBF886220)
#define BASEE  ((volatile unsigned int *) 0xBF886220)
#define BASEF  ((volatile unsigned int *) 0xBF886220)
#define BASEG  ((volatile unsigned int *) 0xBF886220)

extern const volatile unsigned int *pbase[];

#ifdef	__cplusplus
}
#endif

#endif
