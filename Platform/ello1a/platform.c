#include "cfg_bits.h"
#include "platform.h"
#include "../../config.h"
#include "../../RIDE/kbdcodes.h"
#include "../../RIDE/graphics.h"
#include "../../RIDE/ride.h"   /* for the settings{} structure */

#include "m-stack/usb/usb_config.h"
#include "m-stack/usb/usb.h"
#include "m-stack/usb/usb_cdc.h"
#include "m-stack/usb/usb_ch9.h"

const volatile unsigned int *pbase[] = { BASEA, BASEB, BASEC, BASED, BASEE, BASEF, BASEG };

volatile unsigned long keybuf = 0;
volatile unsigned char msK = 0;
volatile long conBuffer[CON_BUFFER_SIZE];
volatile unsigned char conRx_in = 0;    // incoming character index
volatile unsigned char conRx_out = 0;   // outgoing character index
int is_big = 0;
int getch_ch = 0;


// initialise the system
void initPlatform(void) {
    mJTAGPortEnable(DEBUG_JTAGPORT_OFF);
	DisableWDT();
    srand(PORTA + PORTB);
    
    uSec(10000);    // allow some time to the oscillator to stabilise
    TRISA = TRISB = 0xFFFFFFFF;
    ANSELA = ANSELB = 0;
    LATA = LATB = 0;
    CNENA = CNENB = CNCONA = CNCONB = 0;
    CNPUA = CNPUB = CNPDA = CNPDB = 0;

    INTConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);
    INTEnableSystemMultiVectoredInt();  // allow vectored interrupts
    INTEnableInterrupts();
    msClock = msCountdown = 0;
    
    // check for RTC
    I2C_PULLUP |= (SCL | SDA);  // enable the internal pull-up resistor on the I2C lines
    I2C_LAT |= (SCL | SDA);
    I2C_TRIS &= ~(SCL | SDA);
    struct tm tt;
    if(!i2cGetTime(&tt)) {
        tt.tm_year = 121;
        tt.tm_mon = 0;          // January
        tt.tm_mday = 1;
        tt.tm_hour = tt.tm_min = tt.tm_sec = 0;
        tt.tm_isdst = 0;
    }
    sUTime = mktime(&tt);
    
    // setup Timer 4 as a one-millisecond timer to maintain the system timers
    PR4 = 1000 * (_PB_FREQ / 2 / 1000000) - 1;  // 1 ms
    T4CON = 0x8010;         // T4 on, pre-scaler 1:2
    mT4SetIntPriority(5);  	// set priority
    mT4ClearIntFlag();      // clear the interrupt flag
    mT4IntEnable(1);       	// enable interrupt
    
    // initialise the SD card interface
    currentSD = 0;
    CS_HIGH();
    CNPUASET = BIT(1);      // enable the internal pull-up resistor on the SD card CS# line
    
    // initialise the PS/2 keyboard
    LATBSET = (BIT(5) | BIT(7));    // initialise the lines as output for a short time
    TRISBCLR = (BIT(5) | BIT(7));
    CNPUBSET = (BIT(5) | BIT(7));   // enable the internal pull-up resistor on the PS/2 lines
    kbdFlags = DEFAULT_KBD_FLAGS;
    mSec(500);
    initKeyboard();
    
    // check for serial console and initialise it if found
    if((enable_flags & FLAG_PS2) == 0) {
        CNPUBCLR = BIT(5); CNPDBSET = BIT(5);   // set pull-down on the RX pin
        TRISBSET = BIT(5); mSec(1);
        if(PORTB & BIT(5)) enable_flags |= FLAG_SERIAL;
        CNPDBCLR = BIT(5); CNPUBSET = BIT(5);   // restore the pull-up on the RX pin
        if(enable_flags & FLAG_SERIAL) {
            memset((char *) conBuffer, 0, sizeof(conBuffer));
            conRx_in = conRx_out = 0;
            PPSInput(2, U2RX, RPB5);
            SERIAL_TX = 1; SERIAL_TX_TRIS = 0;  // Tx will be bit-banged
            UARTEnable(UART2, 0);
            UARTConfigure(UART2, (UART_ENABLE_HIGH_SPEED | UART_ENABLE_PINS_TX_RX_ONLY));
            UARTSetLineControl(UART2, (UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1));
            UARTSetDataRate(UART2, _PB_FREQ, SERIAL_BAUDRATE);
            UARTSetFifoMode(UART2, UART_INTERRUPT_ON_RX_NOT_EMPTY);
            UARTEnable(UART2, (UART_ENABLE | UART_PERIPHERAL | UART_TX | UART_RX));
            INTSetVectorPriority(INT_UART_2_VECTOR, INT_PRIORITY_LEVEL_3);
            INTEnable(INT_U2RX, INT_ENABLED);
        }
    }
   
    // initialise the system font (must be done before the video!)
    fontScale = 1;
    fontFcol = 0xFFFFFF;
    fontBcol = 0;
    font = (font_t *) &sysFont0508;
    
    // initialise the managed memory (dynamic allocation blocks)
    SysMem = MEMORY; 
    SysMemSize = x_meminit();
    
    // initialise the video generator (uses Timer3, OC3, SPI2, SS2)
    // the video initialisation function will grab the needed RAM
    initVideo(DEFAULT_VMODE);
	clearScreen(0);
    
    // initialise USB
    #ifdef MULTI_CLASS_DEVICE
	cdc_set_interface_list(cdc_interfaces, sizeof(cdc_interfaces));
    #endif
    usb_init();
}


// UART console interrupt handler
void __ISR(_UART_2_VECTOR, IPL3AUTO) ConsoleRxHandler(void) {
    while(UARTReceivedDataIsAvailable(UART2)) {
        int e = UARTGetLineStatus(UART2) & (UART_PARITY_ERROR | UART_FRAMING_ERROR | UART_OVERRUN_ERROR);
        int c = UARTGetDataByte(UART2);
        U1STACLR = e;
        if(!e) {
            keybuf = (keybuf << 8) + c;
            msK = MSK_WAIT_MS;
        }
    }
    INTClearFlag(INT_U2RX);
}


// add (keybuf) to the key buffer
void addKey(void) {
    msK = 0;
    if(keybuf) {
        conBuffer[conRx_in] = keybuf; keybuf = 0;
        if(++conRx_in >= CON_BUFFER_SIZE) conRx_in = 0;
        if(conRx_in == conRx_out) {
            if(++conRx_out >= CON_BUFFER_SIZE) conRx_out = 0;
        }
    }
}


// CPU exceptions handler
void __attribute__ ((nomips16)) _general_exception_handler(void)	{
    const static char *szException[] = {
        "Interrupt",                        // 0
        "Unknown",                          // 1
        "Unknown",                          // 2
        "Unknown",                          // 3
        "Address error (load or ifetch)",   // 4
        "Address error (store)",            // 5
        "Bus error (ifetch)",               // 6
        "Bus error (load/store)",           // 7
        "SysCall",                          // 8
        "Breakpoint",                       // 9
        "Reserved instruction",             // 10
        "Coprocessor unusable",             // 11
        "Arithmetic overflow",              // 12
        "Trap (possible divide by zero)",   // 13
        "Unknown",                          // 14
        "Unknown",                          // 15
        "Implementation specific 1",        // 16
        "CorExtend Unusable",               // 17
        "Coprocessor 2"                     // 18
    };
    volatile static unsigned long codeException;
    volatile static unsigned long addressException;
    const char *pszExcept;
    asm volatile ("mfc0 %0,$13" : "=r" (codeException));
    asm volatile ("mfc0 %0,$14" : "=r" (addressException));
    codeException = (codeException & 0x7C) >> 2;
    if(codeException < 19) {
        pszExcept = szException[codeException];
        printf("\r\n\nCPU EXCEPTION: '%s' at address $%04lx\r\nRestarting...\r\n\n\n", pszExcept, addressException);
        mSec(5000); /* 5 seconds delay */
    }
    SoftReset();
}


