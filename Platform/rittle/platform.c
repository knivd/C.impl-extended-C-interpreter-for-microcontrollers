#include <stdio.h>
#include <limits.h>
#include "fuses.h"
#include "platform.h"
#include "uconsole.h"

/*
macro to reserve flash memory for saving/loading data and initialise to 0xFF's
note that (bytes) need to be a multiple of:
- BYTE_PAGE_SIZE (and aligned that way) if you intend to erase
- BYTE_ROW_SIZE (and aligned that way) if you intend to write rows
- sizeof(int) if you intend to write words
*/
#define ALLOCATE(name,align,bytes) const unsigned char name[(bytes)] __attribute__ ((aligned(align),space(prog),section(".ifs"))) = {[0 ...(bytes)-1]=0xff}
ALLOCATE(ifs_data, BYTE_PAGE_SIZE, (IFS_DRV_KB * 1024));

volatile unsigned long ms_timer = 0;    /* countdown timer for delays */
volatile unsigned long ms_clock = 0;    /* counter of the milliseconds since the last system initialisation */
volatile unsigned long ss_time = 0;     /* counter of the seconds since Jan 1, 1970, 00:00:00.000 */

unsigned char hwp_flags = 0;            /* hardware flags */
unsigned long sys_freq_khz = 0;         /* current system clock frequency in kHz */

unsigned char *SysMem = NULL;   // system memory
unsigned long SysMemSize = 0;   // size in bytes of the allocated system memory

unsigned char *VidMem = NULL;   // video memory
unsigned long VidMemSize = 0;   // size in bytes of the allocated video memory

#ifdef BOOTLOADER

/* user-defined key address in RAM Memory - the last 4 bytes used for boot key sequence */
#define BOOT_KEY    0x12345678

const extern unsigned int _BOOT_KEY_ADDRESS;
static unsigned int BOOT_KEY_ADDRESS;

void bootloader(void) {
    BOOT_KEY_ADDRESS = (unsigned int) &_BOOT_KEY_ADDRESS;
    int t;
    for(t = 0; t < 100; t++) printf("\n");
    printf("\r\n\n\n\n\n>>> Entering bootloader mode...\r\n");
    openCOM(CONSOLE_COM, 0, 0, 0, 0);
	openUSB(0);
    INTDisableInterrupts();
    *((int *) BOOT_KEY_ADDRESS) = BOOT_KEY;
    resetPlatform();
}

#endif


void uSec(unsigned long us) {
    unsigned long i = ((((unsigned long) (us) * 1000) - 600) / (2000000000 / PBCLK7));
    WriteCoreTimer(0);
    while(ReadCoreTimer() < i);
}


void mSec(unsigned long ms) {
    ms_timer = ms;
    while(ms_timer);
}


// transmit/receive data to/from SPI port
unsigned long xchgSPI(unsigned char channel, unsigned long tx_data) {
    unsigned long rx_data = 0;
    if(channel >= 0) {
        SpiChnGetRov((SpiChannel) channel, TRUE);
        SpiChnPutC((SpiChannel) channel, tx_data);  /* transmit data */
        rx_data = SpiChnGetC((SpiChannel) channel); /* receive data */
    }
	return rx_data;
}


/* open SPI_CHANNELx port */
/* opening with speed 0 will close the port */
/* bits can 8, 16, or 32 bits in a SPI word */
/* will return 0 if it has been successfully executed; 1 otherwise */
int openSPI(unsigned char channel, unsigned char spi_mode, unsigned char bits, unsigned long speed) {
    switch(channel) {
        default: return 2;  /* invalid channel */

        case SPI_CHANNEL1:
            SpiChnEnable((SpiChannel) channel, FALSE);
            PORTResetPins(IOPORT_D, (BIT(1) | BIT(2) | BIT(3)));
            PPSUnLock;
            PPSOutput(2, RPD3, NULL);
            if(speed) {
                PORTSetPinsDigitalOut(IOPORT_D, (BIT_1 | BIT_3));   /* RD1 is SCLK1 and RD3 is MOSI1 */
                PORTSetPinsDigitalIn(IOPORT_D, BIT_2);              /* RD2 is MISO1 */
                PPSOutput(2, RPD3, SDO1);
                PPSInput(1, SDI1, RPD2);
            }
            PPSLock;
            break;

        case SPI_CHANNEL2:
            SpiChnEnable((SpiChannel) channel, FALSE);
            PORTResetPins(IOPORT_G, (BIT(6) | BIT(7) | BIT(8)));
            PPSUnLock;
            PPSOutput(1, RPG8, NULL);
            if(speed) {
                PORTSetPinsDigitalOut(IOPORT_G, (BIT_6 | BIT_8));   /* RG6 is SCLK2 and RG8 is MOSI2 */
                PORTSetPinsDigitalIn(IOPORT_G, BIT_7);              /* RG7 is MISO2 */
                PPSOutput(1, RPG8, SDO2);
                PPSInput(2, SDI2, RPG7);
            }
            PPSLock;
            break;

        case SPI_CHANNEL3:
            SpiChnEnable((SpiChannel) channel, FALSE);
            PORTResetPins(IOPORT_B, (BIT(9) | BIT(10) | BIT(14)));
            PPSUnLock;
            PPSOutput(1, RPB10, NULL);
            if(speed) {
                PORTSetPinsDigitalOut(IOPORT_B, (BIT_10 | BIT_14)); /* RB14 is SCLK3 and RB10 is MOSI3 */
                PORTSetPinsDigitalIn(IOPORT_B, BIT_9);              /* RB9 is MISO3 */
                PPSOutput(1, RPB10, SDO3);
                PPSInput(1, SDI3, RPB9);
            }
            PPSLock;
            break;
    }
    if(speed) {
        SpiOpenFlags f = (SPI_OPEN_ON | SPI_OPEN_MSTEN);
        if(bits == 32) f |= SPI_OPEN_MODE32;
        else if(bits == 16) f |= SPI_OPEN_MODE16;
        else f |= SPI_OPEN_MODE8;   /* 8 bits data word length by default */
        spi_mode &= 3;  /* only four SPI modes are possible */
        if(spi_mode == 0) f |= SPI_OPEN_CKE_REV;
        else if(spi_mode == 3) f |= SPI_OPEN_CKP_HIGH;
        else if(spi_mode == 2) f |= (SPI_OPEN_CKP_HIGH | SPI_OPEN_CKE_REV);
        SpiChnOpen((SpiChannel) channel, f, (PBCLK2 / speed));
        SpiChnEnable((SpiChannel) channel, TRUE);
    }
    return 0;
}


