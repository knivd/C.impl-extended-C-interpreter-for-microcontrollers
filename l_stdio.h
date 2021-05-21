#ifdef CIMPL
#ifndef LIB_STDIO_H
#define LIB_STDIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../cimpl.h"

/* FatFs library */
#if DISABLE_FILE == 0
#include "../../fatfs/source/ff.h"

FATFS *stdioFS;         /* FatFs volume area */
FIL *hFIL[MAX_FILES];   /* file handlers */

#endif

extern const sys_const_t stdio_const_table[];
extern const  sys_func_t stdio_func_table[];

void stdio_init(void);
void sf_printf(void);
void sf_sprintf(void);
void sf_scanf(void);
void sf_sscanf(void);
void sf_putchar(void);	/* also putch() */
void sf_puts(void);
void sf_getchar(void);	/* also getch() */
void sf_getche(void);
void sf_gets(void);

#if DISABLE_FILE == 0

/* void sf_ungetc(void); */
/* void sf_fscanf(void); */
void sf_fprintf(void);
void sf_fputc(void);
void sf_putc(void);
void sf_fputs(void);
void sf_fgetc(void);
void sf_getc(void);
void sf_fgets(void);
void sf_fread(void);
void sf_fwrite(void);
void sf_fopen(void);
void sf_fclose(void);
void sf_fflush(void);
void sf_freopen(void);
void sf_setbuf(void);
void sf_setvbuf(void);
void sf_remove(void);
void sf_rename(void);
void sf_tmpfile(void);
void sf_tmpnam(void);
void sf_fseek(void);
void sf_ftell(void);
void sf_rewind(void);
void sf_feof(void);
void sf_ferror(void);
void sf_clearerr(void);
void sf_run(void);      /* void run(const char *file_name) */

#endif  /* DISABLE_FILE check */

#define PRINTF_TYPE     0
#define SCANF_TYPE      1
#define SPRINTF_TYPE    2
#define SSCANF_TYPE     3
#define FPRINTF_TYPE    4
#define FSCANF_TYPE     5

/* internal helper functions for printf() and scanf() variants */
void ff_core(void *buf, uint8_t ftype);
void clear_kbd(void);

#ifdef __cplusplus
}
#endif

#endif  /* LIB_STDIO_H */
#endif
