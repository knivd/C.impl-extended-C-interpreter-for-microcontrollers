#include <string.h>
#include <xc.h>
#include "platform.h"
#include "pic32mzef.h"

/* possible pin functions */
#define PFN_DIN   BIT(0)
#define PFN_DOUT  BIT(1)
#define PFN_AIN   BIT(2)
#define PFN_AOUT  BIT(3)
#define PFN_PUP   BIT(4)
#define PFN_PDN   BIT(5)
#define PFN_ODR   BIT(6)
#define PFN_PWM   BIT(7)

typedef struct _pin_t {
    unsigned short pin;             /* physical pin number */
    volatile unsigned int *pbase;   /* base PORT address */
    unsigned char pbit;             /* bit in the hardware port */
    char adcchn;                    /* ADC channel number (if available, otherwise -1) */
    unsigned short func;            /* bitmask of the possible assignment functions for the port (see PFN_xxx constants) */
} pin_t;

/* pin definitions for 64-pin package PIC32MZ2048EFH064 */
const pin_t pins[] = {
    
    { 1,   BASEE,  5,    17,    (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_ODR) },
    { 2,   BASEE,  6,    16,    (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_ODR) },
    { 3,   BASEE,  7,    15,    (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_ODR) },
    { 4,   BASEG,  6,    14,    (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_ODR) },
    { 5,   BASEG,  7,    13,    (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_ODR) },
    { 6,   BASEG,  8,    12,    (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_ODR) },

    { 10,  BASEG,  9,    11,    (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_ODR) },
    { 11,  BASEB,  5,    45,    (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_ODR) },
    { 12,  BASEB,  4,    4,     (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_ODR) },
    { 13,  BASEB,  3,    3,     (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_ODR) },
    { 14,  BASEB,  2,    2,     (PFN_DOUT | PFN_DIN | PFN_AIN | PFN_PWM | PFN_PUP | PFN_PDN | PFN_ODR) },
    { 15,  BASEB,  1,    1,     (PFN_DOUT | PFN_DIN | PFN_AIN | PFN_PWM | PFN_PUP | PFN_PDN | PFN_ODR) },
    { 16,  BASEB,  0,    0,     (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_ODR) },
    { 17,  BASEB,  6,    46,    (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_ODR) },
    { 18,  BASEB,  7,    47,    (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_ODR) },

    { 21,  BASEB,  8,    48,    (PFN_DOUT | PFN_DIN | PFN_AIN | PFN_PWM | PFN_PUP | PFN_PDN | PFN_ODR) },
    { 22,  BASEB,  9,    49,    (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_ODR) },
    { 23,  BASEB,  10,   5,     (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_ODR) },
    { 24,  BASEB,  11,   6,     (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_ODR) },

    { 27,  BASEB,  12,   7,     (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_ODR) },
    { 28,  BASEB,  13,   8,     (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_ODR) },
    { 29,  BASEB,  14,   9,     (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_ODR) },
    { 30,  BASEB,  15,   10,    (PFN_DOUT | PFN_DIN | PFN_AIN | PFN_PWM | PFN_PUP | PFN_PDN | PFN_ODR) },
    { 31,  BASEC,  12,  -1,     (PFN_DOUT | PFN_DIN |                     PFN_PUP | PFN_PDN | PFN_ODR) },
    { 32,  BASEC,  15,  -1,     (PFN_DOUT | PFN_DIN |                     PFN_PUP | PFN_PDN | PFN_ODR) },

    { 38,  BASEF,  3,   -1,     (PFN_DOUT | PFN_DIN |           PFN_PWM | PFN_PUP | PFN_PDN | PFN_ODR) },

    { 41,  BASEF,  4,   -1,     (PFN_DOUT | PFN_DIN |                     PFN_PUP | PFN_PDN | PFN_ODR) },
    { 42,  BASEF,  5,   -1,     (PFN_DOUT | PFN_DIN |                     PFN_PUP | PFN_PDN | PFN_ODR) },
    { 43,  BASED,  9,   -1,     (PFN_DOUT | PFN_DIN |                     PFN_PUP | PFN_PDN | PFN_ODR) },
    { 44,  BASED,  10,  -1,     (PFN_DOUT | PFN_DIN |                     PFN_PUP | PFN_PDN | PFN_ODR) },
    { 45,  BASED,  11,  -1,     (PFN_DOUT | PFN_DIN |           PFN_PWM | PFN_PUP | PFN_PDN | PFN_ODR) },
    { 46,  BASED,  0,   -1,     (PFN_DOUT | PFN_DIN |           PFN_PWM | PFN_PUP | PFN_PDN | PFN_ODR) },
    { 47,  BASEC,  13,  -1,     (PFN_DOUT | PFN_DIN |                     PFN_PUP | PFN_PDN | PFN_ODR) },
    { 48,  BASEC,  14,  -1,     (PFN_DOUT | PFN_DIN |                     PFN_PUP | PFN_PDN | PFN_ODR) },
    { 49,  BASED,  1,   -1,     (PFN_DOUT | PFN_DIN |                     PFN_PUP | PFN_PDN | PFN_ODR) },
    { 50,  BASED,  2,   -1,     (PFN_DOUT | PFN_DIN |                     PFN_PUP | PFN_PDN | PFN_ODR) },
    { 51,  BASED,  3,   -1,     (PFN_DOUT | PFN_DIN |                     PFN_PUP | PFN_PDN | PFN_ODR) },
    { 52,  BASED,  4,   -1,     (PFN_DOUT | PFN_DIN |                     PFN_PUP | PFN_PDN | PFN_ODR) },
    { 53,  BASED,  5,   -1,     (PFN_DOUT | PFN_DIN |                     PFN_PUP | PFN_PDN | PFN_ODR) },

    { 56,  BASEF,  0,   -1,     (PFN_DOUT | PFN_DIN |                     PFN_PUP | PFN_PDN | PFN_ODR) },
    { 57,  BASEF,  1,   -1,     (PFN_DOUT | PFN_DIN |                     PFN_PUP | PFN_PDN | PFN_ODR) },
    { 58,  BASEE,  0,   -1,     (PFN_DOUT | PFN_DIN |                     PFN_PUP | PFN_PDN | PFN_ODR) },

    { 61,  BASEE,  1,   -1,     (PFN_DOUT | PFN_DIN |                     PFN_PUP | PFN_PDN | PFN_ODR) },
    { 62,  BASEE,  2,   -1,     (PFN_DOUT | PFN_DIN |                     PFN_PUP | PFN_PDN | PFN_ODR) },
    { 63,  BASEE,  3,   -1,     (PFN_DOUT | PFN_DIN |                     PFN_PUP | PFN_PDN | PFN_ODR) },
    { 64,  BASEE,  4,    18,    (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_ODR) },

{0, 0, 0, 0, 0} /* must be final in this list */
};


