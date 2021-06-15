#ifdef CIMPL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <time.h>
#include "../../xmem.h"
#include "../../config.h"
#include "l_stdio.h"
#include "../../ride/ride.h"    /* for getchx() */

/* FatFs library */
#if DISABLE_FILE == 0
#include "../../fatfs/source/ff.h"
#endif

/* PRINTF_CAST needs to be defined in the project options in case printf()/scanf() functions need to */
/* have their integer parameters pre-cast to a maximum type, such as (long). The reason for that is, */
/* some these functions in compilers like XC32 don't support long long int or don't perform automatic */
/* cast on the parameters, so the output may be just garbage */
#ifndef PRINTF_CAST
#define PRINTF_CAST
#endif

/* system function handlers take parameters from *prog and return result in acc[accN] */

const sys_const_t stdio_const_table[] = {
	{ { { .i = (uintptr_t) NULL },  (FT_CONST | FT_CONSTPTR | DT_VOID), 1, {0} },    "NULL", 4 },
    { { { .i = EOF },           (FT_CONST | DT_INT), 0, {0} },   "EOF",          3 },
    { { { .i = FOPEN_MAX },     (FT_CONST | DT_INT), 0, {0} },   "FOPEN_MAX",    9 },
    { { { .i = SEEK_SET },      (FT_CONST | DT_LLONG), 0, {0} }, "SEEK_SET",     8 },
    { { { .i = SEEK_CUR },      (FT_CONST | DT_LLONG), 0, {0} }, "SEEK_CUR",     8 },
    { { { .i = SEEK_END },      (FT_CONST | DT_LLONG), 0, {0} }, "SEEK_END",     8 },
    { { { .i = FILENAME_MAX },  (FT_CONST | DT_INT), 0, {0} },   "FILENAME_MAX", 12 },
	{ { {0}, 0, 0, {0} }, NULL, 0 }	/* must be final in this array */
};

const sys_func_t stdio_func_table[] = {
    { sf_printf,   "printf",   6,  "i,Cc*,.", NULL },
    { sf_sprintf,  "sprintf",  7,  "i,c*,Cc*,.", NULL },
    { sf_scanf,    "scanf",    5,  "i,Cc*,.", NULL },
    { sf_sscanf,   "sscanf",   6,  "i,Cc*,Cc*,.", NULL },
    { sf_putchar,  "putchar",  7,  "i,i", NULL },
	{ sf_putchar,  "putch",    5,  "i,i", NULL },
    { sf_getchar,  "getchar",  7,  "i,v", NULL },
	{ sf_getchar,  "getch",    5,  "i,v", NULL },
	{ sf_getche,   "getche",   6,  "i,v", NULL },
    { sf_puts,     "puts",     4,  "i,Cc*", NULL },
    { sf_gets,     "gets",     4,  "c*,c*", NULL },

    #if DISABLE_FILE == 0

    { sf_fprintf,  "fprintf",  7,  "i,F*,Cc*,.", NULL },
 /* { sf_fscanf,   "fscanf",   6,  "i,F*,Cc*,.", NULL }, */
    { sf_fputc,    "fputc",    5,  "i,i,F*", NULL },
    { sf_putc,     "putc",     4,  "i,i,F*", NULL },
    { sf_fputs,    "fputs",    5,  "i,Cc*,F*", NULL },
    { sf_fgetc,    "fgetc",    5,  "i,F*", NULL },
    { sf_getc,     "getc",     4,  "i,F*", NULL },
    { sf_fgets,    "fgets",    5,  "c*,c*,i,F*", NULL },
 /* { sf_ungetc,   "ungetc",   6,  "i,i,F*", NULL }, */
    { sf_fread,    "fread",    5,  "z,v*,z,z,F*", NULL },
    { sf_fwrite,   "fwrite",   6,  "z,Cv*,z,z,F*", NULL },
    { sf_fopen,    "fopen",    5,  "F*,Cc*,Cc*", NULL },
    { sf_fclose,   "fclose",   6,  "i,F*", NULL },
    { sf_fflush,   "fflush",   6,  "i,F*", NULL },
    { sf_freopen,  "freopen",  7,  "F*,Cc*,Cc*,F*", NULL },
    { sf_setbuf,   "setbuf",   6,  "v,F*,c*", NULL },
    { sf_setvbuf,  "setvbuf",  7,  "i,F*,c*,i,z", NULL },
    { sf_remove,   "remove",   6,  "i,Cc*", NULL },
    { sf_rename,   "rename",   6,  "i,Cc*,Cc*", NULL },
    { sf_tmpfile,  "tmpfile",  7,  "F*,v", NULL },
    { sf_tmpnam,   "tmpnam",   6,  "c*,c*", NULL },
    { sf_fseek,    "fseek",    5,  "i,F*,l,i", NULL },
    { sf_ftell,    "ftell",    5,  "l,F*", NULL },
    { sf_rewind,   "rewind",   6,  "v,F*", NULL },
    { sf_feof,     "feof",     4,  "i,F*", NULL },
    { sf_ferror,   "ferror",   6,  "i,F*", NULL },
    { sf_clearerr, "clearerr", 8,  "v,F*", NULL },
	{ sf_run,      "run",      3,  "v,Cc*", NULL },

    #endif

    {NULL, NULL, 0, NULL, NULL}
};