void __attribute__ ((nomips16)) reset(void) {
    SoftReset();
}


/*
macro to reserve flash memory for saving/loading data and initialise to 0xFF's
note that (bytes) need to be a multiple of:
- BYTE_PAGE_SIZE (and aligned that way) if you intend to erase
- BYTE_ROW_SIZE (and aligned that way) if you intend to write rows
- sizeof(int) if you intend to write words
*/
#define ALLOCATE(name, align, bytes) unsigned char name[(bytes)] __attribute__ ((aligned(align), space(prog), section(".ifs"))) = { [0 ... (bytes)-1] = 0xFF }

#if IFS_DRV_KB > 0
ALLOCATE(ifs_data, BYTE_PAGE_SIZE, (IFS_DRV_KB * 1024ul));
#else
unsigned char ifs_data[1];  // fake IFS area when there is no IFS drive
#endif


// USB  =========================================================================================

#ifdef MULTI_CLASS_DEVICE
unsigned char cdc_interfaces[] = { 0 };
#endif

volatile char usbTxBuf[CON_BUFFER_SIZE];    // transmission buffer
volatile unsigned char usbTx_in = 0;        // incoming character index
volatile unsigned char usbTx_out = 0;       // outgoing character index


void doUSB(void) {
	if(U1OTGSTATbits.VBUSVD) {    // is there 5V on the USB?
        enable_flags |= FLAG_USB_PRES;
        #ifndef USB_USE_INTERRUPTS
        usb_service();
        #endif
        if(usb_is_configured()) {
            enable_flags |= FLAG_USB;
            
            // send data to host
            if(usb_in_endpoint_busy(2) == 0 && !usb_in_endpoint_halted(2)) {
                if(usbTx_in != usbTx_out) {
                    unsigned char *buf = usb_get_in_buffer(2);
                    int i;
                    for(i = 0; i < EP_2_IN_LEN && usbTx_in != usbTx_out; i++) {
                        buf[i] = usbTxBuf[usbTx_out];
                        if(++usbTx_out >= CON_BUFFER_SIZE) usbTx_out = 0;
                    }
                    usb_send_in_buffer(2, i);
                    if(i == EP_2_IN_LEN) {
                        while(usb_in_endpoint_busy(2));
                        usb_send_in_buffer(2, 0);
                    }
                }
            }

            // get data from host
            if(usb_out_endpoint_has_data(2) && !usb_in_endpoint_halted(2)) {
                unsigned char *buf;
                int idx, len = usb_get_out_buffer(2, (const unsigned char **) &buf);
                for(idx = 0; idx < len; idx++) {
                    unsigned char ck = buf[idx];
                    keybuf = (keybuf << 8) + ck; msK = 0;
                    if(keybuf >= 0x100) {   // escape sequences
                        if(((keybuf & 0xFF00) == (MULPS2 << 8))
                            || (keybuf & 0xFF000000)
                            || ((ck >= ':' && ck <= 'Z'))) addKey();
                    }
                    else if(keybuf == KEY_ESC || keybuf == MULPS2) msK = MSK_WAIT_MS;
                    else addKey();  // normal single keys
                }
            }
            usb_arm_out_endpoint(2);
                    
		}
        else enable_flags &= ~FLAG_USB;
	}
    else enable_flags &= ~(FLAG_USB_PRES | FLAG_USB);
}


/* Callbacks. These function names are set in usb_config.h. */

void app_set_configuration_callback(uint8_t configuration) {
}

uint16_t app_get_device_status_callback(void) {
	return 0x0000;
}

void app_endpoint_halt_callback(uint8_t endpoint, bool halted) {
}

int8_t app_set_interface_callback(uint8_t interface, uint8_t alt_setting) {
	return 0;
}

int8_t app_get_interface_callback(uint8_t interface) {
	return 0;
}

void app_out_transaction_callback(uint8_t endpoint) {
}

void app_in_transaction_complete_callback(uint8_t endpoint) {
}

int8_t app_unknown_setup_request_callback(const struct setup_packet *setup) {
	/* To use the CDC device class, have a handler for unknown setup
	 * requests and call process_cdc_setup_request() (as shown here),
	 * which will check if the setup request is CDC-related, and will
	 * call the CDC application callbacks defined in usb_cdc.h. For
	 * composite devices containing other device classes, make sure
	 * MULTI_CLASS_DEVICE is defined in usb_config.h and call all
	 * appropriate device class setup request functions here.
	 */
	return process_cdc_setup_request(setup);
}

int16_t app_unknown_get_descriptor_callback(const struct setup_packet *pkt, const void **descriptor) {
	return -1;
}

void app_start_of_frame_callback(void) {
}

void app_usb_reset_callback(void) {
}

/* CDC Callbacks. See usb_cdc.h for documentation. */

int8_t app_send_encapsulated_command(uint8_t interface, uint16_t length) {
	return -1;
}

int16_t app_get_encapsulated_response(uint8_t interface, uint16_t length, const void **report,
                                      usb_ep0_data_stage_callback *callback, void **context) {
	return -1;
}

int8_t app_set_comm_feature_callback(uint8_t interface, bool idle_setting, bool data_multiplexed_state) {
	return -1;
}

int8_t app_clear_comm_feature_callback(uint8_t interface, bool idle_setting, bool data_multiplexed_state) {
	return -1;
}

int8_t app_get_comm_feature_callback(uint8_t interface, bool *idle_setting, bool *data_multiplexed_state) {
	return -1;
}

static struct cdc_line_coding line_coding = {
	38400,
	CDC_CHAR_FORMAT_1_STOP_BIT,
	CDC_PARITY_NONE,
	8,
};

int8_t app_set_line_coding_callback(uint8_t interface, const struct cdc_line_coding *coding) {
	line_coding = *coding;
	return 0;
}

int8_t app_get_line_coding_callback(uint8_t interface, struct cdc_line_coding *coding) {
	/* This is where baud rate, data, stop, and parity bits are set. */
	*coding = line_coding;
	return 0;
}

int8_t app_set_control_line_state_callback(uint8_t interface, bool dtr, bool dts) {
	return 0;
}

int8_t app_send_break_callback(uint8_t interface, uint16_t duration) {
	return 0;
}


// TIMING =======================================================================================