/* open UARTx port */
/* opening with buffer size 0 will close the port */
/* will return 0 if it has been successfully executed; 1 otherwise */
int openCOM(unsigned char port, unsigned short buff_size, unsigned long bps,
            UART_LINE_CONTROL_MODE f, UART_CONFIGURATION c) {
    switch(port) {
        default: return 2;  /* invalid port number */

        case UART1:
            INTEnable(INT_SOURCE_USART_1_RECEIVE, INT_DISABLED);
            UARTEnable(UART1, 0);
            PORTResetPins(IOPORT_B, (BIT(3) | BIT(5)));
            PPSUnLock;
            PPSOutput(2, RPB3, NULL);
            if(buff_size && bps) {
                PORTSetPinsDigitalOut(IOPORT_B, BIT_3); /* RB3 is TX for COM1 */
                PORTSetPinsDigitalIn(IOPORT_B, BIT_5);  /* RB5 is RX for COM1 */
                CNPUBSET = (BIT_3 | BIT_5); /* set pull-ups */
                UARTConfigure(UART1, c);
                UARTSetLineControl(UART1, f);
                UARTSetDataRate(UART1, PBCLK2, bps);
                UARTSetFifoMode(UART1, UART_INTERRUPT_ON_RX_NOT_EMPTY);
                INTSetVectorPriority(INT_SOURCE_USART_1_RECEIVE, INT_PRIORITY_LEVEL_3);
                INTClearFlag(INT_SOURCE_USART_1_RECEIVE);
                INTEnable(INT_SOURCE_USART_1_RECEIVE, INT_ENABLED);
                UARTEnable(UART1, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_TX | UART_RX));
				PPSInput(1, U1RX, RPB5);
                PPSOutput(2, RPB3, U1TX);
            }
            PPSLock;
            break;

        case UART2:
            INTEnable(INT_SOURCE_USART_2_RECEIVE, INT_DISABLED);
            UARTEnable(UART2, 0);
            PORTResetPins(IOPORT_D, (BIT(4) | BIT(5)));
            PPSUnLock;
            PPSOutput(4, RPD5, NULL);
            if(buff_size && bps) {
                PORTSetPinsDigitalOut(IOPORT_D, BIT_5); /* RD5 is TX for COM2 */
                PORTSetPinsDigitalIn(IOPORT_D, BIT_4);  /* RD4 is RX for COM2 */
                CNPUDSET = (BIT_4 | BIT_5); /* set pull-ups */
                UARTConfigure(UART2, c);
                UARTSetLineControl(UART2, f);
                UARTSetDataRate(UART2, PBCLK2, bps);
                UARTSetFifoMode(UART2, UART_INTERRUPT_ON_RX_NOT_EMPTY);
                INTSetVectorPriority(INT_SOURCE_USART_2_RECEIVE, INT_PRIORITY_LEVEL_3);
                INTClearFlag(INT_SOURCE_USART_2_RECEIVE);
                INTEnable(INT_SOURCE_USART_2_RECEIVE, INT_ENABLED);
                UARTEnable(UART2, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_TX | UART_RX));
				PPSInput(3, U2RX, RPD4);
                PPSOutput(4, RPD5, U2TX);
            }
            PPSLock;
            break;

        case UART3:
            INTEnable(INT_SOURCE_USART_3_RECEIVE, INT_DISABLED);
            UARTEnable(UART3, 0);
            PORTResetPins(IOPORT_F, (BIT(0) | BIT(1)));
            PPSUnLock;
            PPSOutput(1, RPF1, NULL);
            if(buff_size && bps) {
                PORTSetPinsDigitalOut(IOPORT_F, BIT_1); /* RF1 is TX for COM3 */
                PORTSetPinsDigitalIn(IOPORT_F, BIT_0);  /* RF0 is RX for COM3 */
                CNPUFSET = (BIT_0 | BIT_1); /* set pull-ups */
                UARTConfigure(UART3, c);
                UARTSetLineControl(UART3, f);
                UARTSetDataRate(UART3, PBCLK2, bps);
                UARTSetFifoMode(UART3, UART_INTERRUPT_ON_RX_NOT_EMPTY);
                INTSetVectorPriority(INT_SOURCE_USART_3_RECEIVE, INT_PRIORITY_LEVEL_3);
                INTClearFlag(INT_SOURCE_USART_3_RECEIVE);
                INTEnable(INT_SOURCE_USART_3_RECEIVE, INT_ENABLED);
                UARTEnable(UART3, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_TX | UART_RX));
				PPSInput(2, U3RX, RPF0);
                PPSOutput(1, RPF1, U3TX);
            }
            PPSLock;
            break;

        case UART4:
            INTEnable(INT_SOURCE_USART_4_RECEIVE, INT_DISABLED);
            UARTEnable(UART4, 0);
            PORTResetPins(IOPORT_B, (BIT(0) | BIT(2)));
            PPSUnLock;
            PPSOutput(3, RPB0, NULL);
            if(buff_size && bps) {
                PORTSetPinsDigitalOut(IOPORT_B, BIT_0); /* RB0 is TX for COM4 */
                PORTSetPinsDigitalIn(IOPORT_B, BIT_2);  /* RB2 is RX for COM4 */
                CNPUBSET = (BIT_0 | BIT_2); /* set pull-ups */
                UARTConfigure(UART4, c);
                UARTSetLineControl(UART4, f);
                UARTSetDataRate(UART4, PBCLK2, bps);
                UARTSetFifoMode(UART4, UART_INTERRUPT_ON_RX_NOT_EMPTY);
                INTSetVectorPriority(INT_SOURCE_USART_4_RECEIVE, INT_PRIORITY_LEVEL_3);
                INTClearFlag(INT_SOURCE_USART_4_RECEIVE);
                INTEnable(INT_SOURCE_USART_4_RECEIVE, INT_ENABLED);
                UARTEnable(UART4, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_TX | UART_RX));
				PPSInput(4, U4RX, RPB2);
                PPSOutput(3, RPB0, U4TX);
            }
            PPSLock;
            break;

        case UART5:
            INTEnable(INT_SOURCE_USART_5_RECEIVE, INT_DISABLED);
            UARTEnable(UART5, 0);
            PORTResetPins(IOPORT_F, (BIT(4) | BIT(5)));
            PPSUnLock;
            PPSOutput(2, RPF5, NULL);
            if(buff_size && bps) {
                PORTSetPinsDigitalOut(IOPORT_F, BIT_5); /* RF5 is TX for COM5 */
                PORTSetPinsDigitalIn(IOPORT_F, BIT_4);  /* RF4 is RX for COM5 */
                CNPUFSET = (BIT_4 | BIT_5); /* set pull-ups */
                UARTConfigure(UART5, c);
                UARTSetLineControl(UART5, f);
                UARTSetDataRate(UART5, PBCLK2, bps);
                UARTSetFifoMode(UART5, UART_INTERRUPT_ON_RX_NOT_EMPTY);
                INTSetVectorPriority(INT_SOURCE_USART_5_RECEIVE, INT_PRIORITY_LEVEL_3);
                INTClearFlag(INT_SOURCE_USART_5_RECEIVE);
                INTEnable(INT_SOURCE_USART_5_RECEIVE, INT_ENABLED);
                UARTEnable(UART5, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_TX | UART_RX));
				PPSInput(1, U5RX, RPF4);
                PPSOutput(2, RPF5, U5TX);
            }
            PPSLock;
            break;

        case UART6:
            INTEnable(INT_SOURCE_USART_6_RECEIVE, INT_DISABLED);
            UARTEnable(UART6, 0);
            PORTResetPins(IOPORT_B, (BIT(6) | BIT(7)));
            PPSUnLock;
            PPSOutput(3, RPB7, NULL);
            if(buff_size && bps) {
                PORTSetPinsDigitalOut(IOPORT_B, BIT_7); /* RB7 is TX for COM6 */
                PORTSetPinsDigitalIn(IOPORT_B, BIT_6);  /* RB6 is RX for COM6 */
                CNPUBSET = (BIT_6 | BIT_7); /* set pull-ups */
                UARTConfigure(UART6, c);
                UARTSetLineControl(UART6, f);
                UARTSetDataRate(UART6, PBCLK2, bps);
                UARTSetFifoMode(UART6, UART_INTERRUPT_ON_RX_NOT_EMPTY);
                INTSetVectorPriority(INT_SOURCE_USART_6_RECEIVE, INT_PRIORITY_LEVEL_3);
                INTClearFlag(INT_SOURCE_USART_6_RECEIVE);
                INTEnable(INT_SOURCE_USART_6_RECEIVE, INT_ENABLED);
                UARTEnable(UART6, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_TX | UART_RX));
				PPSInput(4, U6RX, RPB6);
                PPSOutput(3, RPB7, U6TX);
            }
            PPSLock;
            break;
    }
    com_buff_size[port] = buff_size;
    com_rx_in[port] = com_rx_out[port] = 0;
	x_free((byte **) &com_buff[port]);
    x_malloc((byte **) &com_buff[port], buff_size);
    return !!(buff_size && !com_buff[port]);
}


