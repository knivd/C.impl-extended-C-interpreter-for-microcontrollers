#ifndef PLATFORM_H
#define PLATFORM_H

#define IFS_DRV_KB 8192     /* IFS drive size in kB (this can be 0 if IFS drive is not required) */
#define RAM_DRV_KB 1024     /* RAM drive size in kB (this can be 0 if RAM drive is not required) */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <time.h>
#include "../../fatfs/source/ff.h"
#include "../../ride/ride.h"

/* always required platform definitions (even if the system has no capability to handle certain functionalities) */

time_t ss_time;	/* counter of the seconds since Jan 1, 1970, 00:00:00.000 */
FATFS FatFs;    /* work area for FatFs */

/* cold initialisation of the hardware platform */
void initPlatform(void);

/* warm initialisation of the hardware platform */
void resetPlatform(void);

/* delay in milliseconds */
void mSec(clock_t ms);

/* system beep */
void beep(void);

unsigned char *SysMem;      // system memory
unsigned long SysMemSize;   // size in bytes of the allocated system memory

unsigned char *VidMem;      // video memory
unsigned long VidMemSize;   // size in bytes of the allocated video memory

int8_t kbdLayout;
uint16_t Hres, Vres;

void initVideo(uint8_t mode);
void clearScreen(int c);
void scrollUp(int vl, int c);
void setPixel(int x, int y, int c);
int getPixel(int x, int y);

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_H */

