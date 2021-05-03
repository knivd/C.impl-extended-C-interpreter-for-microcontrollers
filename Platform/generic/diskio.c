/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various existing       */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include <time.h>
#include <string.h>		/* memcpy() */
#include <stdio.h>		/* printf() */
#include "../../xmem.h"
#include "../../fatfs/source/diskio.h"
#include "../../ride/fos.h"
#include "platform.h"

#define IFSfile "ifs.dat"

FILE *drvIFS = NULL;    /* internal flash disk data entry point for the IFS: drive */
byte *drvRAM = NULL;    /* internal RAM disk data entry point for the RAM: drive */


DWORD get_fattime (void) {
	time_t tt;
	time(&tt);
	struct tm *lt = localtime(&tt);
	return ((DWORD) (lt->tm_year - 80) << 25)
         | ((DWORD) (lt->tm_mon + 1) << 21)
         | ((DWORD) lt->tm_mday << 16)
         | ((DWORD) lt->tm_hour << 11)
         | ((DWORD) lt->tm_min << 5)
         | ((DWORD) lt->tm_sec >> 1);
}


/* helper function for getting current sector size */
unsigned int get_ssize(FATFS *fs) {
#if FF_MIN_SS != FF_MAX_SS
    return fs->ssize;
#else
    return (FF_MAX_SS) + (fs->id - fs->id); /* suppressing the unused variable warning from the compiler */
#endif
}


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive number to identify the drive */
) {
	DSTATUS stat = 0;

	switch(pdrv) {

		case DEV_NUL:
			return stat;

		case DEV_IFS:
            #if IFS_DRV_KB > 0
			drvIFS = fopen(IFSfile, "rb");
			if(!drvIFS) stat |= STA_NODISK;
			fclose(drvIFS);
            #endif
			return stat;

		case DEV_RAM:
			if(!drvRAM) stat |= STA_NODISK;
			return stat;

		case DEV_SD1:

			/* TODO ### */

			return stat;

        default: break;
    }
	return STA_NOINIT;
}


/*-----------------------------------------------------------------------*/
/* Initialise a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive number to identify the drive */
) {
	DSTATUS stat = 0;

	switch(pdrv) {

		case DEV_NUL:
			return stat;

		case DEV_IFS:
            #if IFS_DRV_KB > 0
			if(!drvIFS) {
				drvIFS = fopen(IFSfile, "a+b");
				if(drvIFS) {
					fseek(drvIFS, 0, SEEK_END);
        			unsigned long len = ftell(drvIFS);
        			rewind(drvIFS);
					if(len < (IFS_DRV_KB * 1024)) {    /* create the IFS emulation file */
						char *buff = NULL;
						x_malloc((byte **) &buff, 1024);
						while(len < (IFS_DRV_KB * 1024)) {
							fwrite(buff, 1, 1024, drvIFS);
							len += 1024;
						}
						x_free((byte **) &buff);
					}
                    fclose(drvIFS);
				}
				else stat |= STA_NODISK;
			}
            #endif
			return stat;

		case DEV_RAM:
			if(!drvRAM) {
				x_malloc((byte **) &drvRAM, (RAM_DRV_KB * 1024));	/* allocate RAM disk memory */
				if(!drvRAM) stat |= STA_NODISK;
			}
			return stat;

		case DEV_SD1:

			/* TODO ### */

			return stat;

        default: break;

    }
	return STA_NOINIT;
}