/* UART reception function */
/* will return -1 if the buffer is empty */
int UART_rx(unsigned char port) {
    if(com_rx_out[port] == com_rx_in[port]) return -1;
    uint8_t c = com_buff[port][com_rx_out[port]];
    if(++com_rx_out[port] >= com_buff_size[port]) com_rx_out[port] = 0;
    return (int) c;
}


/* UART reception function */
/* will return -1 if the buffer is empty, otherwise the first character in the buffer but without removing it */
int UART_peek(unsigned char port) {
    if(com_rx_out[port] == com_rx_in[port]) return -1;
    return (int) com_buff[port][com_rx_out[port]];
}


/* return the number of bytes in a UART buffer */
int UART_buffer(unsigned char port) {
    int cnt = com_rx_out[port] - com_rx_in[port];
    if(cnt < 0) cnt += com_buff_size[port];
    return cnt;
}


/* general UART RX function (INTERRUPT) */
void UART_rxInt(unsigned char port) {
	while(UARTReceivedDataIsAvailable(port)) {
		UART_DATA c = UARTGetData(port);
		int e = (UARTGetLineStatus(port) & (UART_OVERRUN_ERROR | UART_FRAMING_ERROR | UART_PARITY_ERROR));
		if(!e && com_buff[port]) {
			com_buff[port][com_rx_in[port]] = (byte) c.__data;
			if(++com_rx_in[port] >= com_buff_size[port]) com_rx_in[port] = 0;
            if(com_rx_in[port] == com_rx_out[port]) {
                if(++com_rx_out[port] >= com_buff_size[port]) com_rx_out[port] = 0;   /* discard the oldest character in the buffer */
            }
            #if CONSOLE_ECHO > 0
                if(CONSOLE_COM == port) _mon_putc(c.__data);
            #endif
		}
	}
}