/* initialisation code */
void stdio_init(void) {
#if DISABLE_FILE == 0
    uint8_t buf;
    for(buf = 0; buf < MAX_FILES; buf++) x_free((byte **) &hFIL[buf]);
    x_free((byte **) &stdioFS);
    x_malloc((byte **) &stdioFS, sizeof(FATFS));
    if(stdioFS == NULL) error(MEMORY_ALLOCATION);
    FRESULT fr = f_mount(stdioFS, "IFS:", 0);
    if(fr != FR_OK && fr != FR_NO_FILESYSTEM)
        f_mount(stdioFS, "", 0);    /* mount NUL: if unsuccessful with IFS: */
#endif
}


/* clear the keyboard buffer */
void clear_kbd(void) {
	mSec(50);
	while(kbhit() > 0) getchx();	/* clear the keyboard buffer */
}


/* internal helper unction for printf() and scanf() variants */
/* 'ftype' specifies the function type of operation */
void ff_core(void *buf, uint8_t ftype) {
	char pbuf1[25], pbuf2[25];
    char *strfs = NULL, *fs, *fs0, *ps, *pd, *tmpbuf = NULL;
	int_t counter = 0;
    uint_t plen;
    data_t fmt, mt;
    get_param(&fmt, (FT_CONST | DT_CHAR), 1);	/* get the format specifier */
	x_malloc((byte **) &strfs, strlen(strbuf[curr_strbuf]) + 1);
	if(!strfs) error(MEMORY_ALLOCATION);
	fs = strfs;
	strcpy(fs, strbuf[curr_strbuf]);
    while(fs && *fs) {
        if(*fs != '%' || (*fs == '%' && *(fs+1) == '%')) {
            if(*fs == '%') fs++;    /* '%%' will produce a '%' character in the output */
            if(ftype == PRINTF_TYPE || ftype == SCANF_TYPE) counter += printf("%c", *(fs++));
            else if(ftype == SPRINTF_TYPE || ftype == SSCANF_TYPE) counter += sprintf(((char *)buf + counter), "%c", *(fs++));
            #if DISABLE_FILE == 0
            else if(ftype == FPRINTF_TYPE || ftype == FSCANF_TYPE) counter += f_printf((FIL *) buf, "%c", *(fs++));
            #endif
        }
        else {
            fs0 = fs;   /* save the start of the %-string */
            char t = 'i';
            char l = '\0';
            fs++;       /* skip the '%' character */
            while(*fs == '-' || *fs == '+' || *fs == ' ' || *fs == '#' || *fs == '0') fs++; /* flags */
            plen = 0;
            memset(pbuf1, 0, sizeof(pbuf1));
            memset(pbuf2, 0, sizeof(pbuf2));
            if(*fs == '*') {
                fs++;
                plen |= 1;
            }
            else {
                while(isdigitC(*fs)) fs++;      /* precision (whole) */
            }
            if(*fs == '.') {
                fs++;
                if(*fs == '*') {
                    fs++;
                    plen |= 2;
                }
                else {
                    while(isdigitC(*fs)) fs++;  /* precision (fraction) */
                }
            }
            if(*fs == 'h') {            /* 'h' and 'hh' length modifiers */
                fs++;
                if(*fs == 'h') fs++;
            }
            else if(*fs == 'l') {       /* 'l' and 'll' length modifiers */
                fs++;
                if(*fs == 'l') fs++;
            }
            else if(*fs == 'L') l = *(fs++);    /* 'L' length modifier */
            if(strchr("csdioxXufFeEaAgGnp", *fs)) t = *(fs++);
            if(plen & 1) {  /* get first * parameter */
                get_comma();
                get_value(1);
                if(!isINT(accN)) {
					x_free((byte **) &strfs);
					x_free((byte **) &tmpbuf);
					error(INVALID_DATA_TYPE);
				}
                if((uint_t) ival(accN) >= sizeof(pbuf1)) ival(accN) = (sizeof(pbuf1)-1);
                sprintf(pbuf1, "%u", (short) ival(accN));
            }
            if(plen & 2) {  /* get second * parameter */
                get_comma();
                get_value(1);
                if(!isINT(accN)) {
					x_free((byte **) &strfs);
					x_free((byte **) &tmpbuf);
					error(INVALID_DATA_TYPE);
				}
                if((uint_t) ival(accN) >= sizeof(pbuf1)) ival(accN) = (sizeof(pbuf1)-1);
                sprintf(pbuf2, "%u", (short) ival(accN));
            }
            plen = ((fs-fs0) + strlen(pbuf1) + strlen(pbuf2));
            if(*pbuf1) plen--;
            if(*pbuf2) plen--;
			x_free((byte **) &tmpbuf);
            x_malloc((byte **) &tmpbuf, plen + 2);	/* providing an extra byte in case an 'i' needs to be appended */
            if(!tmpbuf) {
				x_free((byte **) &strfs);
				error(MEMORY_ALLOCATION);
			}
            ps = fs0; pd = tmpbuf;
            while(plen--) {
                if(*ps != '*') *(pd++) = *(ps++);
                else {
                    ps++;   /* skip the '*' */
                    if(*pbuf1) {
                        strcpy(pd, pbuf1);
                        pd += strlen(pbuf1);
                        *pbuf1 = '\0';
                    }
                    else if(*pbuf2) {
                        strcpy(pd, pbuf2);
                        pd += strlen(pbuf2);
                        *pbuf2 = '\0';
                    }
                    else {
						x_free((byte **) &strfs);
						x_free((byte **) &tmpbuf);
						error(TOO_MANY_PARAMETERS);
					}
                }
            }
            *pd = '\0';
			skip_spaces(0);
            if(*prog == ',') {
				prog++;
                if(t == 'n') ival(accN) = counter;
                else {
                    get_value(1);   /* get the next supplied parameter */
                    memset(&mt, 0, sizeof(data_t));
                }
                if(strchr("fFeEaAgG", t)) { /* FP numbers */
                    #if DISABLE_LONG_DOUBLE != 0
                    if(l == 'L') l = '\0';  /* reduce long double to double */
                    #endif
                    if(l == 'L') {
                        mt.type = DT_LDOUBLE;
                        convert(&acc[accN], &mt);
						if(acc[accN].val.f == -0.0L) acc[accN].val.f = 0.0;
                        if(ftype == PRINTF_TYPE) counter += printf(tmpbuf, (ldouble_t) fval(accN));
                        else if(ftype == SCANF_TYPE) counter += scanf(tmpbuf, &fval(accN));
                        else if(ftype == SPRINTF_TYPE) counter += sprintf(((char *) buf + counter),
                                                                            tmpbuf, (ldouble_t) fval(accN));
                        else if(ftype == SSCANF_TYPE) counter += sscanf(((char *) buf + counter), tmpbuf, &fval(accN));
                        #if DISABLE_FILE == 0
                        else if(ftype == FPRINTF_TYPE) counter += f_printf((FIL *) buf, tmpbuf, (ldouble_t) fval(accN));
                        /* else if(ftype == FSCANF_TYPE) counter += f_scanf((FIL *) buf, tmpbuf, &fval(accN)); */
                        #endif
                    }
                    else {
                        mt.type = DT_DOUBLE;
                        convert(&acc[accN], &mt);
						if(acc[accN].val.f == -0.0) acc[accN].val.f = 0.0;
                        if(ftype == PRINTF_TYPE) counter += printf(tmpbuf, (double) fval(accN));
                        else if(ftype == SCANF_TYPE) counter += scanf(tmpbuf, &fval(accN));
                        else if(ftype == SPRINTF_TYPE) counter += sprintf(((char *) buf + counter),
                                                                            tmpbuf, (double) fval(accN));
                        else if(ftype == SSCANF_TYPE) counter += sscanf(((char *) buf + counter), tmpbuf, &fval(accN));
                        #if DISABLE_FILE == 0
                        else if(ftype == FPRINTF_TYPE) counter += f_printf((FIL *) buf, tmpbuf, (double) fval(accN));
                        /* else if(ftype == FSCANF_TYPE) counter += f_scanf((FIL *) buf, tmpbuf, &fval(accN)); */
                        #endif
                    }
                }
                else if(t == 's') { /* strings */
                    char *p = (char *) (uintptr_t) ival(accN);
                    if(ftype == PRINTF_TYPE || ftype == SPRINTF_TYPE || ftype == FPRINTF_TYPE) {
                        char ch;
                        do {
                            ch = get_char(&p);
                            if(ch) {
                                if(ftype == PRINTF_TYPE) counter += printf("%c", ch);
                                else if(ftype == SPRINTF_TYPE) counter += sprintf(((char *) buf + counter), "%c", ch);
                                #if DISABLE_FILE == 0
                                else if(ftype == FPRINTF_TYPE) counter += f_printf((FIL *) buf, "%c", ch);
                                #endif
                            }
                        } while(ch);
                    }
                    else if(ftype == SCANF_TYPE || ftype == SSCANF_TYPE || ftype == FSCANF_TYPE) {
                        if(ftype == SCANF_TYPE) counter += scanf("%s", p);
                        else if(ftype == SSCANF_TYPE) counter += sscanf(((char *) buf + counter), "%s", p);
                        #if DISABLE_FILE == 0
                        /* else if(ftype == FSCANF_TYPE) counter += f_scanf((FIL *) buf, "%s", p); */
                        #endif
                    }
                }
                else {  /* integers */
                    char *ch = tmpbuf + strlen(tmpbuf) - 1;
                    if(*ch == 'h' || *ch == 'l') {
                        *(ch+2) = '\0';
                        *(ch+1) = 'i';
                    }
                    #if DISABLE_LONG_LONG == 0
                    mt.type = DT_LLONG;
                    #else
                    mt.type = DT_LONG;  /* reduce long long int to long int */
                    #endif
                    if(strchr(tmpbuf, 'u') || strchr(tmpbuf, 'U')) mt.type |= FT_UNSIGNED;
                    convert(&acc[accN], &mt);
                    if(ftype == PRINTF_TYPE) counter += printf(tmpbuf, PRINTF_CAST ival(accN));
                    else if(ftype == SCANF_TYPE) counter += scanf(tmpbuf, &ival(accN));
                    else if(ftype == SPRINTF_TYPE) counter += sprintf(((char *) buf + counter), tmpbuf,
                                                                            PRINTF_CAST ival(accN));
                    else if(ftype == SSCANF_TYPE) counter += sscanf(((char *) buf + counter), tmpbuf, &ival(accN));
                    #if DISABLE_FILE == 0
                    else if(ftype == FPRINTF_TYPE) counter += f_printf((FIL *) buf, tmpbuf, PRINTF_CAST ival(accN));
                    /* else if(ftype == FSCANF_TYPE) counter += f_scanf((FIL *) buf, tmpbuf, &ival(accN)); */
                    #endif
                }
            }
        }
    }
	x_free((byte **) &strfs);
	x_free((byte **) &tmpbuf);
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
    ival(accN) = counter;
}