/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive number to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
) {
	DRESULT res = FR_OK;

	switch(pdrv) {

		case DEV_NUL:
			return res;

		case DEV_IFS:
            #if IFS_DRV_KB > 0
			if(drvIFS) {
				if(sector < ((IFS_DRV_KB * 1024) / get_ssize(&FatFs))) {
					drvIFS = fopen(IFSfile, "rb");
					if(drvIFS) {
						fseek(drvIFS, (sector * get_ssize(&FatFs)), SEEK_SET);
						fread(buff, get_ssize(&FatFs), count, drvIFS);
						fclose(drvIFS);
					}
					else res = RES_NOTRDY;
				}
				else res = RES_PARERR;
			}
			else res = RES_NOTRDY;
            #endif
			return res;

		case DEV_RAM:
			if(drvRAM) {
				if(sector < ((RAM_DRV_KB * 1024) / get_ssize(&FatFs))) {
					memcpy(buff, (drvRAM + (sector * get_ssize(&FatFs))), (count * get_ssize(&FatFs)));
				}
				else res = RES_PARERR;
			}
			else res = RES_NOTRDY;
			return res;

		case DEV_SD1:

			/* TODO ### */

			return res;

        default: break;

    }
	return RES_PARERR;
}


/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive number to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
) {
	DRESULT res = FR_OK;

	switch(pdrv) {

		case DEV_NUL:
			return res;

		case DEV_IFS:
            #if IFS_DRV_KB > 0
			if(drvIFS) {
				if(sector < ((IFS_DRV_KB * 1024) / get_ssize(&FatFs))) {
					drvIFS = fopen(IFSfile, "r+b");
					if(drvIFS) {
						fseek(drvIFS, (sector * get_ssize(&FatFs)), SEEK_SET);
						fwrite(buff, get_ssize(&FatFs), count, drvIFS);
						fclose(drvIFS);
					}
					else res = RES_NOTRDY;
				}
				else res = RES_PARERR;
			}
			else res = RES_NOTRDY;
            #endif
			return res;

		case DEV_RAM:
			if(drvRAM) {
				if(sector < ((RAM_DRV_KB * 1024) / get_ssize(&FatFs))) {
					memcpy((drvRAM + (sector * get_ssize(&FatFs))), buff, (count * get_ssize(&FatFs)));
				}
				else res = RES_PARERR;
			}
			else res = RES_NOTRDY;
			return res;

		case DEV_SD1:

			/* TODO ### */

			return res;

        default: break;

    }
	return RES_PARERR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive number (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
) {
	DRESULT res = FR_OK;
    DWORD v;

	switch(pdrv) {

		case DEV_NUL:
			switch(cmd) {
				case GET_SECTOR_COUNT:
					v = 0;
					memcpy(buff, &v, sizeof(DWORD));
					break;

				case GET_SECTOR_SIZE:
					v = get_ssize(&FatFs);
					memcpy(buff, &v, sizeof(WORD));
					break;

				case GET_BLOCK_SIZE:
					v = 1; /* not flash memory */
					memcpy(buff, &v, sizeof(DWORD));
					break;

                default: break;
			}
			return res;

		case DEV_IFS:
            #if IFS_DRV_KB > 0
			switch(cmd) {
				case GET_SECTOR_COUNT:
					v = (IFS_DRV_KB * 1024) / get_ssize(&FatFs);
					memcpy(buff, &v, sizeof(DWORD));
					break;

				case GET_SECTOR_SIZE:
					v = FF_MIN_SS;
					memcpy(buff, &v, sizeof(WORD));
					break;

				case GET_BLOCK_SIZE:
					v = 1;	/* not flash memory */
					memcpy(buff, &v, sizeof(DWORD));
					break;

                default: break;
			}
            #endif
			return res;

		case DEV_RAM:
			switch(cmd) {
				case GET_SECTOR_COUNT:
					v = (RAM_DRV_KB * 1024) / FF_MIN_SS;
					memcpy(buff, &v, sizeof(DWORD));
					break;

				case GET_SECTOR_SIZE:
					v = FF_MIN_SS;
					memcpy(buff, &v, sizeof(WORD));
					break;

				case GET_BLOCK_SIZE:
					v = 1;	/* not flash memory */
					memcpy(buff, &v, sizeof(DWORD));
					break;

                default: break;
			}
			return res;

		case DEV_SD1:

			/* TODO ### */

			return res;

        default: break;
	}
	return RES_PARERR;
}