// interrupt handler for the 1 ms timer
void __ISR(_TIMER_4_VECTOR, IPL5AUTO) T4Interrupt(void) {
    msClock++;
    if(msClock % 1000 == 0) {
        sUTime++;
        if(enable_flags & FLAG_RTC_UPDATE) {
            enable_flags &= ~FLAG_RTC_UPDATE;
            struct tm *tt = localtime((const time_t *) &sUTime);
            i2cSetTime(tt);
        }
    }
    if(msCountdown) (msCountdown)--;
    if(msKbdTimer) msKbdTimer--;
    if(msK && (--msK == 0)) addKey();
    doUSB();
    mT4ClearIntFlag();  // clear the interrupt flag
}


clock_t clock(void) {
    return (clock_t) msClock;
}


time_t time(time_t *t) {
    struct tm tt;
    if(i2cGetTime(&tt)) sUTime = mktime(&tt);
    if(t) *t = (time_t) sUTime;
    return (time_t) sUTime;
}


// PS/2 KEYBOARD ================================================================================

#define P_PS2CLK		PORTBbits.RB5       // clock
#define P_PS2CLK_TRIS   TRISBbits.TRISB5    // tris
#define P_PS2CLK_OUT    LATBbits.LATB5      // output latch for the clock

#define P_PS2DAT		PORTBbits.RB7		// data
#define P_PS2DAT_TRIS   TRISBbits.TRISB7    // tris
#define P_PS2DAT_OUT    LATBbits.LATB7      // output latch for the data

#define P_PS2_INTERRUPT(a)  mINT3IntEnable(a)   // ensble/disable INT3 keyboard interrupt

#define true        1
#define false       0

// definitions for the keyboard PS/2 state machine
#define PS2START    0
#define PS2BIT      1
#define PS2PARITY   2
#define PS2STOP     3

#define FLAG_SCROLLLOCK (1 << 0)
#define FLAG_NUMLOCK    (1 << 1)
#define FLAG_CAPSLOCK   (1 << 2)

int PS2State, KState, KCount, KParity;
unsigned char KBDBuf;


// send a command to to keyboard.
// NOTE: when we want to signal a logic high we turn the pin into an input which means that the
// pull-up will ensure a high.  When we want to signal a logic low we turn the pin into an output
// and because we have loaded the output latch with a zero the output will be pulled low
void sendCommand(int cmd) {
    int i, j;

    // calculate the parity and add to the command as the 9th bit
    for(j = i = 0; i < 8; i++) j += ((cmd >> i) & 1);
    cmd = (cmd & 0xff) | (((j + 1) & 1) << 8);

 	P_PS2CLK_OUT = 0;                                               // when configured as an output the clock line will be low
 	P_PS2DAT_OUT = 0;                                               // same for data
 	P_PS2CLK_TRIS = 0;                                              // clock low
 	uSec(150);
 	P_PS2DAT_TRIS = 0; uSec(2);                                     // data low
 	P_PS2CLK_TRIS = 1; uSec(2);                                     // release the clock (goes high)
 	msKbdTimer = 500;                                               // timeout of 500mS
 	while(P_PS2CLK) if(msKbdTimer == 0) return;                     // wait for the keyboard to pull the clock low

 	// send each bit including parity
 	for(i = 0; i < 9; i++) {
        P_PS2DAT_TRIS = (cmd & 1);                                  // set the data bit
     	while(!P_PS2CLK) if(msKbdTimer == 0) return;                // wait for the keyboard to bring the clock high
     	while(P_PS2CLK)  if(msKbdTimer == 0) return;                // wait for clock low
     	cmd >>= 1;
    }

    P_PS2DAT_TRIS = 1;                                              // release the data line
    while(P_PS2DAT) if(msKbdTimer == 0) return;                     // wait for the keyboard to pull data low (ACK)
 	while(P_PS2CLK) if(msKbdTimer == 0) return;                     // wait for the clock to go low
 	while(!P_PS2CLK || !P_PS2DAT) if(msKbdTimer == 0) return;       // finally wait for both the clock and data to go high (idle state)
    enable_flags |= FLAG_PS2;   // the keyboard responded
}


// set the keyboard LEDs
// [.0] Scroll; [.1] Num; [.2] Caps
void setKbdLEDs(uint8_t flags) {
    P_PS2_INTERRUPT(false); // disable interrupt while we play
    sendCommand(0xED);      // Set/Reset Status Indicators Command
    uSec(50000);
    sendCommand(flags & 7); // set the various LEDs
    P_PS2CLK_TRIS = 1; P_PS2DAT_TRIS = 1;   // reset the data & clock to inputs in case a keyboard was not plugged in
    uSec(5000);
    P_PS2_INTERRUPT(true);  // re enable interrupt
}


// initialise the keyboard
void initKeyboard(void) {
    P_PS2CLK_TRIS = 1; P_PS2DAT_TRIS = 1;   // initialise the lines as input
    PPSInput(2, INT3, RPB5);
    ConfigINT3(EXT_INT_PRI_2 | FALLING_EDGE_INT | EXT_INT_ENABLE);

    // initialise variables and set the LEDs
    PS2State = PS2START;
    keyDown = EOF;
    //kbdFlags &= ~FLAG_SCROLLLOCK;  
    //kbdFlags &= ~FLAG_NUMLOCK;
    //kbdFlags &= ~FLAG_CAPSLOCK;  
    setKbdLEDs(kbdFlags);
    if((enable_flags & FLAG_PS2) == 0) ConfigINT3(EXT_INT_DISABLE);
}    