/* UART1 RX interrupt */
void __ISR(_UART1_RX_VECTOR, ipl3auto) IntUart1Handler(void) {
	UART_rxInt(UART1);
    U1STA &= ~(UART_OVERRUN_ERROR | UART_FRAMING_ERROR | UART_PARITY_ERROR);
	INTClearFlag(INT_SOURCE_USART_1_RECEIVE);
}


/* UART2 RX interrupt */
void __ISR(_UART2_RX_VECTOR, ipl3auto) IntUart2Handler(void) {
	UART_rxInt(UART2);
    U2STA &= ~(UART_OVERRUN_ERROR | UART_FRAMING_ERROR | UART_PARITY_ERROR);
	INTClearFlag(INT_SOURCE_USART_2_RECEIVE);
}


/* UART3 RX interrupt */
void __ISR(_UART3_RX_VECTOR, ipl3auto) IntUart3Handler(void) {
	UART_rxInt(UART3);
    U3STA &= ~(UART_OVERRUN_ERROR | UART_FRAMING_ERROR | UART_PARITY_ERROR);
	INTClearFlag(INT_SOURCE_USART_3_RECEIVE);
}

/* UART4 RX interrupt */
void __ISR(_UART4_RX_VECTOR, ipl3auto) IntUart4Handler(void) {
	UART_rxInt(UART4);
    U4STA &= ~(UART_OVERRUN_ERROR | UART_FRAMING_ERROR | UART_PARITY_ERROR);
	INTClearFlag(INT_SOURCE_USART_4_RECEIVE);
}


/* UART5 RX interrupt */
void __ISR(_UART5_RX_VECTOR, ipl3auto) IntUart5Handler(void) {
	UART_rxInt(UART5);
    U5STA &= ~(UART_OVERRUN_ERROR | UART_FRAMING_ERROR | UART_PARITY_ERROR);
	INTClearFlag(INT_SOURCE_USART_5_RECEIVE);
}


/* UART6 RX interrupt */
void __ISR(_UART6_RX_VECTOR, ipl3auto) IntUart6Handler(void) {
	UART_rxInt(UART6);
    U6STA &= ~(UART_OVERRUN_ERROR | UART_FRAMING_ERROR | UART_PARITY_ERROR);
	INTClearFlag(INT_SOURCE_USART_6_RECEIVE);
}


/* send character to UART (COM) port */
void UART_tx(unsigned char port, char data) {
    while(!(UARTTransmitterIsReady(port))) continue;
    UART_DATA ud;
    ud.__data = (UINT16) data;
    UARTSendData((port), ud);
    while(!(UARTTransmissionHasCompleted(port))) continue;
}


/* write a character to the console */
/* (ch) character */
void _mon_putc(char ch) {
    if(CONSOLE_COM >= 0) UART_tx(CONSOLE_COM, ch);
    if(hwp_flags & HWP_ECLK_PRESENT) SerUSBPutC(ch); /* output to the USB console (always on) */
}


void _mon_puts(const char *s) {
    while(s && *s) _mon_putc(*(s++));
}


void _mon_write (const char *s, unsigned int count) {
    while(s && count--) _mon_putc(*(s++));
}


/* get character from the console */
/* (blocking) non-zero indicates that the function must be blocking and wait for character */
/* return the current character from the input buffer, or EOF in case the buffer is empty */
int _mon_getc(int blocking) {
    int ch = EOF;
    do {
        if(CONSOLE_COM >= UART1 && CONSOLE_COM < COM_PORTS && com_rx_in[CONSOLE_COM] != com_rx_out[CONSOLE_COM]) {
            ch = com_buff[CONSOLE_COM][com_rx_out[CONSOLE_COM]];
            if(++com_rx_out[CONSOLE_COM] >= com_buff_size[CONSOLE_COM]) com_rx_out[CONSOLE_COM] = 0;
        }
        else if(hwp_flags & HWP_ECLK_PRESENT) ch = serUSBGetC();
        if(ch > EOF ) {
            #ifdef CONSOLE_ECHO
                _mon_putc(ch);
            #endif
        }
    } while(blocking && ch == EOF);
    return ch;
}


/* getch() hook */
char getch(void) {
    return _mon_getc(1);
}


