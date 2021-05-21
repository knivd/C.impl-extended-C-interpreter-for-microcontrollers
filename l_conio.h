#ifdef CIMPL
#ifndef LIB_CONIO_H
#define LIB_CONIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../cimpl.h"
#include "l_stdio.h"

extern const sys_func_t conio_func_table[];

void sf_clrscr(void);
void sf_kbhit(void);
void sf_cscanf(void);
void sf_cprintf(void);

/* these are also included but inherited from the <stdio.h> library
void sf_getchar(void);	// also getch()
void sf_putchar(void);	// also putch()
void sf_cgets(void);
void sf_cputs(void);
*/

#ifdef __cplusplus
}
#endif

#endif  /* LIB_CONIO_H */
#endif