void sf_printf(void) {
    ff_core(NULL, PRINTF_TYPE);
}


void sf_sprintf(void) {
    get_param(&d1, DT_CHAR, 1);
    get_comma();
    ff_core((char *) (uintptr_t) d1.val.i, SPRINTF_TYPE);
}


void sf_scanf(void) {
	clear_kbd();
    ff_core(NULL, SCANF_TYPE);
	clear_kbd();
}


void sf_sscanf(void) {
    get_param(&d1, DT_CHAR, 1);
    get_comma();
    ff_core((char *) (uintptr_t) d1.val.i, SSCANF_TYPE);
}


void sf_putchar(void) {
    get_param(&d1, DT_INT, 0);
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
    uint16_t flags_temp = (enable_flags & FLAG_NO_CTRL);
    enable_flags |= FLAG_NO_CTRL;
    ival(accN) = printf("%c", (int) d1.val.i);
    enable_flags &= ~FLAG_NO_CTRL;
    enable_flags |= flags_temp;
}


void sf_puts(void) {
    get_param(&d1, (FT_CONST | DT_CHAR), 1);
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
    ival(accN) = puts((const char *) (uintptr_t) d1.val.i);
}


void sf_getchar(void) {
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
    uint16_t ee = (enable_flags & FLAG_NO_ECHO);
    enable_flags |= FLAG_NO_ECHO;
    ival(accN) = getchx();
    if(!ee) enable_flags &= ~FLAG_NO_ECHO;
}