void sf_delay_ms(unsigned long milliseconds) {
    unsigned long ce = clock() + milliseconds;
    while(clock() > ce);   /* to cover for the (rare) cases of counter roll-over */
    while(clock() < ce);
}


int openPin(unsigned int pin, unsigned int conf) {
    const pin_t *p = pins;
    while(p->pin && p->pin != (uint16_t) pin) p++;
    if(p->pin == 0) return -2;	/* invalid pin number */
    
    /* reset the pin (to digital input) */
    *(p->pbase + TRISSET) = BIT(p->pbit);
    *(p->pbase + ANSELCLR) = BIT(p->pbit);
    *(p->pbase + LATCLR) = BIT(p->pbit);
    *(p->pbase + PORTCLR) = BIT(p->pbit);    
    *(p->pbase + CNPUCLR) = BIT(p->pbit);
    *(p->pbase + CNPDCLR) = BIT(p->pbit);
    *(p->pbase + ODCCLR) = BIT(p->pbit);
    if(conf & PIN_CLOSE) return;	/* end here when closing the pin */
    int res = -1;
    
    if(conf & PIN_PUP) {
        if((p->func & PFN_PUP) == 0) return -1;
        *(p->pbase + CNPUSET) = BIT(p->pbit);
    }
    if(conf & PIN_PDN) {
        if((p->func & PFN_PDN) == 0) return -1;
        *(p->pbase + CNPDSET) = BIT(p->pbit);
    }
    if(conf & PIN_ODR) {
        if((p->func & PFN_ODR) == 0) return -1;
        *(p->pbase + ODCSET) = BIT(p->pbit);
    }
    
    if(conf & PIN_ANL) {    /* analogue pin */
        if(conf & PIN_IN) {
            if((p->func & PFN_AIN) == 0) return -1;
            *(p->pbase + TRISSET) = BIT(p->pbit);
            *(p->pbase + ANSELSET) = BIT(p->pbit);
        }
        if(conf & PIN_OUT) {
            if((p->func & PFN_AOUT) == 0) return -1;
            *(p->pbase + TRISCLR) = BIT(p->pbit);
            *(p->pbase + ANSELSET) = BIT(p->pbit);
        }
    }
    
    else {  /* digital pin */
        if(conf & PIN_IN) {
            if((p->func & PFN_DIN) == 0) return -1;
            *(p->pbase + TRISSET) = BIT(p->pbit);
            *(p->pbase + ANSELCLR) = BIT(p->pbit);
        }
        if(conf & PIN_OUT) {
            if((p->func & PFN_DOUT) == 0) return -1;
            *(p->pbase + TRISCLR) = BIT(p->pbit);
            *(p->pbase + ANSELCLR) = BIT(p->pbit);
        }
    }
    
    return 0;
}