/* get character from the console without removing it from the buffer */
/* return the current character from the input buffer, or EOF in case the buffer is empty */
int kbhit(void) {
    int ch = EOF;
    if(CONSOLE_COM >= 0 && com_rx_in[CONSOLE_COM] != com_rx_out[CONSOLE_COM]) { /* check the serial console buffer */
        ch = com_buff[CONSOLE_COM][com_rx_out[CONSOLE_COM]];
    }
    else if(hwp_flags & HWP_ECLK_PRESENT) { /* check the USB console */
        ch = serUSBGetC();
        if(ch > EOF && CONSOLE_COM >= 0) {  /* this character will have to be added to the console buffer too */
            com_buff[CONSOLE_COM][com_rx_in[CONSOLE_COM]] = (byte) ch;
            if(++com_rx_in[CONSOLE_COM] >= com_buff_size[CONSOLE_COM]) com_rx_in[CONSOLE_COM] = 0;
            if(com_rx_in[CONSOLE_COM] == com_rx_out[CONSOLE_COM]) {
                if(++com_rx_out[CONSOLE_COM] >= com_buff_size[CONSOLE_COM]) com_rx_out[CONSOLE_COM] = 0; /* discard oldest */
            }
        }
    }
    return ((ch > EOF) ? ch : 0);
}


/* Timer8 interrupt at 1ms */
void __ISR(_TIMER_8_VECTOR, ipl6auto) IntTimer8Handler(void) {
    static uint16_t s_ticks = 0;
    if(++s_ticks >= 1000) { s_ticks = 0; ss_time++; }
    ms_clock++;
    if(ms_timer) ms_timer--;
	INTClearFlag(INT_SOURCE_TIMER_8);
}


clock_t clock(void) {
    return (clock_t) ms_clock;
}


time_t time(time_t *tm) {
    return (time_t) ss_time;
}


/* open (with parameter >0) or close (with parameter 0) the USB console */
void openUSB(int state) {
    if(state) {
        if(hwp_flags & HWP_ECLK_PRESENT) {
            USBCSR0bits.SOFTCONN = 1;
            initUSBConsole();
        }
    }
    else USBCSR0bits.SOFTCONN = 0;  /* close */
}


/* reset the system */
void resetPlatform(void) {
    SoftReset();
}


/* not sure why the compiler wants this ??? */
int open(const char *buf, int flags, int mode) {}


/* CPU exceptions handler */
const static char *szException[] = {
    "Interrupt",                        /* 0 */
    "Unknown",                          /* 1 */
    "Unknown",                          /* 2 */
    "Unknown",                          /* 3 */
    "Address error (load or ifetch)",   /* 4 */
    "Address error (store)",            /* 5 */
    "Bus error (ifetch)",               /* 6 */
    "Bus error (load/store)",           /* 7 */
    "SysCall",                          /* 8 */
    "Breakpoint",                       /* 9 */
    "Reserved instruction",             /* 10 */
    "Coprocessor unusable",             /* 11 */
    "Arithmetic overflow",              /* 12 */
    "Trap (possible divide by zero)",   /* 13 */
    "Unknown",                          /* 14 */
    "Unknown",                          /* 15 */
    "Implementation specific 1",        /* 16 */
    "CorExtend Unusable",               /* 17 */
    "Coprocessor 2"                     /* 18 */
};
void _general_exception_handler(void) {
    volatile static unsigned long codeException;
    volatile static unsigned long addressException;
    asm volatile ("mfc0 %0,$13" : "=r" (codeException));
    asm volatile ("mfc0 %0,$14" : "=r" (addressException));
    codeException = (codeException & 0x7c) >> 2;
    printf("\r\n\nCPU EXCEPTION: %lu", codeException);
    if(codeException < 19) printf(" \"%s\"", szException[codeException]);
    printf(" at address $%04lX\r\nRestarting...\r\n\n\n", addressException);
    mSec(5000); /* 5 seconds delay */
    resetPlatform();
}