// PS/2 keyboard interrupt handler
void __ISR(_EXTERNAL_3_VECTOR, IPL2AUTO) INT3Interrupt(void) {
	unsigned int c = 0;
	static char LShift = 0;
	static char RShift = 0;
	static char Ctrl = 0;
	static char AltGrDown = 0;
    static char Dead = 0;
	static char KeyUpCode = false;
	static char KeyE0 = false;
	static unsigned char Code = 0;
    if(P_PS2CLK == 0) {     // make sure it was a falling edge
        enable_flags |= FLAG_PS2;
	    char d = P_PS2DAT;  // sample the data
        switch(PS2State) {

            default:
            case PS2START:
                if(!d) {                					// PS2DAT == 0
                    KCount = 8;         					// init bit counter
                    KParity = 0;        					// init parity check
                    Code = 0;
                    PS2State = PS2BIT;
                }
                break;

            case PS2BIT:
                Code >>= 1;            						// shift in data bit
                if(d) Code |= 0x80;                			// PS2DAT == 1
                KParity ^= Code;      						// calculate parity
                if(--KCount <= 0) PS2State = PS2PARITY;   	// all bit read
                break;

            case PS2PARITY:
                if(d) KParity ^= 0x80;                		// PS2DAT == 1
                if (KParity & 0x80) PS2State = PS2STOP;		// parity odd, continue
                else PS2State = PS2START;
                break;

            case PS2STOP:
                if(d) {                 					// PS2DAT == 1
	                if(Code == 0xAA) setKbdLEDs(kbdFlags);  // self test code (a keyboard must have just been plugged in)
	                else if(Code == 0xF0) KeyUpCode = true; // a key has been released
	                else if(Code == 0xE0) KeyE0 = true;     // extended keycode prefix
	                else {  // Process a scan code from the keyboard into an ASCII character

                        // if a key has been released we are only interested in resetting state keys
                        if(KeyUpCode) {
                            if(Code == L_SHFT) LShift = 0;                  // left shift button is released
                            else if(Code == R_SHFT) RShift = 0;             // right shift button is released
                            else if(Code == CTRL) Ctrl = 0;			        // left control button is released
                            else if(KeyE0 && Code == ALTGR) AltGrDown = 0;  // release the AltGr key on non US keyboards
                            else keyDown = EOF;
                            goto SkipOut;
                        }

                        // we are only here if the key has been pressed (NOT released)
                        if(Code == L_SHFT) { LShift = 1; goto SkipOut; }    // left shift button is pressed
                        if(Code == R_SHFT) { RShift = 1; goto SkipOut; }	// right shift button is pressed
                        if(Code == CTRL) { Ctrl = 1; goto SkipOut; }		// left control button is pressed
                        if(Code == CAPS) {                                  // caps or num lock pressed
                            kbdFlags ^= FLAG_CAPSLOCK;
                            setKbdLEDs(kbdFlags);
                            goto SkipOut;
                        }
                        if(Code == NUML) {                                  // caps or num lock pressed
                            kbdFlags ^= FLAG_NUMLOCK;
                            setKbdLEDs(kbdFlags);
                            goto SkipOut;
                        }
                        if(KeyE0 && Code == ALTGR) { AltGrDown = 1; goto SkipOut; } // AltGr key pressed

                        // now get the character into c.  Why, oh why, are scan codes so random?
                        if(AltGrDown) {    // AltGr key code corrections
                            if(settings.kbd_layout == LAYOUT_US) {  // US International
                                switch(Code) {
                                    case 0x06: c = (LShift || RShift) ?      0 : '\xAD'; break;
                                    case 0x1E: c = (LShift || RShift) ?      0 : '\xFD'; break;
                                    case 0x25: c = (LShift || RShift) ?      0 : '\x9C'; break;
                                    case 0x26: c = (LShift || RShift) ?      0 : '\x9E'; break;
                                    case 0x36: c = (LShift || RShift) ?      0 : '\xAC'; break;
                                    case 0x3D: c = (LShift || RShift) ?      0 : '\xAB'; break;
                                    case 0x4E: c = (LShift || RShift) ?      0 : '\x9D'; break;
                                    case 0x55: c = (LShift || RShift) ?      0 : '\xF6'; break;
                                    case 0x5D: c = (LShift || RShift) ?    '|' : '\xAA'; break;
                                    case 0x54: c = (LShift || RShift) ?      0 : '\xAE'; break;
                                    case 0x5B: c = (LShift || RShift) ?      0 : '\xAF'; break;
                                    case 0x4C: c = (LShift || RShift) ? '\x14' : '\xF8'; break;
                                    case 0x15: c = (LShift || RShift) ? '\x8E' : '\x84'; break;
                                    case 0x1D: c = (LShift || RShift) ? '\x8F' :      0; break;
                                    case 0x35: c = (LShift || RShift) ? '\x9A' : '\x81'; break;
                                    case 0x3C: c = (LShift || RShift) ?      0 : '\xA3'; break;
                                    case 0x43: c = (LShift || RShift) ?      0 : '\xA1'; break;
                                    case 0x44: c = (LShift || RShift) ?      0 : '\xA2'; break;
                                    case 0x4D: c = (LShift || RShift) ?      0 : '\x94'; break;
                                    case 0x1C: c = (LShift || RShift) ?      0 : '\xA0'; break;
                                    case 0x1B: c = (LShift || RShift) ? '\xE8' : '\xE1'; break;
                                    case 0x4B: c = (LShift || RShift) ? '\xED' :      0; break;
                                    case 0x1A: c = (LShift || RShift) ? '\x92' : '\x91'; break;
                                    case 0x21: c = (LShift || RShift) ? '\x9B' :      0; break;
                                    case 0x31: c = (LShift || RShift) ? '\xA5' : '\xA4'; break;
                                    case 0x3A: c = (LShift || RShift) ?      0 : '\xE6'; break;
                                    case 0x41: c = (LShift || RShift) ? '\x80' : '\x87'; break;
                                    case 0x4A: c = (LShift || RShift) ?      0 : '\xA8'; break;
                                    default:   c = 0; break;
                                }
                            }
                            else if(settings.kbd_layout == LAYOUT_UK) { // UK Extended
                                switch(Code) {
                                    case 0x0E: c = (LShift || RShift) ?      0 :    '|'; break;
                                    case 0x25: c = (LShift || RShift) ?      0 : '\x9E'; break;
                                    case 0x24: c = (LShift || RShift) ? '\x90' : '\x82'; break;
                                    case 0x3C: c = (LShift || RShift) ?      0 : '\xA3'; break;
                                    case 0x43: c = (LShift || RShift) ?      0 : '\xA1'; break;
                                    case 0x44: c = (LShift || RShift) ?      0 : '\xA2'; break;
                                    case 0x1C: c = (LShift || RShift) ?      0 : '\xA0'; break;
                                    case 0x21: c = (LShift || RShift) ? '\x80' : '\x87'; break;
                                    default:   c = 0; break;
                                }
                            }
                            else if(settings.kbd_layout == LAYOUT_DE) { // German
                                switch(Code) {
                                    case 0x1E: c = (LShift || RShift) ?      0 : '\xFD'; break;
                                    case 0x26: c = (LShift || RShift) ?      0 : '\xFC'; break;
                                    case 0x3D: c = (LShift || RShift) ?      0 :    '{'; break;
                                    case 0x3E: c = (LShift || RShift) ?      0 :    '['; break;
                                    case 0x46: c = (LShift || RShift) ?      0 :    ']'; break;
                                    case 0x45: c = (LShift || RShift) ?      0 :    '}'; break;
                                    case 0x4E: c = (LShift || RShift) ?      0 :   '\\'; break;
                                    case 0x05: c = (LShift || RShift) ?      0 :    '@'; break;
                                    case 0x24: c = (LShift || RShift) ?      0 : '\x9E'; break;
                                    case 0x5B: c = (LShift || RShift) ?      0 :    '~'; break;
                                    case 0x61: c = (LShift || RShift) ?      0 :    '|'; break;
                                    case 0x3A: c = (LShift || RShift) ?      0 : '\xE6'; break;
                                    default:   c = 0; break;
                                }
                            }
                            else if(settings.kbd_layout == LAYOUT_FR) { // French
                                switch(Code) {
                                    case 0x1E: c = (LShift || RShift) ?      0 :    '~'; break;
                                    case 0x26: c = (LShift || RShift) ?      0 :    '#'; break;
                                    case 0x25: c = (LShift || RShift) ?      0 :    '{'; break;
                                    case 0x2E: c = (LShift || RShift) ?      0 :    '['; break;
                                    case 0x36: c = (LShift || RShift) ?      0 :    '|'; break;
                                    case 0x3D: c = (LShift || RShift) ?      0 :    '`'; break;
                                    case 0x3E: c = (LShift || RShift) ?      0 :   '\\'; break;
                                    case 0x46: c = (LShift || RShift) ?      0 :    '^'; break;
                                    case 0x45: c = (LShift || RShift) ?      0 :    '@'; break;
                                    case 0x4E: c = (LShift || RShift) ?      0 :    ']'; break;
                                    case 0x55: c = (LShift || RShift) ?      0 :    '}'; break;
                                    case 0x24: c = (LShift || RShift) ?      0 : '\x9E'; break;
                                    default:   c = 0; break;
                                }
                            }
                            
                        }
                        else {
                            if(Code == 0x83) { c = KEY_F7; goto SkipIn; }       // special case - F7
                            else if(Code == 0x78) { c = KEY_F11; goto SkipIn; } // special case - F11
                            else if(KeyE0 && Code == 0x4A) { c = '/'; goto SkipIn; }     // special case - keypad forward slash
                            else if(KeyE0 && Code == 0x5A) { c = KEY_ENT; goto SkipIn; } // special case - keypad enter
                            else if(Code >= 0x68 && Code < 0x80 &&
                                    (KeyE0 || (kbdFlags & FLAG_NUMLOCK) == 0)) {    // a key code from the numeric keypad
                                LShift = 0;     // when num lock LED is on codes are preceeded by left shift
                                if(Code == 0x76) c = KEY_ESC;           // special case - Esc
                                else c = (MULPS2 << 8) + Code;
                                goto SkipIn;
                            }
                            if(!Dead) {
                                if(settings.kbd_layout == LAYOUT_DE) {
                                    if(Code == 0x55) Dead = 1 + !!(LShift || RShift);   // / \ accents
                                    if(Code == 0x0E) Dead = 3;                          // ^
                                }
                                if(settings.kbd_layout == LAYOUT_FR) {
                                    if(Code == 0x54) Dead = 3 + !!(LShift || RShift);   // ^ or umlaut
                                }
                                if(Dead) goto SkipOut;
                            }
                            c = keyCodes[(settings.kbd_layout * 2) + !!(LShift || RShift)][Code % 128];
                            if(kbdFlags & FLAG_CAPSLOCK) {  // adjust for caps lock
                                if(c >= 'a' && c <= 'z') c -= 32;
                                else if(c >= 'A' && c <= 'Z') c += 32;
                            }
                            if(Dead && c) { // dead keys
                                switch(Dead) {  
                                    case 1: {   // /-accent
                                        if(c == 'a') c = '\x86';
                                        if(c == 'e') c = '\x82';
                                        if(c == 'i') c = '\xA1';
                                        if(c == 'o') c = '\xA2';
                                        if(c == 'u') c = '\xA3';
                                        if(c == 'E') c = '\x90';
                                        break;
                                    }
                                    case 2: {   // \-accent
                                        if(c == 'a') c = '\x85';
                                        if(c == 'e') c = '\x8A';
                                        if(c == 'i') c = '\x8D';
                                        if(c == 'o') c = '\x95';
                                        if(c == 'u') c = '\x97';
                                        break;
                                    }
                                    case 3: {   // ^
                                        if(c == 'a') c = '\x83';
                                        if(c == 'e') c = '\x88';
                                        if(c == 'i') c = '\x8C';
                                        if(c == 'o') c = '\x93';
                                        if(c == 'u') c = '\x96';
                                        if(c == 'A') c = '\x8F';
                                        break;
                                    }
                                    case 4: {   // umlaut
                                        if(c == 'a') c = '\x84';
                                        if(c == 'e') c = '\x89';
                                        if(c == 'i') c = '\x8B';
                                        if(c == 'o') c = '\x94';
                                        if(c == 'u') c = '\x81';
                                        if(c == 'y') c = '\x96';
                                        if(c == 'A') c = '\x8E';
                                        if(c == 'O') c = '\x99';
                                        if(c == 'U') c = '\x9A';
                                        break;
                                    }
                                    default: break;
                                }
                                Dead = 0;
                            }
                        }
                        if(!c) goto SkipOut;
                        
                        SkipIn:
                        if(Ctrl) c &= 0x1F; // control characters
    				    keyDown = c;
                        
                        SkipOut:
	                	KeyUpCode = KeyE0 = false;
	                }
                    Code = 0;
                }
                PS2State = PS2START;
                break;
	    }
	}
    mINT3ClearIntFlag();    // clear the interrupt flag
}


