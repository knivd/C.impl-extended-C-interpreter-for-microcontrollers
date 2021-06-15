/*
 * File:   uconsole.h
 * Author: Spas Spasov
 *
 * Created on May 2019
 */

#ifndef UCONSOLE_H
#define	UCONSOLE_H

#ifdef	__cplusplus
extern "C" {
#endif

//** USB INCLUDES ***********************************************************
#include "plibs/usb/driver/usb/usbhs/drv_usbhs.h"
#include "plibs/usb/usb/usb_device.h"
#include "plibs/usb/usb/usb_device_cdc.h"


/*****************************************************************************************************************************
USB specific defines
******************************************************************************************************************************/



/* Application USB Device CDC Read Buffer Size. This should be a multiple of
 * the CDC Bulk Endpoint size */
#define APP_READ_BUFFER_SIZE	384

#define USB_RX_BUFFER_SIZE		(APP_READ_BUFFER_SIZE)
#define USB_TX_BUFFER_SIZE		64
#define USBTIMEOUT				30

#define CONSOLE_RX_BUF_SIZE		256
#define CONSOLE_TX_BUF_SIZE		64


// declare the console I/O functions in this file
extern void initUSBConsole(void);
extern void CloseUSBConsole(void);
extern void SerUSBPutC(int c);
extern void SerUSBPutS(char *s);
extern int serUSBGetC(void);

// declare the USB I/O functions
extern void CheckUSB(void);

#ifdef	__cplusplus
}
#endif

#endif	/* UCONSOLE_H */