int dinPin(unsigned int pin) {
    const pin_t *p = pins;
    while(p->pin && p->pin != (uint16_t) pin) p++;
    if(p->pin == 0 || p->func & PFN_DIN == 0) return -2;	/* invalid pin number */
    return !!(*(p->pbase + PORT) & BIT(p->pbit));
}


int doutPin(unsigned int pin, int data_bit) {
    const pin_t *p = pins;
    while(p->pin && p->pin != (uint16_t) pin) p++;
    if(p->pin == 0 || p->func & PFN_DOUT == 0) return -2;	/* invalid pin number */
    if(data_bit) *(p->pbase + LATSET) & BIT(p->pbit);
    else *(p->pbase + LATCLR) & BIT(p->pbit);
    return !!(*(p->pbase + LAT) & BIT(p->pbit));
}


int dtogPin(unsigned int pin) {
    const pin_t *p = pins;
    while(p->pin && p->pin != (uint16_t) pin) p++;
    if(p->pin == 0 || p->func & PFN_DOUT == 0) return -2;	/* invalid pin number */
    int z = *(p->pbase + LAT) & BIT(p->pbit);
    if(z) *(p->pbase + LATCLR) & BIT(p->pbit); else *(p->pbase + LATSET) & BIT(p->pbit);
    return !z;
}


int ainPin(unsigned int pin) {
    const pin_t *p = pins;
    while(p->pin && p->pin != (uint16_t) pin) p++;
    if(p->pin == 0 || p->func & PFN_AIN == 0 || p->adcchn < 0) return -2;
    EnableADC12();
	SetChanADC12(p->adcchn);
    ADC12SetupChannel(ADC12GetChannel(p->adcchn), ADC12_DATA_12BIT, 2, 14, ADC12_EARLY_INTERRUPT_PRIOR_CLOCK_1);
	OpenADC12(p->adcchn);
	int res = ReadADC12(p->adcchn);
    CloseADC12();
	return res;
}