void sf_getche(void) {
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
    ival(accN) = getchx();
}


void sf_gets(void) {
    get_param(&d1, DT_CHAR, 1);
    acc[accN].ind = 1;
    acc[accN].type = DT_CHAR;
    ival(accN) = (uintptr_t) gets((char *) (uintptr_t) d1.val.i);
	clear_kbd();
}


#if DISABLE_FILE == 0

void sf_run(void) {
    get_param(&d1, (FT_CONST | DT_CHAR), 1);    /* file name */
	file_to_run = (char *) (uintptr_t) d1.val.i;
	error(OK);
}


void sf_fprintf(void) {
    get_param(&d1, DT_FILE, 1);
    get_comma();
    ff_core((FIL *) (uintptr_t) d1.val.i, FPRINTF_TYPE);
}


void sf_fscanf(void) {
    get_param(&d1, DT_FILE, 1);
    get_comma();
    ff_core((FIL *) (uintptr_t) d1.val.i, FSCANF_TYPE);
}


void sf_fgetc(void) {
    get_param(&d1, DT_FILE, 1);
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
    ival(accN) = 0;
    UINT read;
    ival(accN) = f_read((FIL *) (uintptr_t) d1.val.i, (char *) &ival(accN), 1, &read);
}


void sf_fgets(void) {
    get_param(&d1, DT_CHAR, 1);
    get_comma();
    get_param(&d2, DT_INT, 0);
    get_comma();
    get_param(&d3, DT_FILE, 1);
    acc[accN].ind = 1;
    acc[accN].type = DT_CHAR;
    ival(accN) = (uintptr_t) f_gets((char *) (uintptr_t) d1.val.i, d2.val.i, (FIL *) (uintptr_t) d3.val.i);
}