// get character from the console without removing it from the buffer
// return the current character from the input buffer, or EOF in case the buffer is empty
__attribute__ ((used)) int kbhit(void) {
    if(getch_ch) return getch_ch;
    int ch = _mon_getc(0);
    if(ch == EOF) ch = 0;
    return ch;
}


// getch() hook
__attribute__ ((used)) char getch(void) {
    if(getch_ch == 0) {
        getch_ch = _mon_getc(1);
        is_big = (getch_ch & 0xFFFFFF00); 
    }    
    while(getch_ch && (getch_ch & 0xFF000000) == 0) getch_ch <<= 8;
    int r = getch_ch >> 24; getch_ch <<= 8;
    return (r & 0xFF);
}


// get character from the console
// (blocking) non-zero indicates that the function must be blocking and wait for character
// return the current character from the input buffer, or EOF in case the buffer is empty
__attribute__ ((used)) int _mon_getc(int blocking) {
    static int cKey = EOF;
	int ch = EOF;
    if(blocking) drawChar(0);   // clear the key buffer and draw cursor
    do {
        if(enable_flags & (FLAG_SERIAL | FLAG_USB)) {
            if(conRx_in != conRx_out) {
                ch = conBuffer[conRx_out];
                if(blocking) {
                    if(++conRx_out >= CON_BUFFER_SIZE) conRx_out = 0;
                }
                break;
            }
        }
        if(enable_flags & FLAG_PS2) {
            ch = keyDown;
            if(ch != EOF) {
                if((ch & 0xFF000000) == 0xFF000000) ch &= ~0xFF000000;
                if((ch & 0xFF0000) == 0xFF0000) ch &= ~0xFF0000;
                if((ch & 0xFF00) == 0xFF00) ch &= ~0xFF00;
            }
            if(blocking) {
                if(ch != EOF && ch != cKey) { cKey = keyDown; keyDown = EOF; }
                else cKey = ch;
            }
            else ch = keyDown;
        }
    } while(blocking && ch == EOF);
    if(ch > EOF && ch < 0x100 && ch != settings.brk_code) {
        if(enable_flags & FLAG_VIDEO) {
            if((enable_flags & FLAG_NO_ECHO) == 0) printf("%c", ch);
        }
        else printf("%c", ch);
    }
    return ch;
}


// VIDEO GENERATOR AND HARDWARE-DEPENDENT BASIC VIDEO FUNCTIONS =================================

// video parameter indexes (the first dimension in the array is the video mode):
#define VGA_HRES	 	0   // horizontal resolution in pixels
#define VGA_VRES	 	1   // vertical resolution in pixels
#define VGA_SCAN_DIV    2   // pixel clock divider; (_PB_FREQ) divided by this value produces the scanning pixel clock
#define VGA_LINE_NS   	3   // (H Period) Time in ns for a full horizontal line
#define VGA_HSYNC_PIX  	4   // (H Sync) Pulse width in number of pixels
#define VGA_VFRPORCH_LN 5   // (V Front Porch) Number of blank lines at the top of the screen
#define VGA_VSYNC_LN    6   // (V Sync) Number of V-sync lines
#define VGA_VBKPORCH_LN 7   // (V Back Porch) Number of blank lines at the bottom of the screen

// table with parameters for the supported video modes
unsigned int VideoParams[1][8] = {
    { 480, 320, 3, 33297, 228, 48, 25, 28 }
};