/* full initialisation of the hardware platform */
void initPlatform(void) {
    INTDisableInterrupts();
    DisableWDT();

	SystemUnlock();
	OSCCONbits.SLP2SPD = 1;	/* use FRC until the system clock is ready */
	SystemLock();

	hwp_flags |= HWP_ECLK_PRESENT;  /* ### TODO: check for present external clock before switching to it */

    if(sys_freq_khz == 0) {
        sys_freq_khz = ((unsigned long) SYSCLK) / 1000ul;
        set_sysFreq(sys_freq_khz);
        SysWaitStateConfig(1000ul * sys_freq_khz);
        SysPerformanceConfig((1000ul * sys_freq_khz), PCACHE_PREFETCH_ENABLE_ALL);
    }

    #ifdef DISABLE_DEBUG
    mJTAGPortEnable(DEBUG_JTAGPORT_OFF);
    #endif

	DisableWDT();
	SystemUnlock();

	CFGCONbits.IOLOCK = 0;	/* enable writing to the PPSx registers */
	CFGCONbits.PMDLOCK = 0;	/* enable writing to the PMDx registers */
	/* see Errata #9 */
	/* CFGCONbits.USBSSEN = 1; */	/* shut down the USB in sleep mode */
	CFGCONbits.OCACLK = 1;	/* OC outputs will use their own timers */
	CFGCONbits.ICACLK = 1;	/* IC inputs will use their own timers */
	CFGCONbits.IOANCPEN = 0; /* disable the analogue port charge pump */

    #ifdef DISABLE_DEBUG
	CFGCONbits.JTAGEN = 0;	/* disable JTAG */
	CFGCONbits.TDOEN = 0;	/* disable TDO output */
	CFGCONbits.TROEN = 0;	/* disable trace output */
    #endif

	OSCPBOutputClockDisable(OSC_PERIPHERAL_BUS_8);	/* PBCLK8 is never needed */
	SystemLock();

	PRISS = 0x76543210;		/* assign shadow set #7-#1 to priority level #7-#1 ISRs */
    RNGCONbits.TRNGEN = 1;  /* enable the true hardware random number generator */
    srand(RNGSEED2);

    /* initialise and enable Timer8 with clock exactly 1 MHz to generate interrupts every 1 millisecond */
    OpenTimer8((T8_ON | T8_IDLE_CON | T8_GATE_OFF | T8_32BIT_MODE_OFF | T8_SOURCE_INT | T8_PS_1_64), 1000);
	WriteTimer8(0);
    ConfigIntTimer8(T8_INT_ON | T8_INT_PRIOR_6);

    /* initialise the managed memory (dynamic allocation blocks) */
    if(SysMem == NULL || SysMemSize == 0) {
        SysMem = MEMORY;
        SysMemSize = x_meminit();
    }

    /* no video in the PIC32MZ port */
    x_malloc((byte **) &VidMem, 0); // set VidMem = NULL
    VidMemSize = 0;
    Vmode = Hres = Vres = 0;

    /* initialise the RTC */
    if(ss_time == 0) {
        struct tm tt;
        tt.tm_year = 121;   // 2021
        tt.tm_mon = 0;      // January
        tt.tm_mday = 1;
        tt.tm_hour = tt.tm_min = tt.tm_sec = 0;
        tt.tm_isdst = 0;
        ss_time = mktime(&tt);
    }

	/* disable serial port modules */
    openSPI(SPI_CHANNEL1, 0, 0, 0);
    openSPI(SPI_CHANNEL2, 0, 0, 0);
    openSPI(SPI_CHANNEL3, 0, 0, 0);
    openCOM(UART1, 0, 0, 0, 0);
    openCOM(UART2, 0, 0, 0, 0);
    openCOM(UART3, 0, 0, 0, 0);
    openCOM(UART4, 0, 0, 0, 0);
    openCOM(UART5, 0, 0, 0, 0);
    openCOM(UART6, 0, 0, 0, 0);

	/* close PWM ports */
	CloseOC1();
	CloseOC2();
    CloseOC3();
	CloseOC4();
	CloseOC5();
    CloseOC6();
	CloseOC7();
	CloseOC8();
	CloseOC9();
	CloseTimer23();
	CloseTimer45();
	CloseTimer67();

	/* reset the PMD registers and disable the hardware modules which are never used */
	PMD1 = PMD2 = PMD3 = PMD4 = PMD5 = PMD6 = PMD7 = 0;
	PMD1 |= BIT(12);	/* comparator voltage reference */
	PMD2 |= (BIT(0) | BIT(1));	/* comparators 1 and 2 */
	PMD3 |= 0x1FF;      /* input capture 1..9 */
	PMD5 |= (BIT(11) | BIT(12) | BIT(13));	/* SPI4, SPI5, SPI6 */
	PMD5 |= (BIT(17) | BIT(18) | BIT(19));	/* I2C2, I2C3, I2C4 */
	PMD5 |= BIT(29);	/* CAN2 */
	PMD6 |= (BIT(8) | BIT(9) | BIT(10) | BIT(11));	/* reference output clocks */
	PMD6 |= BIT(16);	/* PMP */
	PMD6 |= BIT(17);	/* EBI */
	PMD6 |= BIT(23);	/* SQI */
	PMD6 |= BIT(28);	/* Ethernet */
	PMD7 |= BIT(4);     /* DMA */
	PMD7 |= BIT(22);	/* Crypto */

	/* full port reset; all ports initialised as digital inputs */
	TRISBSET = TRISCSET = TRISDSET = TRISESET = TRISFSET = TRISGSET = ULONG_MAX;
    ANSELBCLR = ANSELECLR = ANSELGCLR =  0;
    LATBCLR = LATCCLR = LATDCLR = LATECLR = LATFCLR = LATGCLR = 0;
    CNENBCLR = CNENCCLR = CNENDCLR = CNENECLR = CNENFCLR = CNENGCLR = 0;
    CNCONBCLR = CNCONCCLR = CNCONDCLR = CNCONECLR = CNCONFCLR = CNCONGCLR = 0;
    CNPUBCLR = CNPUCCLR = CNPUDCLR = CNPUECLR = CNPUFCLR = CNPUGCLR = 0;
    CNPDBCLR  = CNPDCCLR = CNPDDCLR = CNPDECLR = CNPDFCLR = CNPDGCLR = 0;

	/* set slew rate for the ports */
	SRCON0B = SRCON0E = SRCON0F = SRCON0G = 0;
	SRCON1B = SRCON1E = SRCON1F = SRCON1G = ULONG_MAX;

    /* initialise the SD card interface */
    currentSD = 0;
    CS_HIGH();

    /* reserve the external oscillator SLEEP control output */
    /* if(hwp_flags & HWP_ECLK_PRESENT) {
        PORTSetPinsDigitalOut(IOPORT_C, BIT_15);
        LATCbits.LATC15 = 0;
    } */

	/* basic initialisation of the ADC */
	ADC12Setup(ADC12_VREF_AVDD_AVSS, ADC12_CHARGEPUMP_DISABLE,
				ADC12_OUTPUT_DATA_FORMAT_INTEGER, FALSE,
				ADC12_FAST_SYNC_SYSTEM_CLOCK_DISABLE, ADC12_FAST_SYNC_PERIPHERAL_CLOCK_DISABLE,
				ADC12_INTERRUPT_BIT_SHIFT_LEFT_0_BITS, 0,
				ADC12_CLOCK_SOURCE_FRC, 1, ADC12_WARMUP_CLOCK_128);

	/* initialise and open the console ports */
    openCOM(CONSOLE_COM, COM_RX_SIZE, CONSOLE_BAUD,
                (UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1), (UART_ENABLE_PINS_TX_RX_ONLY));
    if(hwp_flags & HWP_ECLK_PRESENT) openUSB(1);

    if((hwp_flags & HWP_ECLK_WARNED) == 0) {    /* missing clock warning */
        printf("UUUUUUUUUUUUUUUUUUUUUUUU\r\n"); /* do this only once */
        if((hwp_flags & HWP_ECLK_PRESENT) == 0) printf("\r\n\nWARNING: missing external clock\r\n\n");
        hwp_flags |= HWP_ECLK_WARNED;
    }

	/* configure and enable interrupts */
	INTConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);
	INTEnableSystemMultiVectoredInt();
}


