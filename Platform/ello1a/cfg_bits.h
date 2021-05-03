#ifndef CFG_BITS_H
#define	CFG_BITS_H

#ifdef	__cplusplus
extern "C" {
#endif

// NOTE: configuration bits calibrated for 20 MHz external clock

#pragma config FNOSC = PRIPLL       // Oscillator Selection
                                    // PRI      Primary oscillator (XT, HS, EC)
                                    // PRIPLL   Primary oscillator with PLL (XT, HS, EC)
                                    // SOSC     Secondary oscillator
                                    // LPRC     Low power RC oscillator
                                    // FRC      Fast RC oscillator
                                    // FRCPLL   Fast RC oscillator with PLL
                                    // FRCDIV16 Fast RC oscillator with divide by 16
                                    // FRCDIV   Fast RC oscillator with divide

#pragma config POSCMOD = EC         // Primary Oscillator Selection
                                    // HS   High Speed oscillator
                                    // EC   External Clock oscillator
                                    // XT   Crystal oscillator
                                    // OFF  Disabled

#pragma config FPLLIDIV = DIV_4     // PLL Input Divide by 1, 2, 3, 4, 5, 6, 10, 12
#pragma config FPLLMUL = MUL_20     // PLL Multiply by 15, 16, 17, 18, 19, 20, 21, 24
#pragma config FPLLODIV = DIV_2     // PLL Output Divide by 1, 2, 4, 8, 16, 32, 64, 256

#pragma config UPLLIDIV = DIV_5     // USB PLL Input Divide by 1, 2, 3, 4, 5, 6, 10, 12
#pragma config UPLLEN = ON          // USB PLL ON or OFF

#pragma config FPBDIV = DIV_1       // Peripheral Bus Clock Divide by 1, 2, 4, 8
#pragma config OSCIOFNC = OFF       // CLKO output on the OSCO pin ON or OFF
#pragma config FSOSCEN = OFF        // Secondary Oscillator ON or OFF

#pragma config IESO = ON            // Internal External Switchover (Two-Speed Start-up) ON or OFF
#pragma config FCKSM = CSECME       // Clock Switching and Monitor Selection
                                    // CSECME   Clock Switching Enabled, Clock Monitoring Enabled
                                    // CSECMD	Clock Switching Enabled, Clock Monitoring Disabled
                                    // CSDCMD	Clock Switching Disabled, Clock Monitoring Disabled

#pragma config FWDTEN = OFF         // Watchdog Timer ON or OFF
#pragma config WDTPS = PS128        // Watchdog Timer Postscale from 1:1 to 1:1,048,576
#pragma config WINDIS = OFF         // Watchdog Timer Window Mode ON (window mode) or OFF (non-window mode)
#pragma config FWDTWINSZ = WINSZ_75 // Watchdog Timer Window Size (75%, 50%, 37%, 25%)

#pragma config IOL1WAY = OFF        // Peripheral Pin Select Configuration ON (one reconfig) or OFF (multiple)
#pragma config PMDL1WAY = OFF       // Peripheral Module Disable Configuration ON (one reconfig) or OFF (multiple)

#pragma config FVBUSONIO = OFF      // USB VBUS_ON pin ON or OFF
#pragma config FUSBIDIO = OFF       // USB USBID pin ON or OFF

#pragma config JTAGEN = OFF         // JTAG ON or OFF
#pragma config DEBUG = OFF          // Background Debugger ON or OFF

#pragma config BWP = OFF            // Boot Flash Write Protect ON or OFF
#pragma config CP = OFF             // Code Protect Enable ON or OFF
#pragma config PWP = OFF            // Program Flash Write Protect ON, OFF or PWPnK (n = 1...256)

#ifdef	__cplusplus
}
#endif

#endif
