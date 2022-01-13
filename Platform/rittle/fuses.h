#ifndef FUSES_H
#define	FUSES_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <xc.h>

/* DEVCFG0 */
#pragma config ICESEL =     ICS_PGx2
#pragma config EJTAGBEN =   NORMAL
#pragma config DEBUG =      OFF
#pragma config JTAGEN =     OFF
#pragma config TRCEN =      OFF
#pragma config BOOTISA =    MIPS32
#pragma config FECCCON =    OFF_UNLOCKED
#pragma config FSLEEP =     OFF
#pragma config DBGPER =     PG_ALL
#pragma config SMCLR =      MCLR_NORM
#pragma config SOSCGAIN =   GAIN_1_5X
#pragma config SOSCBOOST =  OFF
#pragma config POSCGAIN =   GAIN_1_5X
#pragma config POSCBOOST =  OFF
#pragma config CP =         OFF

/* DEVCFG1 */
#pragma config FNOSC =      SPLL
#pragma config DMTINTV =    WIN_127_128
#pragma config FSOSCEN =    OFF
#pragma config IESO =       ON
#pragma config POSCMOD =    HS		/* change to EC to force use of external clock only */
#pragma config OSCIOFNC =   OFF
#pragma config FCKSM =      CSECME
#pragma config WDTPS =      PS128
#pragma config WDTSPGM =    STOP
#pragma config FWDTEN =     OFF
#pragma config WINDIS =     NORMAL
#pragma config FWDTWINSZ =  WINSZ_75
#pragma config DMTCNT =     DMT31
#pragma config FDMTEN =     OFF

/*
    DEVCFG2
    System starts initially from the internal FRC (no USB available from it!)
    8 MHz input clock from FRC
        : divided by 1 = 8 MHz PLL input (must be within range 5 - 64)
        : multiplied by 48 = 384 MHz PLL frequency (must be within range 350 - 700)
        : divided by 2 = 192 MHz system clock (must be within range 60 - 200)
*/
#pragma config FPLLIDIV =   DIV_1		/* change to DIV_3 for forced 24 MHz external clock only */
#pragma config FPLLRNG =    RANGE_5_10_MHZ
#pragma config FPLLICLK =   PLL_FRC     /* change to PLL_POSC to force external clock only */
#pragma config FPLLMULT =   MUL_48
#pragma config FPLLODIV =   DIV_2
#pragma config UPLLFSEL =   FREQ_24MHZ

/* DEVCFG3 */
#pragma config FMIIEN =     OFF
#pragma config FETHIO =     OFF
#pragma config PGL1WAY =    OFF
#pragma config PMDL1WAY =   OFF
#pragma config IOL1WAY =    OFF
#pragma config FUSBIDIO =   OFF
#pragma config USERID =     0x7686

/* BF1SEQ3 */
#pragma config TSEQ =       0x0000
#pragma config CSEQ =       0xFFFF

#ifdef	__cplusplus
}
#endif

#endif  // FUSES_H