void sf_fputc(void) {
    get_param(&d1, DT_INT, 0);
    get_comma();
    get_param(&d2, DT_FILE, 1);
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
    ival(accN) = f_putc(d1.val.i, (FIL *) (uintptr_t) d2.val.i);
}


void sf_fputs(void) {
    get_param(&d1, (FT_CONST | DT_CHAR), 1);
    get_comma();
    get_param(&d2, DT_FILE, 1);
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
    ival(accN) = f_puts((const char *) (uintptr_t) d1.val.i, (FIL *) (uintptr_t) d2.val.i);
}


void sf_getc(void) {
    sf_fgetc();
}


void sf_putc(void) {
    sf_fputc();
}


/* void sf_ungetc(void) {
    get_param(&d1, DT_INT, 0);
    get_comma();
    get_param(&d2, DT_FILE, 1);
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
    ival(accN) = ungetc(d1.val.i, (FILE *) (uintptr_t) d2.val.i);
} */


void sf_fread(void) {
    get_param(&d1, DT_VOID, 1);
    get_comma();
    get_param(&d2, DT_SIZE_T, 0);
    get_comma();
    get_param(&d3, DT_SIZE_T, 0);
    get_comma();
    get_param(&d4, DT_FILE, 1);
    acc[accN].ind = 0;
    acc[accN].type = DT_SIZE_T;
    ival(accN) = 0;
    ival(accN) = f_read((FIL *) (uintptr_t) d4.val.i, (void *) (uintptr_t) d1.val.i,
                                (d2.val.i * d3.val.i), (UINT *) &ival(accN));
}


