#ifdef CIMPL

#include <limits.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "l_misc.h"

/* system function handlers take parameters from *prog and return result in acc[accN] */


/* stdbool.h ============================================================================== */

const sys_const_t stdbool_const_table[] = {
    { { { .i = 1 }, (FT_CONST | FT_UNSIGNED | DT_CHAR), 0, {0} },	"true",     4 },
    { { { .i = 1 }, (FT_CONST | FT_UNSIGNED | DT_CHAR), 0, {0} },   "TRUE",     4 },
    { { { .i = 0 }, (FT_CONST | FT_UNSIGNED | DT_CHAR), 0, {0} },   "false",    5 },
    { { { .i = 0 }, (FT_CONST | FT_UNSIGNED | DT_CHAR), 0, {0} },   "FALSE",	5 },
    { { {0}, 0, 0, {0} }, NULL, 0 }	/* must be final in this array */
};


/* limits.h ============================================================================== */

const sys_const_t limits_const_table[] = {
	{ { { .i = CHAR_BIT },     (FT_CONST | FT_UNSIGNED | DT_CHAR), 0, {0} },	"CHAR_BIT",     8 },
    { { { .i = SCHAR_MIN },    (FT_CONST | DT_CHAR), 0, {0} },                  "SCHAR_MIN",    9 },
    { { { .i = SCHAR_MAX },    (FT_CONST | DT_CHAR), 0, {0} },                  "SCHAR_MAX",    9 },
    { { { .i = UCHAR_MAX },    (FT_CONST | FT_UNSIGNED | DT_CHAR), 0, {0} },    "UCHAR_MAX",    9 },
    { { { .i = CHAR_MIN },     (FT_CONST | DT_CHAR), 0, {0} },                  "CHAR_MIN",     8 },
    { { { .i = CHAR_MAX },     (FT_CONST | DT_CHAR), 0, {0} },                  "CHAR_MAX",     8 },
    { { { .i = SHRT_MIN },     (FT_CONST | DT_SHORT), 0, {0} },                 "SHRT_MIN",     8 },
    { { { .i = SHRT_MAX },     (FT_CONST | DT_SHORT), 0, {0} },                 "SHRT_MAX",     8 },
    { { { .i = USHRT_MAX },    (FT_CONST | FT_UNSIGNED | DT_SHORT), 0, {0} },   "USHRT_MAX",    9 },
    { { { .i = INT_MIN },      (FT_CONST | DT_INT), 0, {0} },                   "INT_MIN",      7 },
    { { { .i = INT_MAX },      (FT_CONST | DT_INT), 0, {0} },                   "INT_MAX",      7 },
    { { { .i = UINT_MAX },     (FT_CONST | FT_UNSIGNED | DT_INT), 0, {0} },     "UINT_MAX",     8 },
    { { { .i = LONG_MIN },     (FT_CONST | DT_LONG), 0, {0} },                  "LONG_MIN",     8 },
    { { { .i = LONG_MAX },     (FT_CONST | DT_LONG), 0, {0} },                  "LONG_MAX",     8 },
    { { { .i = ULONG_MAX },    (FT_CONST | FT_UNSIGNED | DT_LONG), 0, {0} },    "ULONG_MAX",    9 },
    
    #if DISABLE_LONG_LONG == 0 && defined(LLONG_MIN)
    { { { .i = LLONG_MIN },    (FT_CONST | DT_LLONG), 0, {0} },                 "LLONG_MIN",    9 },
    { { { .i = LLONG_MAX },    (FT_CONST | DT_LLONG), 0, {0} },                 "LLONG_MAX",    9 },
    { { { .i = ULLONG_MAX },   (FT_CONST | FT_UNSIGNED | DT_LLONG), 0, {0} },   "ULLONG_MAX",   10 },
    #elif DISABLE_LONG_LONG < 0
    { { { .i = LONG_MIN },     (FT_CONST | DT_LLONG), 0, {0} },                 "LLONG_MIN",    9 },
    { { { .i = LONG_MAX },     (FT_CONST | DT_LLONG), 0, {0} },                 "LLONG_MAX",    9 },
    { { { .i = ULONG_MAX },    (FT_CONST | FT_UNSIGNED | DT_LLONG), 0, {0} },   "ULLONG_MAX",   10 },
    #endif    

    { { {0}, 0, 0, {0} }, NULL, 0 }	/* must be final in this array */
};


/* stdint.h ============================================================================== */