#define SV_PREEQ    0   // generating blank lines before the vertical sync
#define SV_SYNC     1   // generating the vertical sync
#define SV_POSTEQ   2   // generating blank lines after the vertical sync
#define SV_LINE     3   // visible lines, send the video data out

int VC[4], VS[4] = { SV_SYNC, SV_POSTEQ, SV_LINE, SV_PREEQ };   // state machine tables
volatile uint8_t VState = SV_PREEQ;
volatile uint32_t *VPtr;
uint16_t Hwords32;  // number of 32-bit words needed for one horizontal line of video


// initialise the video output in specified video mode
void initVideo(uint8_t mode) {
    mT3IntEnable(0);
    CloseTimer3();
    CloseOC3();
    SpiChnClose(2);
    DmaChnEnable(0);
    VidMemSize = 0;
    Hres = Vres = 1;
    x_free((byte **) &VidMem); VidMem = NULL;
    x_defrag();
    SysMemSize = x_avail();
    enable_flags &= ~FLAG_VIDEO;
   
    /* set the video mode parameters */
    VpageAddr = 0;
    Vmode = mode;
    VC[0] = VideoParams[mode][VGA_VSYNC_LN];
    VC[1] = VideoParams[mode][VGA_VBKPORCH_LN];
    VC[2] = VideoParams[mode][VGA_VRES];
    VC[3] = VideoParams[mode][VGA_VFRPORCH_LN];
    Hwords32 = (VideoParams[mode][VGA_HRES] + 31) / 32;
    
    // detect presence of a monitor
    CNPUASET = (1 << 4); TRISASET = (1 << 4);           // enable the internal pull-up
    if(PORTA & BIT(4)) { CNPUACLR = (1 << 4); return; } // no monitor detected
    CNPUACLR = (1 << 4);
    VidMemSize = (Hwords32 * 4 * VideoParams[mode][VGA_VRES]);
    x_malloc((byte **) &VidMem, VidMemSize);
    if(!VidMem) { VidMemSize = 0; return; }
    VPtr = (uint32_t *) &VidMem[VpageAddr];
    SysMemSize = x_avail();
    enable_flags |= FLAG_VIDEO;
    Hres = VideoParams[mode][VGA_HRES];
    Vres = VideoParams[mode][VGA_VRES];
   
    // setup SPI2 as the video generator.  The output of this module is a stream of bits which are the pixels in a horizontal scan line
    PPSOutput(3, RPA4, SDO2);   // A4 is the video out for VGA
    PPSInput(4, SS2, RPA3);     // A3 is the framing input

    // the vertical sync uses B4
    TRISBCLR = (1 << 4);        // Vertical sync output used by VGA
    #define P_VERT_SET_HI   LATBSET = (1 << 4)  // set vertical sync hi
    #define P_VERT_SET_LO   LATBCLR = (1 << 4)  // set vertical sync lo

    // the horizontal sync uses Timer 3 and Output Compare 3 to generate the pulse and interrupt level 7
    PPSOutput(4, RPB0, OC3);    // B0 is the horizontal sync output (ie, the output from OC3)
    OpenOC3((OC_ON | OC_TIMER3_SRC | OC_CONTINUE_PULSE), 0, VideoParams[mode][VGA_HSYNC_PIX]);  // OC3 is used to time the width of H-sync pulse
    OpenTimer3((T3_ON | T3_PS_1_1 | T3_SOURCE_INT), (((_PB_FREQ / 1E4) * VideoParams[mode][VGA_LINE_NS]) / 1E5) - 1);  // Timer 3 sets the  frequency
    mT3SetIntPriority(7);
    mT3IntEnable(1);    // set priority level 7 for the timer 3 interrupt (H-synch) and enable it

    // setup the SPI channel then DMA channel which will copy the memory bitmap buffer to the SPI channel
    // open the SPI in framing mode.  Note that SPI_OPEN_DISSDI will disable the input (which we do not need)
    SpiChnOpen(2, (SPICON_ON | SPICON_MSTEN | SPICON_MODE32 | SPICON_FRMEN | SPICON_FRMSYNC | SPICON_FRMPOL | SPI_OPEN_DISSDI), VideoParams[mode][VGA_SCAN_DIV]);
    SPI2CON2bits.IGNROV = 1; SPI2CON2bits.IGNTUR = 1;   // instruct the SPI module to ignore any errors that might occur
    DmaChnOpen(1, 3, DMA_OPEN_DEFAULT);     // setup DMA 1 to send data to SPI channel 2
    DmaChnSetEventControl(1, (DMA_EV_START_IRQ_EN | DMA_EV_START_IRQ(_SPI2_TX_IRQ)));
    DmaChnSetTxfer(1, (void *) &VidMem[VpageAddr], (void *) &SPI2BUF, (Hwords32 * 4), 4, 4);
}


// video output with Timer 3 interrupt handler
void __ISR(_TIMER_3_VECTOR, IPL7AUTO) T3Interrupt(void) {
    static unsigned int VCount = 1;
    if(--VCount == 0) {         // count down the number of lines
        VCount = VC[VState];    // set the new count
        VState = VS[VState];    // and advance the state machine
        if(VState == SV_PREEQ) {
            SPI2BUF = 0;
            VPtr = (uint32_t *) &VidMem[VpageAddr];
        }
        else if(VState == SV_POSTEQ) P_VERT_SET_HI; 
        else if(VState == SV_SYNC) P_VERT_SET_LO;
    }
    if(VState == SV_LINE) {     // shift out video data
        SPI2BUF = 0;            // initially load the SPI buffer with 4 zero bytes to pad the start of the video
        DCH1SSA = KVA_TO_PA((void *) (VPtr));   // update the DMA1 source address (DMA1 is used for VGA data)
        VPtr += Hwords32;       // move the pointer to the start of the next line
        DmaChnEnable(1);        // arm the DMA transfer
    }
    mT3ClearIntFlag();          // clear the interrupt flag
}


// clear the entire screen
void clearScreen(int c) {
    if(enable_flags & FLAG_VIDEO) {
        uint32_t *a = (uint32_t *) &VidMem[VpageAddr];
        if(c == COL_TRANSP) c = 0;
        if(c == COL_INVERT) {
            uint32_t t = (Hwords32 * VideoParams[Vmode][VGA_VRES]);
            while(t--) *(a++) ^= -1ul;
        }
        else {
            if(c) c = -1;   // fill all bits with 1
            memset(a, c, (Hwords32 * 4 * VideoParams[Vmode][VGA_VRES]));
        }
        posX = (Hres % (font->header.blankL + font->header.width + font->header.blankR)) / 2;
        posY = (Vres % (font->header.blankT + font->header.height + font->header.blankB)) / 2;
    }
    else {
        int i;
        printf("\r");
        for(i = 0; i < 100; i++) printf("\n");
    }
}


// scroll the screen up by defined number of lines; the bottom part is filled with given colour
void scrollUp(int vl, int c) {
    if(enable_flags & FLAG_VIDEO) {
        if(vl > Vres) vl = Vres;
        if(c == COL_TRANSP) c = 0;
        memmove(&VidMem[VpageAddr], &VidMem[VpageAddr + (vl * 4 * Hwords32)], ((Vres - vl) * 4 * Hwords32));
        memset(&VidMem[VpageAddr + ((Vres - vl) * 4 * Hwords32)], c, (vl * 4 * Hwords32));
    }
    else printf("\r\n");
}


