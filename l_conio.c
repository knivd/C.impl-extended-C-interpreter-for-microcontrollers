#ifdef CIMPL

#include <conio.h>	/* needed for getch() and kbhit() */
#include "l_conio.h"

/* system function handlers take parameters from *prog and return result in acc[accN] */

const sys_func_t conio_func_table[] = {
	{ sf_clrscr,   "clrscr",   6,  "v,v", NULL },
	{ sf_kbhit,    "kbhit",    5,  "i,v", NULL },
	{ sf_getchar,  "getch",    5,  "i,v", NULL },
	{ sf_putchar,  "putch",    5,  "i,i", NULL },
	{ sf_gets,     "gets",     4,  "c*,c*", NULL },
	{ sf_puts,     "puts",     4,  "i,Cc*", NULL },
    { sf_cscanf,   "cscanf",   6,  "i,Cc*,.", NULL },
	{ sf_cprintf,  "cprintf",  7,  "i,Cc*,.", NULL },
    {NULL, NULL, 0, NULL, NULL}
};


void sf_kbhit(void) {
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
    ival(accN) = kbhit();
}


void sf_clrscr(void) {
	printf("\r");
	int ch = 100; while(ch--) printf("\n");
}


void sf_cprintf(void) {
	ff_core(NULL, PRINTF_TYPE);
}


void sf_cscanf(void) {
	ff_core(NULL, SCANF_TYPE);
	clear_kbd();
}

#endif