void sf_fwrite(void) {
    get_param(&d1, (FT_CONST | DT_VOID), 1);
    get_comma();
    get_param(&d2, DT_SIZE_T, 0);
    get_comma();
    get_param(&d3, DT_SIZE_T, 0);
    get_comma();
    get_param(&d4, DT_FILE, 1);
    acc[accN].ind = 0;
    acc[accN].type = DT_SIZE_T;
    ival(accN) = 0;
    ival(accN) = f_write((FIL *) (uintptr_t) d4.val.i, (const void *) (uintptr_t) d1.val.i,
                                (d2.val.i * d3.val.i), (UINT *) &ival(accN));
}


void sf_fclose(void) {
    get_param(&d1, DT_FILE, 1);
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
    ival(accN) = f_close((FIL *) (uintptr_t) d1.val.i);
    x_free((byte **) &d1.val.i);    /* release the memory for this file handler */
}


void sf_fflush(void) {
    get_param(&d1, DT_FILE, 1);
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
    ival(accN) = f_sync((FIL *) (uintptr_t) d1.val.i);
}


void sf_setbuf(void) {
    get_param(&d1, DT_FILE, 1);
    get_comma();
    get_param(&d2, DT_CHAR, 1);
    /* setbuf((FILE *) (uintptr_t) d1.val.i, (char *) (uintptr_t) d2.val.i); */
}


void sf_setvbuf(void) {
    get_param(&d1, DT_FILE, 1);
    get_comma();
    get_param(&d2, DT_CHAR, 1);
    get_comma();
    get_param(&d3, DT_INT, 0);
    get_comma();
    get_param(&d4, DT_SIZE_T, 0);
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
    ival(accN) = 0; /* setvbuf((FILE *) (uintptr_t) d1.val.i, (char *) (uintptr_t) d2.val.i, d3.val.i, d4.val.i); */
}


