#ifndef CONFIG_H
#define CONFIG_H

/* platform type */
/* 0: Generic - no hardware platform */
/* 1: ELLO 1A (PIC32MX270B) */
/* 2: Rittle Board (PIC32MZ2048EFH064) */
#ifndef PLATFORM
#define PLATFORM    1
#endif

#define SW_VERSION  "120"

/* these must be the actual platform headers */

/* generic (non platform-specific) system */
#if PLATFORM == 0
	#define PLATFORM_NAME	"Generic RIDE w/ C.impl"
    #define FF_MAX_SS       512
	#include "platform/generic/platform.h"
	#include "platform/generic/l_platfm.h"

/* ELLO 1A */
#elif PLATFORM == 1
	#define PLATFORM_NAME	"ELLO 1A"
    #define FF_MAX_SS       1024
	#include "platform/ello1a/platform.h"
	#include "platform/ello1a/l_platfm.h"

/* Rittle Board */
#elif PLATFORM == 2
    #define PLATFORM_NAME	"Rittle Board"
    #define FF_MAX_SS       2048
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
#define FLAG_EXECUTING  BIT(10)     // ongoing execution flag

uint16_t enable_flags;              // see the FLAG_xxx definitions

#define PRINT_VER_INFO() printf("R%s (C) %c%c%c %s, %s", SW_VERSION, \
                                                        __DATE__[strlen(__DATE__) - 11], \
                                                        __DATE__[strlen(__DATE__) - 10], \
                                                        __DATE__[strlen(__DATE__) - 9], \
                                                        &__DATE__[strlen(__DATE__) - 4], \
                                                        AUTHOR);

#ifndef min
#define min(x,y) (((x)<(y))?(x):(y))
#endif

#ifndef max
#define max(x,y) (((x)>(y))?(x):(y))
#endif

#endif