// return the colour of a pixel at (x,y)
int getPixel(int x, int y) {
    if(x >= 0 && x < Hres && y >= 0 && y < Vres && (enable_flags & FLAG_VIDEO)) {
        uint32_t *a = (uint32_t *) (&VidMem[VpageAddr] + (((y * Hwords32) + (x >> 5)) << 2));
        return !!(*a & (0x80000000 >> (x & 31)));
    }
    else return 0;
}


// set a single pixel at coordinates (x,y)
void setPixel(int x, int y, int c) {
    if(x >= 0 && x < Hres && y >= 0 && y < Vres && (enable_flags & FLAG_VIDEO)) {
        uint32_t *a = (uint32_t *) (&VidMem[VpageAddr] + (((y * Hwords32) + (x >> 5)) << 2));
        if(c == COL_GREY50) c = (((x & 1) && (y & 1) == 0) || ((x & 1) == 0 && (y & 1))) ? COL_INVERT : 0;
        else if(c == COL_GREY25) c = (x % 4 == 0 && y % 4 == 2) ? COL_INVERT : 0;
        else if(c == COL_HLN50) c = (y & 1) ? COL_INVERT : 0;
        else if(c == COL_HLN33) c = (y % 3 == 0) ? COL_INVERT : 0;
        else if(c == COL_HLN25) c = (y % 4 == 0) ? COL_INVERT : 0;
        else if(c == COL_VLN50) c = (x & 1) ? COL_INVERT : 0;
        else if(c == COL_VLN33) c = (x % 3 == 0) ? COL_INVERT : 0;
        else if(c == COL_VLN25) c = (x % 4 == 0) ? COL_INVERT : 0;
        else if(c == COL_DGL33) c = ((x + y) % 4 == 0) ? COL_INVERT : 0;
        else if(c == COL_DGL25) c = ((x + y) % 6 == 0) ? COL_INVERT : 0;
        else if(c == COL_DGR33) c = ((x - y) % 4 == 0) ? COL_INVERT : 0;
        else if(c == COL_DGR25) c = ((x - y) % 6 == 0) ? COL_INVERT : 0;
        else if(c == COL_SPL33) c = ((x % 3) + (y % 3) == 1) ? COL_INVERT : 0;
        else if(c == COL_SPL25) c = ((x % 5) + (y % 5) == 1) ? COL_INVERT : 0;
        else if(c == COL_SPR33) c = ((x % 3) - (y % 3) == 1) ? COL_INVERT : 0;
        else if(c == COL_SPR25) c = ((x % 5) - (y % 5) == 1) ? COL_INVERT : 0;
        else if(c >= -50 && c <= -30) c = !!((rand() % 100) < (-(c + 30) * 5));
        if(c > 0) *a |= (0x80000000 >> (x & 31));           // turn the pixel on
        else if(c == 0) *a &= ~(0x80000000 >> (x & 31));    // turn the pixel off
        else if(c == COL_INVERT) *a ^= (0x80000000 >> (x & 31));    // invert the pixel
    }                                                       // else: do not draw (transparent pixel)
}


__attribute__ ((used)) void _mon_putc(char ch) {
    if(ch) {
        if(enable_flags & FLAG_SERIAL) {
            uint16_t bitp = (1000000 / SERIAL_BAUDRATE);
            char i = 8, b = ch;
            if(enable_flags & FLAG_VIDEO) { // synchronise with VSYNC in order to avoid garbled serial output
                while(VState == SV_PREEQ); 
            }
            INTEnable(INT_U2RX, INT_DISABLED);
            mT4IntEnable(0);    // disable the millisecond timer
            SERIAL_TX = 0; uSec(bitp);  // start bit
            while(i--) {                // data bits
                SERIAL_TX = (b & 1); b >>= 1;
                uSec(bitp);
            }
            SERIAL_TX = 1; uSec(bitp);  // stop bit
            mT4IntEnable(1);    // enable again the millisecond timer
            INTEnable(INT_U2RX, INT_ENABLED);
        }
        if(enable_flags & FLAG_USB) {
            int z = usbTx_in;
            if(++z >= CON_BUFFER_SIZE) z = 0;
            if(z == usbTx_out) {    // wait to flush the buffer when it is full
                int zz = CON_BUFFER_SIZE;
                while((enable_flags & FLAG_USB) && usbTx_in != usbTx_out && zz--) {
                    mT4IntEnable(0);    // disable the millisecond timer
                    doUSB();
                    mT4IntEnable(1);    // enable again the millisecond timer
                }
            }
            usbTxBuf[usbTx_in] = ch;
            if(++usbTx_in >= CON_BUFFER_SIZE) usbTx_in = 0;
        }
    }
    if(enable_flags & FLAG_VIDEO) drawChar((unsigned char) ch);
}


__attribute__ ((used)) void _mon_puts(const char *s) {
    while(s && *s) _mon_putc((int) *(s++));
}


// SYSTEM SPI AND FILE SYSTEM ===================================================================

/* not completely sure why but the compiler wants this definition... */
int open (const char *buf, int flags, int mode) { return 0; }


/* open SPI channel */
/* opening with speed 0 will close the port */
/* bits can be 8, 16, or 32 bits in a SPI word */
/* will return 0 if it has been successfully executed; 1 otherwise */
int openSPI(char channel, unsigned char spi_mode, unsigned char bits, unsigned long speed) {
    int res = 0;
    if(speed) {
        SpiOpenFlags f = (SPI_OPEN_ON | SPI_OPEN_MSTEN);
        if(bits == 32) f |= SPI_OPEN_MODE32;
        else if(bits == 16) f |= SPI_OPEN_MODE16;
        else f |= SPI_OPEN_MODE8;   /* 8 bits data word length by default */
        spi_mode &= 3;  /* only four SPI modes are possible */
        if(spi_mode == 0) f |= SPI_OPEN_CKE_REV;
        else if(spi_mode == 3) f |= SPI_OPEN_CKP_HIGH;
        else if(spi_mode == 2) f |= (SPI_OPEN_CKP_HIGH | SPI_OPEN_CKE_REV);
        if(channel == SPI_CHANNEL1) {
            PPSInput(2, SDI1, RPB1);
            PPSOutput(3, RPB13, SDO1);
            CNPUBSET = (BIT(1) | BIT(13) | BIT(14));    // enable internal port pull-up resistors on the SPI lines
        }
        else res = -1;  /* invalid channel */
        SpiChnOpen((SpiChannel) channel, f, (_PB_FREQ / speed));
        SpiChnEnable((SpiChannel) channel, TRUE);
    }
    else {
        SpiChnEnable((SpiChannel) channel, FALSE);
        if(channel == SPI_CHANNEL1) {
            PPSOutput(3, RPB13, NULL);
            PPSOutput(4, RPB14, NULL);
            PORTResetPins(IOPORT_B, (BIT(1) | BIT(13) | BIT(14)));
        }
        else res = -1;  /* invalid channel */
    }
    return res;
}


