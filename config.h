#ifndef ARCH_H

/* platform type */
/* 0: Generic - no hardware platform */
/* 1: ELLO 1A (PIC32MX270B) */
/* 2: Rittle Board (PIC32MZ2048EFH064) */
#ifndef PLATFORM
#define PLATFORM    0
#endif

#define SW_VERSION  "114"

/* these must be the actual platform headers */

/* generic (non platform-specific) system */
#if PLATFORM == 0
	#define PLATFORM_NAME	"Generic RIDE w/ C.impl"
	#include "platform/generic/platform.h"
	#include "platform/generic/l_platfm.h"

/* ELLO 1A */
#elif PLATFORM == 1
	#define PLATFORM_NAME	"ELLO 1A"
	#include "platform/ello1a/platform.h"
	#include "platform/ello1a/l_platfm.h"

/* Rittle Board */
#elif PLATFORM == 2
    #define PLATFORM_NAME	"Rittle Board"
	#include "platform/rittle/platform.h"
	#include "platform/rittle/l_platfm.h"

#else
	#error "#define PLATFORM <platform_code> is missing or invalid"
#endif

#ifndef AUTHOR
#define AUTHOR      "KnivD"
#endif

#include <stdint.h>

#ifndef BIT
#define BIT(b) (1ull << (b))
#endif

#define FLAG_VIDEO      BIT(0)      // video output is enabled
#define FLAG_NO_SCROLL	BIT(1)		// disable screen scroll
#define FLAG_NO_CTRL	BIT(2)		// disable output of control characters
#define FLAG_NO_ECHO	BIT(3)		// disable echo characters
#define FLAG_PS2        BIT(4)      // PS/2 keyboard detected
#define FLAG_SERIAL     BIT(5)      // serial console is enabled on RB5(RX)/RB7(TX)
#define FLAG_USB_PRES   BIT(6)      // USB connectivity is possible
#define FLAG_USB        BIT(7)      // USB console is configured and active
#define FLAG_RTC        BIT(8)      // RTC detected
#define FLAG_RTC_UPDATE BIT(9)      // request to update the RTC with the value from (ss_time)

uint16_t enable_flags;              // see the FLAG_xxx definitions

#endif
