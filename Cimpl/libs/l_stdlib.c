#ifdef CIMPL

#include <stdlib.h>
#include "l_stdlib.h"
#include "../../xmem.h"
#include "../../RIDE/ride.h"    /* for the settings{} structure */

/* system function handlers take parameters from *prog and return result in acc[accN] */

const sys_const_t stdlib_const_table[] = {
    { { { .i = (uintptr_t) NULL },  (FT_CONST | FT_CONSTPTR | DT_VOID), 1, {0} },    "NULL", 4 },
    { { { .i = RAND_MAX },      (FT_CONST | DT_INT), 0, {0} },	"RAND_MAX",     8 },
    { { { .i = EXIT_SUCCESS },  (FT_CONST | DT_INT), 0, {0} },  "EXIT_SUCCESS",	12 },
    { { { .i = EXIT_FAILURE },  (FT_CONST | DT_INT), 0, {0} },  "EXIT_FAILURE", 12 },
    { { {0}, 0, 0, {0} }, NULL, 0 }	/* must be final in this array */
};

const sys_func_t stdlib_func_table[] = {
    { sf_BIT,      "BIT",      3,  "uL,uc", NULL },
    { sf_exit,     "exit",     4,  "v,i", NULL },
    { sf_rand,     "rand",     4,  "i,v", NULL },
    { sf_srand,    "srand",    5,  "v,i", NULL },
    { sf_atof,     "atof",     4,  "d,c*", NULL },
    { sf_atoi,     "atoi",     4,  "i,c*", NULL },
    { sf_atol,     "atol",     4,  "l,c*", NULL },
    { sf_atoll,    "atoll",    5,  "L,c*", NULL },
    { sf_strtod,   "strtod",   6,  "d,c*,c**", NULL },
    { sf_strtol,   "strtol",   6,  "l,c*,c**,i", NULL },
    { sf_strtoul,  "strtoul",  7,  "ul,c*,c**,i", NULL },
    { sf_malloc,   "malloc",   6,  "v*,z", NULL },
    { sf_calloc,   "calloc",   6,  "v*,z,z", NULL },
    { sf_realloc,  "realloc",  7,  "v*,v*,z", NULL },
    { sf_free,     "free",     4,  "v,v*", NULL },
    { sf_abs,      "abs",      3,  "i,i", NULL },
    { sf_labs,     "labs",     4,  "l,l", NULL },

	#if DISABLE_LONG_LONG <= 0
    { sf_llabs,    "llabs",    5,  "L,L", NULL },
	{ sf_strtoll,  "strtoll",  7,  "L,c*,c**,i", NULL },
    { sf_strtoull, "strtoull", 8,  "uL,c*,c**,i", NULL },
	#endif

	#if DISABLE_LONG_DOUBLE <= 0
    { sf_strtold,  "strtold",  7,  "D,c*,c**", NULL },
    #endif

    {NULL, NULL, 0, NULL, NULL}
};


void sf_BIT(void) {
    get_param(&d1, (FT_UNSIGNED | DT_CHAR), 0);
    acc[accN].ind = 0;
    acc[accN].type = (FT_UNSIGNED | DT_LONG);
    ival(accN) = (1ul << d1.val.i);
}


void sf_exit(void) {
    get_param(&d1, (DT_INT), 0);    /* ignored */
    settings.brk_code = -1;
}


void sf_rand(void) {
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
    ival(accN) = rand();
}


void sf_srand(void) {
    get_param(&d1, DT_INT, 0);
    srand(d1.val.i);
}


void sf_atof(void) {
    get_param(&d1, DT_CHAR, 1);
    acc[accN].ind = 0;
    acc[accN].type = DT_DOUBLE;
    fval(accN) = atof((char *) (uintptr_t) d1.val.i);
}


void sf_atoi(void) {
    get_param(&d1, DT_CHAR, 1);
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
    ival(accN) = atoi((char *) (uintptr_t) d1.val.i);
}


void sf_atol(void) {
    get_param(&d1, DT_CHAR, 1);
    acc[accN].ind = 0;
    acc[accN].type = DT_LONG;
    ival(accN) = atol((char *) (uintptr_t) d1.val.i);
}


#if DISABLE_LONG_LONG == 0
void sf_atoll(void) {
    get_param(&d1, DT_CHAR, 1);
    acc[accN].ind = 0;
    acc[accN].type = DT_LLONG;
    ival(accN) = atoll((char *) (uintptr_t) d1.val.i);
}
#elif DISABLE_LONG_LONG < 0
void sf_atoll(void) {
    sf_atol();
}
#endif


