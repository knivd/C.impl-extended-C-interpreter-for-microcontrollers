#include <conio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "../xmem.h"
#include "fos.h"
#include "ride.h"

#define FOS_BUF	FF_LFN_BUF	/* path and filename buffer size */

char *wbuf = NULL;			/* working buffer */


/* show file error message */
int errfile(FRESULT r, char *msg) {
	file_to_run = NULL;
    printf("\r\n\n>>> ERROR: (FR%d) ", r);
    if(!msg || !*msg) printf("Unable to perform file system operation\r\n");
    else printf("%s\r\n", msg);
    return (int) r;
}


/* function "What???" */
static void what(void) {
	file_to_run = NULL;
    printf(" ???\r\n");
}


/* show memory allocation error message */
static void errmem(void) {
	file_to_run = NULL;
    printf("\r\n\n>>> ERROR: Unable to allocate memory\r\n");
}


/* copy individual file from source to destination */
FRESULT fcopy(char *src, char *dst) {
	TCHAR *rwbuf = NULL, *swbuf = NULL, *dwbuf = NULL;
    FIL *ffs = NULL, *ffd = NULL;
	x_malloc((byte **) &swbuf, FOS_BUF + 1);
	x_malloc((byte **) &dwbuf, FOS_BUF + 1);
    x_malloc((byte **) &ffs, sizeof(FIL));
    x_malloc((byte **) &ffd, sizeof(FIL));
    x_malloc((byte **) &rwbuf, FOS_BUF + 1);
	if(rwbuf) f_getcwd(rwbuf, FOS_BUF);
    uint32_t FCOPY_BUF_SIZE = (x_avail() / 4) * 3;  /* try to grab 75% of the currently available memory */
    TCHAR *buf = NULL;
    while(buf == NULL && FCOPY_BUF_SIZE > 256) {
        FCOPY_BUF_SIZE -= 256;
        x_malloc((byte **) &buf, FCOPY_BUF_SIZE);
    }
	if(strchr(src, ':')) strcpy(swbuf, src); else f_getcwd(swbuf, FOS_BUF);
	if(strchr(dst, ':')) strcpy(dwbuf, dst); else f_getcwd(dwbuf, FOS_BUF);
    UINT rw, rww, rpos = 0;
	FRESULT r = FR_NOT_ENOUGH_CORE; /* not enough memory for buffers */
    while(swbuf && dwbuf && buf && ffs && ffd) {	/* this loop always executes maximum once */
        r = FR_OK;
        if(r == FR_OK) f_chdrive(dwbuf);
        if(r == FR_OK) f_mount(&FatFs, dwbuf, 1);
        if(r == FR_OK) r = f_open(ffd, dst, (FA_CREATE_ALWAYS | FA_WRITE));
        f_close(ffd);
        do {
            if(r == FR_OK) r = f_chdrive(swbuf);
            if(r == FR_OK) r = f_mount(&FatFs, swbuf, 1);
            if(r == FR_OK) r = f_open(ffs, src, (FA_OPEN_EXISTING | FA_READ));
            if(r == FR_OK) r = f_lseek(ffs, rpos);
            if(r == FR_OK) r = f_read(ffs, buf, FCOPY_BUF_SIZE, &rw);
            f_close(ffs);
			rpos += FCOPY_BUF_SIZE;
            if(!rw) break;
            if(r == FR_OK) r = f_chdrive(dwbuf);
            if(r == FR_OK) r = f_mount(&FatFs, dwbuf, 1);
            if(r == FR_OK) r = f_open(ffd, dst, (FA_OPEN_APPEND | FA_WRITE));
            if(r == FR_OK) r = f_write(ffd, buf, rw, &rww);
            f_close(ffd);
            if(rw < FCOPY_BUF_SIZE) break;
        } while(r == FR_OK && rw == rww);
        break;
    }
    if(!ffs || !ffd) errmem();
	if(rwbuf) {
		f_chdrive(rwbuf);
		f_mount(&FatFs, rwbuf, 1);
	}
    x_free((byte **) &buf);
	x_free((byte **) &rwbuf);
	x_free((byte **) &swbuf);
	x_free((byte **) &dwbuf);
    x_free((byte **) &ffs);
    x_free((byte **) &ffd);
    return r;
}