/* set system clock frequency in MHz and return 0 if successful or -1 in case of invalid parameter */
int __attribute__((nomips16, nomicromips)) __attribute__((optimize("-O0"))) set_sysFreq(unsigned long freq_khz) {
	unsigned int mul = 48;	/* PLL multiplier value (multiplying input clock 8 MHz) */
	OSC_SYSPLL_OUT_DIV div = OSC_SYSPLL_OUT_DIV_2;		/* PLL divider value */
	/* reminder: peripheral clocks (especially PBCLK3) must be kept at 64 MHz at all times */
	OSC_PB_CLOCK_DIV_TYPE pdiv145 = OSC_PB_CLOCK_DIV_2;	/* divider for PBCLK1/4/5 (up to 100 MHz) */
	OSC_PB_CLOCK_DIV_TYPE pdiv23 = OSC_PB_CLOCK_DIV_3;	/* divider for PBCLK2/3 (fixed 64 MHz) */
	OSC_PB_CLOCK_DIV_TYPE pdiv7 = OSC_PB_CLOCK_DIV_1;	/* divider for PBCLK7 */

	if(freq_khz == 4000) {
		mul = 64;
		div = OSC_SYSPLL_OUT_DIV_8;
		pdiv145 = OSC_PB_CLOCK_DIV_1;
		pdiv23 = OSC_PB_CLOCK_DIV_1;
		pdiv7 = OSC_PB_CLOCK_DIV_16;
	}

	else if(freq_khz == 16000) {
		mul = 64;
		div = OSC_SYSPLL_OUT_DIV_8;
		pdiv145 = OSC_PB_CLOCK_DIV_1;
		pdiv23 = OSC_PB_CLOCK_DIV_1;
		pdiv7 = OSC_PB_CLOCK_DIV_4;
	}

	else if(freq_khz == 64000) {
		mul = 64;
		div = OSC_SYSPLL_OUT_DIV_8;
		pdiv145 = OSC_PB_CLOCK_DIV_1;
		pdiv23 = OSC_PB_CLOCK_DIV_1;
		pdiv7 = OSC_PB_CLOCK_DIV_1;
	}

	else if(freq_khz == 128000) {
		mul = 64;
		div = OSC_SYSPLL_OUT_DIV_4;
		pdiv145 = OSC_PB_CLOCK_DIV_2;
		pdiv23 = OSC_PB_CLOCK_DIV_2;
		pdiv7 = OSC_PB_CLOCK_DIV_1;
	}

	else if(freq_khz == 192000) {
		mul = 48;
		div = OSC_SYSPLL_OUT_DIV_2;
		pdiv145 = OSC_PB_CLOCK_DIV_2;
		pdiv23 = OSC_PB_CLOCK_DIV_3;
		pdiv7 = OSC_PB_CLOCK_DIV_1;
	}

	else if(freq_khz == 256000) {
		mul = 64;
		div = OSC_SYSPLL_OUT_DIV_2;
		pdiv145 = OSC_PB_CLOCK_DIV_3;
		pdiv23 = OSC_PB_CLOCK_DIV_4;
		pdiv7 = OSC_PB_CLOCK_DIV_1;
	}

	else return -1;
	OSCPLLClockUnlock();	/* temporarily switch to FRC */
	OSCClockSourceSwitch(OSC_FRC, OSC_SYSPLL_FREQ_RANGE_BYPASS, OSC_SYSPLL_IN_DIV_1, 1, 1, TRUE);
	OSCPLLClockLock();

	SystemUnlock();
	OscPBClockDivisorSet(OSC_PERIPHERAL_BUS_1, pdiv145);
	OscPBOutputClockEnable(OSC_PERIPHERAL_BUS_1);
	OscPBClockDivisorSet(OSC_PERIPHERAL_BUS_2, pdiv23);
	OscPBOutputClockEnable(OSC_PERIPHERAL_BUS_2);
	OscPBClockDivisorSet(OSC_PERIPHERAL_BUS_3, pdiv23);
	OscPBOutputClockEnable(OSC_PERIPHERAL_BUS_3);
	OscPBClockDivisorSet(OSC_PERIPHERAL_BUS_4, pdiv145);
	OscPBOutputClockEnable(OSC_PERIPHERAL_BUS_4);
	OscPBClockDivisorSet(OSC_PERIPHERAL_BUS_5, pdiv145);
	OscPBOutputClockEnable(OSC_PERIPHERAL_BUS_5);
	OscPBClockDivisorSet(OSC_PERIPHERAL_BUS_7, pdiv7);
	OscPBOutputClockEnable(OSC_PERIPHERAL_BUS_7);
	OscPBClockDivisorSet(OSC_PERIPHERAL_BUS_8, pdiv145);	/* not used but still have to make sure it is within limits */
	SystemLock();

	unsigned long sysfreq = (8000000 * mul) / (div + 1);    /* calculate the new SYSCLK from (mul) and (div) and PLL input clock 8 MHz */
	SysWaitStateConfig(sysfreq);
	SysPerformanceConfig(sysfreq, PCACHE_PREFETCH_ENABLE_ALL);
	OSCPLLClockUnlock();
	if(hwp_flags & HWP_ECLK_PRESENT) {  /* using the external clock */
		OSCClockSourceSwitch(OSC_PRIMARY_WITH_PLL,
					OSC_SYSPLL_FREQ_RANGE_5M_TO_10M, OSC_SYSPLL_IN_DIV_3, mul, div, TRUE);
	}
	else {  /* using the 8 MHz FRC */
		OSCClockSourceSwitch(OSC_FRC_WITH_PLL,
					OSC_SYSPLL_FREQ_RANGE_5M_TO_10M, OSC_SYSPLL_IN_DIV_1, mul, div, TRUE);
	}
	OSCPLLClockLock();
	sys_freq_khz = sysfreq / 1000;
	return 0;
}