int openPinPWM(unsigned int pin, unsigned long freq) {
    const pin_t *p = pins;
    while(p->pin && p->pin != (uint16_t) pin) p++;
    if(p->pin == 0 || p->func & PFN_PWM == 0) return -2;
    
    /* reset the pin (to digital input) */
    *(p->pbase + TRISSET) = BIT(p->pbit);
    *(p->pbase + ANSELCLR) = BIT(p->pbit);
    *(p->pbase + LATCLR) = BIT(p->pbit);
    *(p->pbase + PORTCLR) = BIT(p->pbit);    
    *(p->pbase + CNPUCLR) = BIT(p->pbit);
    *(p->pbase + CNPDCLR) = BIT(p->pbit);
    *(p->pbase + ODCCLR) = BIT(p->pbit);
    if(freq == 0) return;	/* end here if closing the PWM pin */
    
    *(p->pbase + TRISCLR) = BIT(p->pbit);   /* set the pin as output (needed to drive the PWM out) */
    unsigned long duty = (PBCLK3 / freq);	/* it is already verified by now that d2.val.i > 0 */
	int res = 0;
    
    /* Group 1 - Timer23, OC4 (pin 15) */
    if(p->pin == 15) {
        if(freq > 0) {  /* open */
            PPSUnLock;
            PPSOutput(2, RPB1, OC4);
            PPSLock;
            OpenTimer23((T23_ON | T23_IDLE_CON | T23_GATE_OFF | T23_32BIT_MODE_ON | T23_PS_1_1), duty);
            WriteTimer23(0);
            OpenOC4((OC_ON | OC_IDLE_CON | OC_TIMER_MODE32 | OC_TIMER2_SRC | OC_CONTINUE_PULSE), duty, (duty / 2));
        }
        else PPSOutput(2, RPB1, NULL);  /* close */
    }
    
    /* Group 1 - Timer23, OC5 (pin 21) */
    else if(p->pin == 21) {
        if(freq > 0) {  /* open */
            PPSUnLock;
            PPSOutput(3, RPB8, OC5);
            PPSLock;
            OpenTimer23((T23_ON | T23_IDLE_CON | T23_GATE_OFF | T23_32BIT_MODE_ON | T23_PS_1_1), duty);
            WriteTimer23(0);
            OpenOC5((OC_ON | OC_IDLE_CON | OC_TIMER_MODE32 | OC_TIMER2_SRC | OC_CONTINUE_PULSE), duty, (duty / 2));
        }
        else PPSOutput(3, RPB8, NULL);  /* close */
    }
    
    /* Group 2 - Timer45, OC1 (pin 14) */
    else if(p->pin == 14) {
        if(freq > 0) {  /* open */
            PPSUnLock;
            PPSOutput(4, RPB2, OC1);
            PPSLock;
            OpenTimer45((T45_ON | T45_IDLE_CON | T45_GATE_OFF | T45_32BIT_MODE_ON | T45_PS_1_1), duty);
            WriteTimer45(0);
            OpenOC1((OC_ON | OC_IDLE_CON | OC_TIMER_MODE32 | OC_TIMER2_SRC | OC_CONTINUE_PULSE), duty, (duty / 2));
        }
        else PPSOutput(4, RPB2, NULL);  /* close */
    }
    
    /* Group 2 - Timer45, OC2 (pin 38) */
    else if(p->pin == 38) {
        if(freq > 0) {  /* open */
            PPSUnLock;
            PPSOutput(4, RPF3, OC2);
            PPSLock;
            OpenTimer45((T45_ON | T45_IDLE_CON | T45_GATE_OFF | T45_32BIT_MODE_ON | T45_PS_1_1), duty);
            WriteTimer45(0);
            OpenOC2((OC_ON | OC_IDLE_CON | OC_TIMER_MODE32 | OC_TIMER2_SRC | OC_CONTINUE_PULSE), duty, (duty / 2));
        }
        else PPSOutput(4, RPF3, NULL);  /* close */
    }
    
    /* Group 3 - Timer67, OC8 (pin 30) */
    else if(p->pin == 30) {
        if(freq > 0) {  /* open */
            PPSUnLock;
            PPSOutput(3, RPB15, OC8);
            PPSLock;
            OpenTimer67((T67_ON | T67_IDLE_CON | T67_GATE_OFF | T67_32BIT_MODE_ON | T67_PS_1_1), duty);
            WriteTimer67(0);
            OpenOC8((OC_ON | OC_IDLE_CON | OC_TIMER_MODE32 | OC_TIMER2_SRC | OC_CONTINUE_PULSE), duty, (duty / 2));
        }
        else PPSOutput(3, RPB15, NULL); /* close */
    }
    
    /* Group 3 - Timer67, OC7 (pin 45) */
    else if(p->pin == 45) {
        if(freq > 0) {  /* open */
            PPSUnLock;
            PPSOutput(2, RPD11, OC7);
            PPSLock;
            OpenTimer67((T67_ON | T67_IDLE_CON | T67_GATE_OFF | T67_32BIT_MODE_ON | T67_PS_1_1), duty);
            WriteTimer67(0);
            OpenOC7((OC_ON | OC_IDLE_CON | OC_TIMER_MODE32 | OC_TIMER2_SRC | OC_CONTINUE_PULSE), duty, (duty / 2));
        }
        else PPSOutput(2, RPD11, NULL); /* close */
    }
    
    /* Group 3 - Timer67, OC9 (pin 46) */
    else if(p->pin == 46) {
        if(freq > 0) {  /* open */
            PPSUnLock;
            PPSOutput(4, RPD0, OC9);
            PPSLock;
            OpenTimer67((T67_ON | T67_IDLE_CON | T67_GATE_OFF | T67_32BIT_MODE_ON | T67_PS_1_1), duty);
            WriteTimer67(0);
            OpenOC9((OC_ON | OC_IDLE_CON | OC_TIMER_MODE32 | OC_TIMER2_SRC | OC_CONTINUE_PULSE), duty, (duty / 2));
        }
        else PPSOutput(4, RPD0, NULL); /* close */
    }
    
    else res = -2;	/* bug in the initialisations here!! */
	return res;
}