/* helper function: compose new file name from the original in case of wildcards being used */
/* the name mask is supplied in (*mask) */
/* the original file name is supplied in (*p1) with no path */
/* (*p2) holds the entire second parameter which may contain drive, path, and/or wildcard characters */
/* (*buf) is a pointer to available buffer where the result will be returned */
char *wildname(char *mask, char *p1, char *p2, char *buf) {
	strcpy(buf, p2);
	char *fn = strchr(buf, ':');
	char *fnsl = strrchr(buf, '/');
    char *fnbs = strrchr(buf, '\\');
    fn = max(fn, fnsl);
	fn = max(fn, fnbs);
	if(fn) fn++; else fn = buf;
    if(fn && *fn == '\0') strcpy(&buf[strlen(buf)], p1);
	if(!strchr(fn, '*') && !strchr(fn, '?')) return buf;
	char i = *mask;	/* the only reason for this is to avoid the 'unused variable' message by the compiler */
	char *f2 = fn + strlen(p1) + 1;
	memmove(f2, fn, (strlen(fn) + 1));	/* move the file name in p2 to a new safe location */
	for(i = 0; i < 2; i++) {
		while(*f2 && *f2 != '.') {
			if(*f2 == '*') {
				f2++;
				while(*p1 && *p1 != '.') *(fn++) = *(p1++);
			}
			else if(*f2 == '?') {
				f2++;
				if(*p1 && *p1 != '.') *(fn++) = *(p1++);
				else *(fn++) = *(f2++);
			}
			else {
				*(fn++) = *(f2++);
				if(*p1) p1++;
			}
		}
		if(i == 0) {	/* executed only after the main file name */
			if(*f2 == '.') *(fn++) = *(f2++);
			while(*p1 && *p1 != '.') p1++;
			if(*p1 == '.') p1++;
		}
	}
	*fn = '\0';
	return buf;
}


