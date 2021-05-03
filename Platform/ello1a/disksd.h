#ifndef DISK_SD_H
#define	DISK_SD_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "../../fatfs/source/ff.h"
#include "../../fatfs/source/diskio.h"

unsigned char currentSD;    /* current SD card index: 0=SD1, 1=SD2, ... */
signed long SDdlyCntr;      /* delay counter decreased in the xchgSPI() function */

void CS_LOW(void);
void CS_HIGH(void);

/* Get Disk Status */
DSTATUS sd_status (
	BYTE pdrv		/* Physical drive number (0) */
);

/* Initialise Disk Drive */
DSTATUS sd_init (
	BYTE pdrv		/* Physical drive number (0) */
);

/* Read Sector(s) */
DRESULT sd_read (
	BYTE pdrv,		/* Physical drive number (0) */
	BYTE *buff,		/* Pointer to the data buffer to store read data */
	DWORD sector,	/* Start sector number (LBA) */
	UINT count		/* Sector count (1..128) */
);

/* Write Sector(s) */
#if FF_FS_READONLY == 0
DRESULT sd_write (
	BYTE pdrv,				/* Physical drive number (0) */
	const BYTE *buff,		/* Pointer to the data to be written */
	DWORD sector,			/* Start sector number (LBA) */
	UINT count				/* Sector count (1..128) */
);
#endif

/* Miscellaneous Functions */
DRESULT sd_ioctl (
	BYTE pdrv,		/* Physical drive number (0) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive data block */
);

#ifdef	__cplusplus
}
#endif

#endif