int setPinPWM(unsigned int pin, unsigned long width) {
    const pin_t *p = pins;
    while(p->pin && p->pin != (uint16_t) pin) p++;
    if(p->pin == 0 || p->func & PFN_PWM == 0) return -2;

    if(p->pin == 14) {
        SetPulseOC1(((width * ReadDCOC1PWM()) / 1000000), ReadDCOC1PWM());
        WriteTimer45(0);
    }
    else if(p->pin == 15) {
        SetPulseOC4(((width * ReadDCOC4PWM()) / 1000000), ReadDCOC4PWM());
        WriteTimer23(0);
    }
	else if(p->pin == 21) {
        SetPulseOC5(((width * ReadDCOC5PWM()) / 1000000), ReadDCOC5PWM());
        WriteTimer23(0);
    }
	else if(p->pin == 30) {
        SetPulseOC8(((width * ReadDCOC8PWM()) / 1000000), ReadDCOC8PWM());
        WriteTimer67(0);
    }
	else if(p->pin == 38) {
        SetPulseOC2(((width * ReadDCOC2PWM()) / 1000000), ReadDCOC2PWM());
        WriteTimer45(0);
    }
	else if(p->pin == 45) {
        SetPulseOC7(((width * ReadDCOC7PWM()) / 1000000), ReadDCOC7PWM());
        WriteTimer67(0);
    }
	else if(p->pin == 46) {
        SetPulseOC9(((width * ReadDCOC9PWM()) / 1000000), ReadDCOC9PWM());
        WriteTimer67(0);
    }
	return 0;
}