/* helper function for execution of grouping commands */
/* 'cmd' specifies the command: 'L': dir, 'D': del, 'R': ren */
/* 'chb' is pointer to free working buffer */
/* 'msgf' enables messages on the screen (ignored for 'L') */
void group_exec(char cmd, char *pcmd, char *chb, char msgf) {
    DIR *dir = NULL;
	FILINFO *finfo = NULL;
	x_malloc((byte **) &dir, sizeof(DIR));
	x_malloc((byte **) &finfo, sizeof(FILINFO));
    char *par2 = strchr(pcmd, ',');
    if(par2) {
        *(par2++) = '\0';   /* detach the second parameter from pcmd */
        while(*par2 == ' ') par2++;
        if(*par2 == '\0') par2 = NULL;
    }
    char *cbuf = NULL;
    x_malloc((byte **) &cbuf, FOS_BUF + 1);	/* buffer for the current path */
	x_free((byte **) &wbuf);
	x_malloc((byte **) &wbuf, FOS_BUF + 1);	/* working buffer for file names and path */
	if(dir && finfo && chb && cbuf && wbuf) {
        f_getcwd(cbuf, FOS_BUF);			/* store the current path */
        char *sep = strrchr(pcmd, '/');
        char *sep2 = strrchr(pcmd, '\\');
        sep = max(sep, sep2);
        if(sep) {   /* this assumes 'pcmd' is always given in RAM */
            *sep = '\0';
            f_chdir(pcmd);
            pcmd = sep + 1;
        }
        FRESULT fr = FR_OK;
        char *d = strchr(pcmd, ':');
        if(d) {
            if(fr == FR_OK) f_chdrive(pcmd);
            if(fr == FR_OK) fr = f_mount(&FatFs, pcmd, 1);
            pcmd = d + 1;
        }
        if(fr == FR_OK) f_getcwd(chb, FOS_BUF); /* get the new path */
        if(cmd == 'D' && !strcmp("*.*", pcmd)) *pcmd = '\0';
        if(*pcmd == '\0') fr = f_opendir(dir, "");
        if(fr == FR_OK && *pcmd) fr = f_findfirst(dir, finfo, chb, pcmd);
		if(fr == FR_OK && cmd == 'L') printf("\r\n\nDirectory %s\r\n\n", chb);
        FATFS *fs = &FatFs;
        unsigned long long p1 = 0;
		unsigned long s1 = 0, s2 = 0, fcnt;
		for(fcnt = 0 ; fr == FR_OK; fcnt++) {
            if(*pcmd == '\0') fr = f_readdir(dir, finfo);
			if(fr != FR_OK || finfo->fname[0] == '\0') break;
            if(cmd == 'L') {    /* list */
                if((finfo->fattrib & (AM_HID | AM_SYS)) == 0) { /* do not show hidden or system files */
                    if(finfo->fattrib & AM_DIR) s2++;
                    else { s1++; p1 += finfo->fsize; }
                    printf("%c%c%c%c%c %4u/%02u/%02u %02u:%02u ",
    		               (finfo->fattrib & AM_DIR) ? 'D' : '-',
    		               (finfo->fattrib & AM_RDO) ? 'R' : '-',
    		               (finfo->fattrib & AM_HID) ? 'H' : '-',
    		               (finfo->fattrib & AM_SYS) ? 'S' : '-',
    		               (finfo->fattrib & AM_ARC) ? 'A' : '-',
    		               (finfo->fdate >> 9) + 1980, (finfo->fdate >> 5) & 15, finfo->fdate & 31,
    		               (finfo->ftime >> 11), (finfo->ftime >> 5) & 63);
                    if(finfo->fattrib & AM_DIR) printf("     <DIR>   ");
                    else printf("%10lu   ", finfo->fsize);
                    char fn[9];
                    char *fnn = fn;
                    char *fnx = finfo->fname;
                    while(*fnx && *fnx != '.' && fnx < &finfo->fname[8]) *(fnn++) = *(fnx++);
                    *fnn = 0;
                    printf("%-8s ", fn);    /* print the file name (up to 8 characters) */
                    if(*(fnx++) == '.') {
                        fnn = fn;
                        while(*fnx && fnx < &finfo->fname[12]) *(fnn++) = *(fnx++);
                        *fnn = 0;
                        printf("%-3s", fn); /* print the extension (up to 3 characters) */
                    }
                    printf("\r\n");
                    if(fcnt >= 19 && (fcnt % 20) == 0) {
                        if(wait_for_brcont() == settings.brk_code) break;
                    }
                }
            }
            else if(cmd == 'D' && *pcmd) {  /* delete */
                s1++;
                if(fr == FR_OK) {
                    if(msgf) printf("\r\n%s%s", chb, finfo->fname);
                    fr = f_unlink(finfo->fname);
                }
            }
            else if(cmd == 'R' && *pcmd) {  /* rename */
                s1++;
                if(fr == FR_OK && par2) {
					char *p2 = wildname(pcmd, finfo->fname, par2, wbuf);
                    if(msgf) printf("\r\n%s%s -> %s", chb, finfo->fname, p2);
                    fr = f_rename(finfo->fname, p2);
                }
                else what();
            }
            else if(cmd == 'C' && *pcmd) {  /* copy */
                s1++;
                if(fr == FR_OK && par2) {
					char *p2 = wildname(pcmd, finfo->fname, par2, wbuf);
                    if(msgf) printf("\r\n%s%s -> %s", chb, finfo->fname, p2);
                    fr = fcopy(finfo->fname, p2);
        			if(fr == FR_OK && *pcmd) {	/* reset the fnext pointer and skip the previously found files */
						fr = f_findfirst(dir, finfo, chb, pcmd);
						unsigned long i;
						for(i = 0; fr == FR_OK && i < fcnt; i++) fr = f_findnext(dir, finfo);
					}
                }
                else what();
            }
            if(fr == FR_OK && *pcmd) fr = f_findnext(dir, finfo);
		}
		f_closedir(dir);
        if(cmd == 'L') {
            if(fr == FR_OK) printf("\r\n%4lu file(s), %12lu bytes total\r\n%4lu dir(s)", s1, (unsigned long) p1, s2);
            if(fr == FR_OK) fr = f_getfree(chb, (DWORD*) &p1, &fs);
    		if(fr == FR_OK ) {
    			printf(",  %12lu bytes free\r\n", (unsigned long) (p1 * fs->csize *
    				#if FF_MIN_SS != FF_MAX_SS
    					fs->ssize
    				#else
    					FF_MAX_SS
    				#endif
    			));
                printf("\r\n");
    		}
        }
        else if(cmd == 'D') {
            if(fr == FR_OK && msgf) printf("\r\n\n>>> %4lu file(s) deleted", s1);
        }
        else if(cmd == 'R' && par2) {
            if(fr == FR_OK && msgf) printf("\r\n\n>>> %4lu file(s) renamed", s1);
        }
        else if(cmd == 'C' && par2) {
            if(fr == FR_OK && msgf) printf("\r\n\n>>> %4lu file(s) copied", s1);
        }
		if(fr != FR_OK && msgf) errfile(fr, NULL);
        f_chdrive(cbuf);
        f_mount(&FatFs, cbuf, 1);
        f_chdir(cbuf);
    }
    else if(msgf) errmem();
	x_free((byte **) &finfo);
	x_free((byte **) &dir);
    x_free((byte **) &cbuf);
	x_free((byte **) &wbuf);
}