// I2C ==========================================================================================

#define SCL0    { I2C_LAT &= ~SCL; I2C_TRIS &= ~SCL; }
#define SCL1    { I2C_LAT |= SCL; I2C_TRIS &= ~SCL; }
#define SDA0    { I2C_LAT &= ~SDA; I2C_TRIS &= ~SDA; }
#define SDA1    { I2C_LAT |= SDA; I2C_TRIS &= ~SDA; }
#define SDAin   { I2C_TRIS |= SDA; }
#define SDAget  !!(I2C_PORT & SDA)

uint32_t i2c_bit_us = I2C_BIT_US;


int i2cInit(int baudrate) {
    I2C_PULLUP |= (SCL | SDA);
    I2C_LAT |= (SCL | SDA);
    I2C_TRIS &= ~(SCL | SDA);
    if(baudrate > 0) {
        i2c_bit_us = 1000000 / baudrate;
        if(baudrate == 0) baudrate++;
    }
    return 1000000 / baudrate;
}


// on entry: SDA=?, SCL=?
// on exit: SDA=0, SCL=0
void i2cStart(void) {
    I2C_PULLUP |= (SCL | SDA);
    SCL1; SDAin;
    while(SDAget == 0) {
        I2C_LAT ^= SCL;
        uSec(i2c_bit_us);
    }
    SCL1; SDA1;
    uSec(i2c_bit_us);
    SDA0; uSec(i2c_bit_us);
    SCL0; uSec(i2c_bit_us);
}


// on entry: SDA=?, SCL=0
// on exit: SDA=0, SCL=0
void i2cRepStart(void) {
    SDA1; SCL1; uSec(i2c_bit_us);
    SDA0; uSec(i2c_bit_us);
    SCL0; uSec(i2c_bit_us);
}


// on entry: SDA=?, SCL=0
// on exit: SDA=in, SCL=in
void i2cStop(void) {
    SDA0; SCL1; uSec(i2c_bit_us);
    SDA1; uSec(i2c_bit_us);
    SCL1; uSec(i2c_bit_us);
    //I2C_TRIS |= (SCL | SDA);
}


// will return 0 when the sent byte has been acknowledged, 1 in NAK case
int i2cSend(unsigned char data8) {
    char b = 8;
    while(b--) {
        if(data8 & 0x80) { SDA1; } else { SDA0; }
        data8 <<= 1;
        SCL1; uSec(i2c_bit_us);
        SCL0; SDAin; uSec(i2c_bit_us);
    }
    SCL1; uSec(i2c_bit_us);
    b = SDAget;
    SCL0; uSec(i2c_bit_us);
    return (int) b;
}


// will return the received byte
// parameter (ack) determines whether the received byte will be acknowledged (ack=1), or not (ack=0)
unsigned char i2cRecv(int ack) {
    unsigned char data8 = 0;
    SDAin;
    char b = 8;
    while(b--) {
        SCL1; uSec(i2c_bit_us);
        data8 <<= 1;
        if(I2C_PORT & SDA) data8 |= 1;
        SCL0; uSec(i2c_bit_us);
    }
    if(ack) { SDA1; } else { SDA0; }
    SCL1; uSec(i2c_bit_us);
    SCL0; uSec(i2c_bit_us);
    return data8;
}


// primitive function stubs =====================================================================

int dummy;	/* used only to avoid the annoying "unused variable" messages from the compiler */
void initVideo(uint8_t mode) { dummy = mode; }
void clearScreen(int c) { dummy = c; }
void scrollUp(int vl, int c) { dummy = vl = c; }
void setPixel(int x, int y, int c) { dummy = x = y = c; }
int getPixel(int x, int y) { dummy = x = y; return 0; }
void beep(void) {}