/* transmit/receive data to/from SPI port */
unsigned long xchgSPI(char channel, unsigned long tx_data) {
    if(SDdlyCntr > 0) SDdlyCntr--;
    unsigned long rx_data = 0;
    if(channel >= 0) {
        SpiChnGetRov((SpiChannel) channel, TRUE);
        SpiChnPutC((SpiChannel) channel, tx_data);  /* transmit data */
        rx_data = SpiChnGetC((SpiChannel) channel); /* receive data */
    }
	return rx_data;
}


// SPECIAL VERSION OF NVMProgram() ADAPTED FOR PIC32MX1xx/2xx ===================================

unsigned int NVMProgramMX1(unsigned char *address, unsigned char *data, unsigned int size, unsigned char *pagebuff) {
    uintptr_t pageStartAddr;
    unsigned int numBefore, numAfter, numLeftInPage;
    unsigned int index;

	if((size & 3) || ((uintptr_t) pagebuff & 3)) return 1;          // 1. make sure that the size and pagebuff are word aligned
	if(size == 0) return 0;     // nothing to program
    
    pageStartAddr = (uintptr_t) address & (~(BYTE_PAGE_SIZE - 1));  // 2. calculate Page Start address
	numBefore = (uintptr_t) address & (BYTE_PAGE_SIZE - 1);         // 3. calculate the number of bytes that need to be copied from Flash.
    memcpy(pagebuff, (unsigned char *) pageStartAddr, numBefore);   // 4. make a copy of original data, if necessary

    while(size) {
        numLeftInPage = BYTE_PAGE_SIZE - numBefore;                 // 5. determine how many to copy from Source data
        if(size <= numLeftInPage) {
            memcpy((pagebuff + numBefore), data, size);             // copy all of it
            numAfter = numLeftInPage - size;                        // calculate the number of bytes that need to be stored after the address.
    	    if(numAfter) memcpy((pagebuff + numBefore + size), (unsigned char *) ((uintptr_t) address + size), numAfter); // copy whats left
            size = 0;
        }
        else {
            memcpy((pagebuff + numBefore), data, numLeftInPage);    // copy numLeft of it
            size -= numLeftInPage;                                  // decrement size
            address += numLeftInPage; data += numLeftInPage;        // increment addresses
        }
        NVMErasePage((void *) pageStartAddr);                       // erase the Page
        for(index = 0; index < NUM_ROWS_PAGE; index++) {            // program the Page
            NVMWriteRow((unsigned char *) (pageStartAddr + (index * BYTE_ROW_SIZE)), (pagebuff + (index * BYTE_ROW_SIZE)));
        }
		numBefore = 0;                                              // done with partial page, move to page boundary
		pageStartAddr = (uintptr_t) address;
    }
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


// RTC ==========================================================================================

uint8_t BCD2BIN(uint8_t v) {
    return (((v >> 4) * 10) + (v & 0x0F));
}


uint8_t BIN2BCD(uint8_t v) {
    return (((v / 10) << 4) + (v % 10));
}


// get time from DS3231 RTC
// will return 1 if successful, 0 otherwise
// will also set (enable_flags) accordingly
int i2cGetTime(struct tm *t) {
    enable_flags &= ~FLAG_RTC;
    i2cStart();
    if(i2cSend(I2C_DS3231)) goto i2cGetTimeFail;
    if(i2cSend(0x00)) goto i2cGetTimeFail;  // register address
    i2cRepStart();
    if(i2cSend(I2C_DS3231 | 1)) goto i2cGetTimeFail;
    t->tm_sec = BCD2BIN(i2cRecv(0));
    t->tm_min = BCD2BIN(i2cRecv(0));
    t->tm_hour = BCD2BIN(i2cRecv(0));
    i2cRecv(0); // not using the weekday
    t->tm_mday = BCD2BIN(i2cRecv(0));
    uint8_t v = i2cRecv(0);
    uint8_t c = 1 + !!(v & 0x80);
    v &= 0x7f;
    t->tm_mon = BCD2BIN(v - 1);
    t->tm_year = (BCD2BIN(i2cRecv(1)) % 100) + (c * 100);
    enable_flags |= FLAG_RTC;
    
    i2cGetTimeFail:
    i2cStop();
    return !!(enable_flags & FLAG_RTC);
}


// set time into DS3231 RTC
int i2cSetTime(struct tm *t) {
    enable_flags &= ~FLAG_RTC;
    i2cStart();
    if(i2cSend(I2C_DS3231)) goto i2cSetTimeFail;
    if(i2cSend(0x0E)) goto i2cSetTimeFail;  // register address
    i2cRepStart();
    if(i2cSend(I2C_DS3231 | 1)) goto i2cSetTimeFail;
    uint8_t ctl = i2cRecv(1);   // read the control register
    i2cStop();
    
    if(ctl & BIT(7)) {  // make sure bit 7 is always 0 in order to enable the oscillator running on Vbat
        i2cStart();
        if(i2cSend(I2C_DS3231)) goto i2cSetTimeFail;
        if(i2cSend(0x0E)) goto i2cSetTimeFail;  // register address
        if(i2cSend(ctl & ~BIT(7))) goto i2cSetTimeFail;
        i2cStop();
    }
    
    i2cStart();
    if(i2cSend(I2C_DS3231)) goto i2cSetTimeFail;
    if(i2cSend(0x00)) goto i2cSetTimeFail;  // register address
    if(i2cSend(BIN2BCD(t->tm_sec))) goto i2cSetTimeFail;
    if(i2cSend(BIN2BCD(t->tm_min))) goto i2cSetTimeFail;
    if(i2cSend(BIN2BCD(t->tm_hour))) goto i2cSetTimeFail;
    if(i2cSend(1)) goto i2cSetTimeFail;     // weekday is not used
    if(i2cSend(BIN2BCD(t->tm_mday))) goto i2cSetTimeFail;
    if(i2cSend(BIN2BCD((t->tm_mon + 1) | ((t->tm_year > 199) ? 0x80 : 0)))) goto i2cSetTimeFail;
    if(i2cSend(BIN2BCD(t->tm_year % 100))) goto i2cSetTimeFail;
    i2cStop();
    enable_flags |= FLAG_RTC;
    
    i2cSetTimeFail:
    i2cStop();
    return !!(enable_flags & FLAG_RTC);
}


// SOUND ========================================================================================

// frequency in Hz, sound volume between 0 and 1000
void sound(int freq, int vol) {
    I2C_PULLUP |= (SCL | SDA);
    SCL1; SDA1;
    if(freq && vol) {
        int p = (_PB_FREQ / 16 / freq);
        OpenTimer2((T2_ON | T2_PS_1_16 | T2_SOURCE_INT), p);
        OpenOC2((OC_ON | OC_TIMER2_SRC | OC_CONTINUE_PULSE), ((p / 1000) * (vol % 1001)), 0);
        PPSOutput(2, RPB8, OC2);    // B8 is the output to the speaker
    }
    else {
        CloseTimer2();
        CloseOC2();
        PPSOutput(2, RPB8, NULL); 
    }
    SCL1; SDAin;
    while(SDAget == 0) {
        I2C_LAT ^= SCL;
        uSec(i2c_bit_us);
    }
    SCL1; SDA1;
    uSec(i2c_bit_us);
}


// 945Hz, 0.1s
void beep(void) {
    sound(945, 1000);
    mSec(100);
    sound(0, 0);
}
