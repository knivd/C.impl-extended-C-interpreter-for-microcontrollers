#ifndef KBLAYOUT_H
#define	KBLAYOUT_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "ride.h"

// keyboard layout codes
#define LAYOUT_US	0
#define LAYOUT_UK	1
#define LAYOUT_DE	2
#define LAYOUT_FR	3

// PS/2 scankey codes that must be tracked for up/down state
#define CTRL  		0x14        // left and right CTRL generate the same code
#define L_SHFT  	0x12
#define R_SHFT  	0x59
#define CAPS    	0x58
#define NUML    	0x77
#define ALTGR       0x11

extern const unsigned short keyCodes[][128];

#ifdef	__cplusplus
}
#endif

#endif