/* helper function for fopen() to convert standard fopen() mode string into f_open() mode byte */
BYTE get_fopen_mode(const char *s) {
    char m[4];
    uint8_t x = 0;
    if(!s || !*s) return 0;
    memset(&m, 0, sizeof(m));
    while(*s && x < sizeof(m)) {
        m[x] = *(s++);
        if(m[x] != 'b') x++;    /* FatFs does not need a 'b' option */
    }
    if(!strcmp(m, "r")) return (FA_READ);
    else if(!strcmp(m, "r+")) return (FA_READ | FA_WRITE);
    else if(!strcmp(m, "w")) return (FA_CREATE_ALWAYS | FA_WRITE);
    else if(!strcmp(m, "w+")) return (FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
    else if(!strcmp(m, "a")) return (FA_OPEN_APPEND | FA_WRITE);
    else if(!strcmp(m, "a+")) return (FA_OPEN_APPEND | FA_WRITE | FA_READ);
    else if(!strcmp(m, "wx")) return (FA_CREATE_NEW | FA_WRITE);
    else if(!strcmp(m, "w+x") || !strcmp(m, "wx+")) return (FA_CREATE_NEW | FA_WRITE | FA_READ);
    else return 0;
}


void sf_fopen(void) {
    get_param(&d1, (FT_CONST | DT_CHAR), 1);
    get_comma();
    get_param(&d2, (FT_CONST | DT_CHAR), 1);
    acc[accN].ind = 1;
    acc[accN].type = DT_FILE;
    uint8_t t = 0;
    while(t < MAX_FILES && hFIL[t]) t++;
    FRESULT fr = FR_OK;
    if(t < MAX_FILES && !hFIL[t]) {
        x_malloc((byte **) &hFIL[t], sizeof(FIL));
        if(hFIL[t]) fr = f_open(hFIL[t], (const char *) (uintptr_t) d1.val.i,
                                    get_fopen_mode((const char *) (uintptr_t) d2.val.i));
        if(fr != FR_OK) {
            f_close(hFIL[t]);
            x_free((byte **) &hFIL[t]);
        }
    }
    ival(accN) = (uintptr_t) ((t < MAX_FILES && fr == FR_OK) ? hFIL[t] : NULL);
}


void sf_freopen(void) {
    get_param(&d1, (FT_CONST | DT_CHAR), 1);
    get_comma();
    get_param(&d2, (FT_CONST | DT_CHAR), 1);
    get_comma();
    get_param(&d3, DT_FILE, 1);
    acc[accN].ind = 1;
    acc[accN].type = DT_FILE;
    f_close((FIL *) (uintptr_t) d3.val.i);
    FRESULT fr = f_open((FIL *) (uintptr_t) d3.val.i, (const char *) (uintptr_t) d1.val.i,
                            get_fopen_mode((const char *) (uintptr_t) d2.val.i));
    if(fr != FR_OK) {
        f_close((FIL *) (uintptr_t) d3.val.i);
        x_free((byte **) &d3.val.i);
    }
    ival(accN) = ((fr == FR_OK) ? (uintptr_t) d3.val.i : (uintptr_t) NULL);
}


void sf_remove(void) {
    get_param(&d1, (FT_CONST | DT_CHAR), 1);
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
    ival(accN) = f_unlink((const char *) (uintptr_t) d1.val.i);
}


void sf_rename(void) {
    get_param(&d1, (FT_CONST | DT_CHAR), 1);
    get_comma();
    get_param(&d2, (FT_CONST | DT_CHAR), 1);
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
    ival(accN) = f_rename((const char *) (uintptr_t) d1.val.i, (const char *) (uintptr_t) d2.val.i);
}


void sf_tmpfile(void) {
    acc[accN].ind = 1;
    acc[accN].type = DT_FILE;
    ival(accN) = (uintptr_t) tmpfile();
}


void sf_tmpnam(void) {
    get_param(&d1, DT_CHAR, 1);
    acc[accN].ind = 1;
    acc[accN].type = DT_CHAR;
    ival(accN) = (uintptr_t) tmpnam((char *) (uintptr_t) d1.val.i);
}


void sf_fseek(void) {
    get_param(&d1, DT_FILE, 1);
    get_comma();
    get_param(&d2, DT_LONG, 0);
    get_comma();
    get_param(&d3, DT_INT, 0);
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
    if(d3.val.i == SEEK_END) ival(accN) = f_size((FIL *) (uintptr_t) d1.val.i) - d2.val.i;
    else if(d3.val.i == SEEK_CUR) ival(accN) = f_tell((FIL *) (uintptr_t) d1.val.i) + d2.val.i;
    else ival(accN) = f_lseek((FIL *) (uintptr_t) d1.val.i, d2.val.i);
}


void sf_ftell(void) {
    get_param(&d1, DT_FILE, 1);
    acc[accN].ind = 0;
    acc[accN].type = DT_LONG;
    ival(accN) = f_tell((FIL *) (uintptr_t) d1.val.i);
}


void sf_rewind(void) {
    get_param(&d1, DT_FILE, 1);
    f_lseek((FIL *) (uintptr_t) d1.val.i, 0);
}


void sf_clearerr(void) {
    get_param(&d1, DT_FILE, 1);
    ((FIL *) (uintptr_t) d1.val.i)->err = 0;
}


void sf_ferror(void) {
    get_param(&d1, DT_FILE, 1);
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
    ival(accN) = f_error((FIL *) (uintptr_t) d1.val.i);
}


void sf_feof(void) {
    get_param(&d1, DT_FILE, 1);
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
    ival(accN) = f_eof((FIL *) (uintptr_t) d1.val.i);
}

#endif  /* DISABLE_FILE check */

#endif
