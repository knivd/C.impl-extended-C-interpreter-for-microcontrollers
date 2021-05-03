#ifndef RIDE_FOS_H
#define	RIDE_FOS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "ride.h"
#include "../config.h"
#include "../FatFs/source/ff.h"

#ifndef PLATFORM_H
FATFS FatFs;    /* work area for FatFs */
#endif

#if (0)	/* these definitions are now placed in ffconf.h */

/* Definitions of physical drive number for each drive */
#define DEV_NUL     0   /* NULL device */
#define DEV_IFS     1	/* internal flash disk */
#define DEV_RAM     2	/* small internal RAM disk for data exchange between files */
#define DEV_SD1     3	/* external SD card 1 */
#define DEV_SD2     4	/* external SD card 2 */

#endif

FATFS FatFs;    	/* work area for FatFs */
FIL File;           /* working area for file operations in RIDE */
char *file_to_run;	/* filename to run (needed for the run() system library function) */

int errfile(FRESULT r, char *msg);

#ifdef	__cplusplus
}
#endif

#endif	/* RIDE_FOS_H */