void sf_strtod(void) {
    get_param(&d1, DT_CHAR, 1);
    get_comma();
    get_param(&d2, DT_CHAR, 2);
    acc[accN].ind = 0;
    acc[accN].type = DT_DOUBLE;
    fval(accN) = strtod((char *) (uintptr_t) d1.val.i, (char **) (uintptr_t) d2.val.i);
}


#if DISABLE_LONG_DOUBLE == 0
void sf_strtold(void) {
    get_param(&d1, DT_CHAR, 1);
    get_comma();
    get_param(&d2, DT_CHAR, 2);
    acc[accN].ind = 0;
    acc[accN].type = DT_LDOUBLE;
    fval(accN) = strtold((char *) (uintptr_t) d1.val.i, (char **) (uintptr_t) d2.val.i);
}
#elif DISABLE_LONG_DOUBLE < 0
void sf_strtold(void) {
    sf_strtod(void);
}
#endif


void sf_strtol(void) {
    get_param(&d1, DT_CHAR, 1);
    get_comma();
    get_param(&d2, DT_CHAR, 2);
    get_comma();
    get_param(&d3, DT_INT, 0);
    acc[accN].ind = 0;
    acc[accN].type = DT_LONG;
    ival(accN) = strtol((char *) (uintptr_t) d1.val.i, (char **) (uintptr_t) d2.val.i, d3.val.i);
}


void sf_strtoul(void) {
    get_param(&d1, DT_CHAR, 1);
    get_comma();
    get_param(&d2, DT_CHAR, 2);
    get_comma();
    get_param(&d3, DT_INT, 0);
    acc[accN].ind = 0;
    acc[accN].type = (FT_UNSIGNED | DT_LONG);
    ival(accN) = strtoul((char *) (uintptr_t) d1.val.i, (char **) (uintptr_t) d2.val.i, d3.val.i);
}


#if DISABLE_LONG_LONG == 0
void sf_strtoll(void) {
    get_param(&d1, DT_CHAR, 1);
    get_comma();
    get_param(&d2, DT_CHAR, 2);
    get_comma();
    get_param(&d3, DT_INT, 0);
    acc[accN].ind = 0;
    acc[accN].type = DT_LLONG;
    ival(accN) = strtoll((char *) (uintptr_t) d1.val.i, (char **) (uintptr_t) d2.val.i, d3.val.i);
}
#elif DISABLE_LONG_LONG < 0
void sf_strtoll(void) {
    sf_strtol();
}
#endif


#if DISABLE_LONG_LONG == 0
void sf_strtoull(void) {
    get_param(&d1, DT_CHAR, 1);
    get_comma();
    get_param(&d2, DT_CHAR, 2);
    get_comma();
    get_param(&d3, DT_INT, 0);
    acc[accN].ind = 0;
    acc[accN].type = (FT_UNSIGNED | DT_LLONG);
    ival(accN) = strtoull((char *) (uintptr_t) d1.val.i, (char **) (uintptr_t) d2.val.i, d3.val.i);
}
#elif DISABLE_LONG_LONG < 0
void sf_strtoull(void) {
    sf_strtoul();
}
#endif


void sf_malloc(void) {
    char *p = NULL;
    get_param(&d1, DT_SIZE_T, 0);
    x_malloc((byte **) &p, d1.val.i);
    ival(accN) = (uintptr_t) p;
}


void sf_calloc(void) {
    char *p = NULL;
    get_param(&d1, DT_SIZE_T, 0);
    get_comma();
    get_param(&d2, DT_SIZE_T, 0);
    acc[accN].ind = 1;
    acc[accN].type = DT_VOID;
    x_malloc((byte **) &p, (d1.val.i * d2.val.i));
    ival(accN) = (uintptr_t) p;
}


void sf_realloc(void) {
    get_param(&d1, DT_VOID, 1);
    char *p = (char *) (uintptr_t) d1.val.i;
    get_comma();
    get_param(&d2, DT_SIZE_T, 0);
    acc[accN].ind = 1;
    acc[accN].type = DT_VOID;
    x_malloc((byte **) &p, d2.val.i);
    ival(accN) = (uintptr_t) p;
}


void sf_free(void) {
    get_param(&d1, DT_VOID, 1);
    char *p = (char *) (uintptr_t) d1.val.i;
    x_free((byte **) &p);
}


void sf_abs(void) {
    get_param(&d1, DT_INT, 0);
    ival(accN) = abs((int) d1.val.i);
}


void sf_labs(void) {
    get_param(&d1, DT_LONG, 0);
    ival(accN) = labs((long) d1.val.i);
}


#if DISABLE_LONG_LONG == 0
void sf_llabs(void) {
    get_param(&d1, DT_LLONG, 0);
    ival(accN) = llabs(d1.val.i);
}
#elif DISABLE_LONG_LONG < 0
void sf_llabs(void) {
    sf_labs();
}
#endif

#endif