/* execute FOS command line */
void execute_cmd_fos(char *pcmd) {
    while(*pcmd == ' ') pcmd++;
    if(*pcmd == '\0') return;
    char *chb1 = NULL;
    x_malloc((byte **) &chb1, FOS_BUF + 1);  /* buffer for path and names */
    if(chb1) memset(chb1, 0, FOS_BUF + 1);

    #ifdef BOOTLOADER_APP
    /* enter bootloader mode */
    if(!strncmp("update", pcmd, 6)) bootloader();
    #endif

    /* init (format) drive */
    if(!strncmp("init", pcmd, 4)) {
        pcmd += 4; while(*pcmd == ' ') pcmd++;
        if(pcmd) {
            FATFS *fs = &FatFs;
            FRESULT fr = f_mount(fs, pcmd, 1);
            if(fr != FR_OK && fr != FR_NO_FILESYSTEM) {
                errfile(fr, "Unable to mount");
                return;
            }
            DWORD cl = 0, sz = FR_MKFS_ABORTED;
            char *rwbuf = NULL;
            x_malloc((byte **) &rwbuf, FF_MAX_SS);
            MKFS_PARM pp;
            memset(&pp, 0, sizeof(MKFS_PARM));
            pp.fmt = (FM_ANY | FM_SFD);
            if(rwbuf) {
                printf(" ... ");
                fr = f_mkfs(pcmd, &pp, rwbuf, FF_MAX_SS);
                x_free((byte **) &rwbuf);
                if(fr == FR_OK) {   /* get drive free space after initialisation */
                    fr = f_mount(fs, pcmd, 1);
                    if(fr == FR_OK) fr = f_getfree(pcmd, &cl, &fs);
                    if(fr == FR_OK) {
                        sz = ((unsigned long long) cl * fs->csize *
                            #if FF_MIN_SS != FF_MAX_SS
                                fs->ssize
                            #else
                                FF_MAX_SS
                            #endif
                        ) / 1024;
                    }
                    else errfile(fr, "Unable to read filesystem");
                }
                else errfile(fr, "Unable to create filesystem");
            }
            else errmem();
            if(fr == FR_OK && sz > 0) {
				printf("\r\n>>> %s initialised size %lu Kbytes\r\n", pcmd, (unsigned long) sz);
				if((*pcmd == 'i' || *pcmd == 'I') &&
					(*(pcmd+1) == 'f' || *(pcmd+1) == 'F') &&
					(*(pcmd+2) == 's' || *(pcmd+2) == 'S') &&
					(*(pcmd+3) == ':'))
						store_settings();
			}
        }
        else what();
    }

    /* change drive/directory */
    else if(!strncmp("chdir", pcmd, 5) || !strncmp("cd", pcmd, 2) || !strncmp("\003d", pcmd, 2)) {
        char mode = *(pcmd++);
        pcmd += ((*pcmd == 'h') ? 4 : 1);
        while(*pcmd == ' ') pcmd++;
        FRESULT fr = FR_OK;
        if(*pcmd) {
            char *d = strchr(pcmd, ':');
            if(d) {
                fr = f_chdrive(pcmd);
                if(fr == FR_OK) fr = f_mount(&FatFs, pcmd, 1);
                pcmd = d + 1;
            }
            if(fr == FR_OK) fr = f_chdir(pcmd);
            if(fr != FR_OK) errfile(fr, "Unable to change drive or directory");
        }
        else {
            if(chb1) {
                f_getcwd(chb1, FOS_BUF);
                if(mode != '\003') printf("\r\n>>> ");
                printf("Current path: %s", chb1);
                if(mode != '\003') printf("\r\n");
            }
            else errmem();
        }
    }

    /* delete files(s) */
    else if(!strncmp("del", pcmd, 3)) {
        pcmd += 3; while(*pcmd == ' ') pcmd++;
        group_exec('D', pcmd, chb1, 1);
    }

    /* rename files(s) */
    else if(!strncmp("ren", pcmd, 3)) {
        pcmd += 3; while(*pcmd == ' ') pcmd++;
        group_exec('R', pcmd, chb1, 1);
    }

    /* copy files(s) */
    else if(!strncmp("copy", pcmd, 4)) {
        pcmd += 4; while(*pcmd == ' ') pcmd++;
        group_exec('C', pcmd, chb1, 1);
    }

    /* list directory */
    else if(!strncmp("dir", pcmd, 3)) {
        pcmd += 3; while(*pcmd == ' ') pcmd++;
        group_exec('L', pcmd, chb1, 1);
    }

    /* make new directory */
    else if(!strncmp("mkdir", pcmd, 5)) {
        pcmd += 5; while(*pcmd == ' ') pcmd++;
        FATFS *fs = &FatFs;
        FRESULT fr = f_mount(fs, pcmd, 1);
        if(fr == FR_OK) f_mkdir(pcmd);
        if(fr != FR_OK) errfile(fr, "Unable to make directory");
    }

    /* remove directory */
    else if(!strncmp("rmdir", pcmd, 5)) {
        pcmd += 5; while(*pcmd == ' ') pcmd++;
        FATFS *fs = &FatFs;
        FRESULT fr = f_mount(fs, pcmd, 1);
        if(fr == FR_OK) f_rmdir(pcmd);
        if(fr != FR_OK) errfile(fr, "Unable to remove directory");
    }

	/* list text file */
	else if(!strncmp("list", pcmd, 4)) {
		pcmd += 4; while(*pcmd == ' ') pcmd++;
		FRESULT fr = FR_OK;
		byte *buf = NULL;
		FIL *ff = NULL;
		x_malloc((byte **) &buf, LINE_WIDTH + 1);
		x_malloc((byte **) &ff, sizeof(FIL));
		if(buf && ff) {
			fr = FR_OK;
	        char *d = strchr(pcmd, ':');
	        if(d) {
	            if(fr == FR_OK) f_chdrive(pcmd);
	            if(fr == FR_OK) fr = f_mount(&FatFs, pcmd, 1);
	            pcmd = d + 1;
	        }
			uint32_t ln = 0;
			uint32_t r0, r1;
			uint8_t *p, c;
			if(fr == FR_OK) fr = f_open(ff, pcmd, (FA_OPEN_EXISTING | FA_READ));
			if(fr == FR_OK) printf("\r\n\n%3lu: ", (unsigned long) ++ln);
			while(fr == FR_OK) {
				r0 = 0;
				fr = f_read(ff, buf, LINE_WIDTH, (UINT *) &r0);
				r1 = r0;
				p = buf;
				while(r1--) {
					char k = 0;
					while(kbhit() > 0 || (k == ' ')) {
						k = getchx();
						while(kbhit() > 0) getchx();
						if(k == settings.brk_code) break;
					}
					if(k == settings.brk_code) {
						printf("\r\n^cancel\r\n");
						ln = 0;
						break;
					}
					c = *(p++);
					if(c == 0) {
						ln = 0;
						break;
					}
					printf("%c", c);
					if(c == '\n') printf("\r%3lu: ", (unsigned long) ++ln);
				}
				if((ln == 0) || (r0 < LINE_WIDTH)) break;
			}
			f_close(ff);
		}
		else errmem();
		if(fr != FR_OK) errfile(fr, NULL);
		x_free((byte **) &buf);
		x_free((byte **) &ff);
	}

	/* list binary file */
	else if(!strncmp("blist", pcmd, 5)) {
		pcmd += 5; while(*pcmd == ' ') pcmd++;
		FRESULT fr = FR_OK;
		byte *buf = NULL;
		FIL *ff = NULL;
        uint8_t ch[settings.page_width / 5];    /* calculate the number of bytes per line */
        unsigned int BLIST_BUF_SIZE = sizeof(ch) * sizeof(ch);
		x_malloc((byte **) &buf, BLIST_BUF_SIZE);
		x_malloc((byte **) &ff, sizeof(FIL));
		if(buf && ff) {
			fr = FR_OK;
	        char *d = strchr(pcmd, ':');
	        if(d) {
	            if(fr == FR_OK) f_chdrive(pcmd);
	            if(fr == FR_OK) fr = f_mount(&FatFs, pcmd, 1);
	            pcmd = d + 1;
	        }
			uint32_t ln = 0;
			uint32_t r0, r1;
			uint8_t *p, z, f, pc = 0;
			if(fr == FR_OK) fr = f_open(ff, pcmd, (FA_OPEN_EXISTING | FA_READ));
			if(fr == FR_OK) printf("\r\n\n%06lX:", (unsigned long) ln);
			while(fr == FR_OK) {
				r0 = 0;
				fr = f_read(ff, buf, BLIST_BUF_SIZE, (UINT *) &r0);
				r1 = r0;
				p = buf;
				while(r1--) {
					char k = 0;
					while(kbhit() > 0 || (k == ' ')) {
						k = getchx();
						while(kbhit() > 0) getchx();
						if(k == settings.brk_code) break;
					}
					if(k == settings.brk_code) {
						printf("\r\n^cancel\r\n");
						ln = 0;
						break;
					}
					ch[pc++] = *p;
					printf(" %02X", (int) *(p++));
					if((r1 == 0) || (pc >= sizeof(ch))) {
						f = 1;
						while(pc < sizeof(ch)) {
							printf("   ");
							ch[pc++] = ' ';
							f = 0;
						}
						printf("  | ");
						for(z = 0; z < pc; z++) {
							if(ch[z] >= ' ' && ch[z] != 0x7F) printf("%c", ch[z]);
							else printf("%c", '.');
						}
						ln += pc; pc = 0;
						if(f) printf("\r\n%06lX:", (unsigned long) ln);
					}
				}
				if((ln == 0) || (r0 < BLIST_BUF_SIZE)) break;
			}
			f_close(ff);
		}
		else errmem();
		if(fr != FR_OK) errfile(fr, NULL);
		x_free((byte **) &buf);
		x_free((byte **) &ff);
	}

    else what();
    x_free((byte **) &chb1);
    printf("\r\n\n");
}