const sys_const_t stdint_const_table[] = {
	{ { { .i = INTMAX_MIN },    (FT_CONST | DT_INT), 0, {0} },                  "INTMAX_MIN",   10 },
    { { { .i = INTMAX_MAX },    (FT_CONST | DT_INT), 0, {0} },                  "INTMAX_MAX",   10 },
    { { { .i = UINTMAX_MAX },   (FT_CONST | FT_UNSIGNED | DT_INT), 0, {0} },    "UINTMAX_MAX",  11 },
    { { { .i = INTPTR_MIN },    (FT_CONST | DT_INT), 0, {0} },                  "INTPTR_MIN",   10 },
    { { { .i = INTPTR_MAX },    (FT_CONST | DT_INT), 0, {0} },                  "INTPTR_MAX",   10 },
    { { { .i = UINTPTR_MAX },   (FT_CONST | FT_UNSIGNED | DT_INT), 0, {0} },    "UINTPTR_MAX",  11 },
    { { { .i = INT8_MIN },      (FT_CONST | DT_CHAR), 0, {0} },                 "INT8_MIN",     8 },
    { { { .i = INT8_MAX },      (FT_CONST | DT_CHAR), 0, {0} },                 "INT8_MAX",     8 },
    { { { .i = UINT8_MAX },     (FT_CONST | FT_UNSIGNED | DT_CHAR), 0, {0} },   "UINT8_MAX",    9 },
    { { { .i = INT16_MIN },     (FT_CONST | DT_SHORT), 0, {0} },                "INT16_MIN",    9 },
    { { { .i = INT16_MAX },     (FT_CONST | DT_SHORT), 0, {0} },                "INT16_MAX",    9 },
    { { { .i = UINT16_MAX },    (FT_CONST | FT_UNSIGNED | DT_SHORT), 0, {0} },  "UINT16_MAX",   10 },
    { { { .i = INT32_MIN },     (FT_CONST | DT_LONG), 0, {0} },                 "INT32_MIN",    9 },
    { { { .i = INT32_MAX },     (FT_CONST | DT_LONG), 0, {0} },                 "INT32_MAX",    9 },
    { { { .i = UINT32_MAX },    (FT_CONST | FT_UNSIGNED | DT_LONG), 0, {0} },   "UINT32_MAX",   10 },
    
    #if DISABLE_LONG_LONG == 0
    { { { .i = INT64_MIN },     (FT_CONST | DT_LLONG), 0, {0} },                "INT64_MIN",    9 },
    { { { .i = INT64_MAX },     (FT_CONST | DT_LLONG), 0, {0} },                "INT64_MAX",    9 },
    { { { .i = UINT64_MAX },    (FT_CONST | DT_LLONG), 0, {0} },                "UINT64_MAX",   10 },
    #endif

    { { { .i = SIZE_MAX },      (FT_CONST | FT_UNSIGNED | DT_INT), 0, {0} },    "SIZE_MAX",     8 },
    { { { .i = PTRDIFF_MIN },   (FT_CONST | DT_INT), 0, {0} },                  "PTRDIFF_MIN",  11 },
    { { { .i = PTRDIFF_MAX },   (FT_CONST | DT_INT), 0, {0} },                  "PRTDIFF_MAX",  11 },
    { { {0}, 0, 0, {0} }, NULL, 0 }	/* must be final in this array */
};


/* stdarg.h ============================================================================== */

const sys_func_t stdarg_func_table[] = {
    { sf_va_start, "va_start", 8,  ":void va_start(va_list, pN)", NULL },
    { sf_va_end,   "va_end",   6,  ":void va_end(va_list)", NULL },
    { sf_va_arg,   "va_arg",   6,  ":type va_arg(va_list, type)", NULL },
    {NULL, NULL, 0, NULL, NULL}
};


void sf_va_start(void) {
	var_t *v = get_token();
	if(token != IDENTIFIER || !v || v->data.type != DT_VA_LIST) error(SYNTAX);
	ival(accN) = (uintptr_t) ellipsis_ptr;
	acc[accN].type = DT_VA_LIST;
	acc[accN].ind = 0;
	memset(&acc[accN].dim, 0, sizeof(acc[accN].dim));
	var_set(v, &acc[accN], 0);
	get_comma();
	v = get_token();
	if(token != IDENTIFIER || !v) error(SYNTAX);
}


void sf_va_end(void) {
	var_t *v = get_token();
	if(token != IDENTIFIER || !v || v->data.type != DT_VA_LIST) error(SYNTAX);
	memset(&acc[accN], 0, sizeof(acc[accN]));
	acc[accN].type = DT_VA_LIST;
	var_set(v, &acc[accN], 0);
}


