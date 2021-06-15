/* collection of various useful functions related to PIC32MZ-EF */

#ifndef PIC32MZEF_H
#define PIC32MZEF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "platform.h"

void delay_ms(unsigned long milliseconds);

/* bitmask definitions for pin configuration */
#define PIN_OPEN    0
#define PIN_CLOSE   BIT(0)
#define PIN_PUP     BIT(1)
#define PIN_PDN     BIT(2)
#define PIN_ODR     BIT(3)
#define PIN_DIG     0
#define PIN_ANL     BIT(4)
#define PIN_IN      0
#define PIN_OUT     BIT(5)

/* initialise an I/O pin */
int openPin(unsigned int pin,		/* pin number */
				unsigned int conf);	/* configuration bitmask (see PIN_xxx constants) */
									/* closing the port disregards all other configuration options */
									/* the function returns 0 when successfully executed */
									/*                     -1 when the configuration was invalid */
									/*                     -2 when the pin number was invalid */

/* read digital pin */
int dinPin(unsigned int pin);		/* pin number */
									/* the function returns 0 or 1 (bit read from the pin) when successfully executed */
									/*                     -2 when the pin number was invalid */

/* set digital pin */
int doutPin(unsigned int pin,		/* pin number */
				int data_bit);		/* value 0 or 1 (not 0) */
									/* the function returns 0 or 1 (bit set to the pin) when successfully executed */
									/*                     -2 when the pin number was invalid */

/* toggle digital pin */
int dtogPin(unsigned int pin);		/* pin number */
									/* the function returns 0 or 1 (bit set to the pin) when successfully executed */
									/*                     -2 when the pin number was invalid */

/* read analogue pin */
int ainPin(unsigned int pin);		/* pin number */
									/* the function returns value read from the ADC when successfully executed */
									/*                     -2 when the pin number was invalid */

/* open/close PWM output */
int openPinPWM(unsigned int pin,	/* pin number */
				unsigned long freq); /* carrier frequency in Hz */
									/* calling the function with frequency 0 will close the PWM pin */
									/* the function returns 0 when properly executed */
									/*                     -2 when the pin number was invalid */

/* set duty cycle for PWM output */
int setPinPWM(unsigned int pin,		/* pin number */
				unsigned long width); /* pulse width in 1/1,000,000 resolution */
									/* calling the function with frequency 0 will close the PWM pin */
									/* the function returns 0 when properly executed */
									/*                     -2 when the pin number was invalid */

#ifdef __cplusplus
}
#endif

#endif  /* PIC32MZEF_H */