void sf_va_arg(void) {
	var_t *v = get_token();
	if(token != IDENTIFIER || !v || v->data.type != DT_VA_LIST) error(SYNTAX);
	incN();		/* acc[accN+1] is the va_list */
	memset(&acc[accN].dim, 0, sizeof(acc[accN].dim));
	acc[accN].ind = 0;
	var_get(v);
	if(ival(accN) == 0) error(INVALID_ELLIPSIS);
	get_comma();
	incN();		/* acc[accN+2] is the required data type */
	get_token();
	if(token != DATA_TYPE) error(DATA_TYPE_EXPECTED);
	accN--;		/* back to the va_list */
	char *pt = prog; prog = (char *) (uintptr_t) ival(accN);
	accN--;		/* acc[accN] will be the returned value */
	get_comma();
	get_value(1);
	convert(&acc[accN], &acc[accN + 2]);	/* convert the data to the required type */
	incN();		/* to the va_list */
	ival(accN) = (uintptr_t) prog;			/* update the ellipsis pointer */
	var_set(v, &acc[accN], 0);
	accN--;		/* back to the original accN index */
	prog = pt;	
}


/* ctype.h ============================================================================== */

const sys_func_t ctype_func_table[] = {
    { sf_tolower,  "tolower",  7,  "i,i", NULL },
    { sf_toupper,  "toupper",  7,  "i,i", NULL },
    { sf_isalnum,  "isalnum",  7,  "i,i", NULL },
    { sf_isalpha,  "isalpha",  7,  "i,i", NULL },
    { sf_isblank,  "isblank",  7,  "i,i", NULL },
    { sf_iscntrl,  "iscntrl",  7,  "i,i", NULL },
    { sf_isdigit,  "isdigit",  7,  "i,i", NULL },
    { sf_isgraph,  "isgraph",  7,  "i,i", NULL },
    { sf_islower,  "islower",  7,  "i,i", NULL },
    { sf_isupper,  "isupper",  7,  "i,i", NULL },
    { sf_isprint,  "isprint",  7,  "i,i", NULL },
    { sf_ispunct,  "ispunct",  7,  "i,i", NULL },
    { sf_isspace,  "isspace",  7,  "i,i", NULL },
    { sf_isxdigit, "isxdigit", 8,  "i,i", NULL },
    {NULL, NULL, 0, NULL, NULL}
};


void sf_tolower(void) {
    get_param(&d1, DT_INT, 0);
    ival(accN) = tolower(d1.val.i);
}


void sf_toupper(void) {
    get_param(&d1, DT_INT, 0);
    ival(accN) = toupper(d1.val.i);
}


void sf_isalpha(void) {
    get_param(&d1, DT_INT, 0);
    ival(accN) = ((d1.val.i >= 0x41 && d1.val.i <= 0x5A) || (d1.val.i >= 0x61 && d1.val.i <= 0x7A));
}


void sf_isalnum(void) {
    get_param(&d1, DT_INT, 0);
    ival(accN) = ((d1.val.i >= 0x30 && d1.val.i <= 0x39) || (d1.val.i >= 0x41 && d1.val.i <= 0x5A) ||
                            (d1.val.i >= 0x61 && d1.val.i <= 0x7A));
}


void sf_isblank(void) {
    get_param(&d1, DT_INT, 0);
    ival(accN) = (d1.val.i == 0x09 || d1.val.i == 0x20);
}


void sf_iscntrl(void) {
    get_param(&d1, DT_INT, 0);
    ival(accN) = ((d1.val.i >= 0x00 && d1.val.i <= 0x1F) || d1.val.i == 0x7F);
}


void sf_isdigit(void) {
    get_param(&d1, DT_INT, 0);
    ival(accN) = (d1.val.i >= 0x30 && d1.val.i <= 0x39);
}


void sf_isgraph(void) {
    get_param(&d1, DT_INT, 0);
    ival(accN) = (d1.val.i >= 0x21 && d1.val.i <= 0x7E);
}


void sf_isprint(void) {
    get_param(&d1, DT_INT, 0);
    ival(accN) = (d1.val.i >= 0x20 && d1.val.i <= 0x7E);
}


void sf_islower(void) {
    get_param(&d1, DT_INT, 0);
    ival(accN) = (d1.val.i >= 0x61 && d1.val.i <= 0x7A);
}


void sf_isupper(void) {
    get_param(&d1, DT_INT, 0);
    ival(accN) = (d1.val.i >= 0x41 && d1.val.i <= 0x5A);
}


void sf_ispunct(void) {
    get_param(&d1, DT_INT, 0);
    ival(accN) = ((d1.val.i >= 0x21 && d1.val.i <= 0x2F) || (d1.val.i >= 0x3A && d1.val.i <= 0x40) ||
                            (d1.val.i >= 0x5B && d1.val.i <= 0x60) || (d1.val.i >= 0x7B && d1.val.i <= 0x7E));
}


void sf_isspace(void) {
    get_param(&d1, DT_INT, 0);
    ival(accN) = ((d1.val.i >= 0x09 && d1.val.i <= 0x0D) || d1.val.i == 0x20);
}


void sf_isxdigit(void) {
    get_param(&d1, DT_INT, 0);
    ival(accN) = ((d1.val.i >= 0x30 && d1.val.i <= 0x39) || (d1.val.i >= 0x41 && d1.val.i <= 0x46) ||
                            (d1.val.i >= 0x61 && d1.val.i <= 0x66));
}


/* time.h ============================================================================== */

const sys_const_t time_const_table[] = {
    { { { .i = 1000 }, (FT_CONST | FT_UNSIGNED | DT_INT), 0, {0} },  "CLOCKS_PER_SEC",   14 },
    { { {0}, 0, 0, {0} }, NULL, 0 }	/* must be final in this array */
};

const sys_func_t time_func_table[] = {
    { sf_clock,    "clock",    5,  "ul,v", NULL },
    { sf_time,     "time",     4,  "t,t*", NULL },
    { sf_difftime, "difftime", 8,  "d,t,t", NULL },
    { sf_mktime,   "mktime",   6,  "t,m*", NULL },
    { sf_asctime,  "asctime",  7,  "c*,Cm*", NULL },
    { sf_ctime,    "ctime",    5,  "c*,Ct*", NULL },
    {NULL, NULL, 0, NULL, NULL}
};


void sf_clock(void) {
    acc[accN].ind = 0;
    acc[accN].type = (FT_UNSIGNED | DT_LONG);
    ival(accN) = clock();
}


void sf_difftime(void) {
    get_param(&d1, (FT_UNSIGNED | DT_LONG), 0);
    get_comma();
    get_param(&d2, (FT_UNSIGNED | DT_LONG), 0);
    acc[accN].ind = 0;
    acc[accN].type = DT_DOUBLE;
    fval(accN) = difftime(d1.val.i, d2.val.i);
}


void sf_mktime(void) {
    get_param(&d1, DT_STRUCT, 1);
    acc[accN].ind = 0;
    acc[accN].type = (FT_UNSIGNED | DT_LONG);
    ival(accN) = mktime((struct tm *) (uintptr_t) d1.val.i);
}


void sf_time(void) {
    get_param(&d1, (FT_UNSIGNED | DT_LONG), 1);
    acc[accN].ind = 0;
    acc[accN].type = (FT_UNSIGNED | DT_LONG);
    ival(accN) = time((time_t *) (uintptr_t) d1.val.i);
}


void sf_asctime(void) {
    get_param(&d1, (FT_CONST | DT_STRUCT), 1);
    acc[accN].ind = 1;
    acc[accN].type = DT_CHAR;
    ival(accN) = (uintptr_t) asctime((const struct tm *) (uintptr_t) d1.val.i);
}


void sf_ctime(void) {
    get_param(&d1, (FT_CONST | FT_UNSIGNED | DT_LONG), 1);
    acc[accN].ind = 1;
    acc[accN].type = DT_CHAR;
    ival(accN) = (uintptr_t) ctime((const time_t *) (uintptr_t) d1.val.i);
}


/* assert.h ============================================================================== */

const sys_func_t assert_func_table[] = {
    { sf_assert,   "assert",   6,  "v,.", NULL },
	{ sf_error,    "error",    5,  "v,i,Cc*", NULL },
    {NULL, NULL, 0, NULL, NULL}
};


void sf_assert(void) {
	uint8_t af = assert_flag; assert_flag = 1;
	get_value(0);
	if(ival(accN) == 0 || fval(accN) == 0.0) error(ASSERTION_FAILED);
	assert_flag = af;
}


void sf_error(void) {
	get_param(&d1, DT_INT, 0);
    get_comma();
    get_param(&d2, (FT_CONST | DT_CHAR), 1);
	if(d2.val.i) printf("\r\n%s", (char *) (uintptr_t) d2.val.i);
	error((error_t) d1.val.i);
}

#endif
