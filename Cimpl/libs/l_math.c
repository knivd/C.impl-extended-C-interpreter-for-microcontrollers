#ifdef CIMPL

#include <stdlib.h>
#include <math.h>
#include "l_math.h"

const sys_const_t math_const_table[] = {
	{ { { .f = 3.14159265358979323846264338327950288L }, (FT_CONST | DT_LDOUBLE), 0, {0} }, "_PI", 3 },
	{ { { .f = 2.71828182845904523536028747135266249L }, (FT_CONST | DT_LDOUBLE), 0, {0} }, "_E", 2 },
	{ { { .f = HUGE_VAL },	(FT_CONST | DT_LDOUBLE), 0, {0} }, "INFINITY", 8 },
	{ { { .f = HUGE_VAL },	(FT_CONST | DT_LDOUBLE), 0, {0} }, "NaN", 3 },  /* actual value set in install_sys_library() */
	{ { {0}, 0, 0, {0} }, NULL, 0 }	/* must be final in this array */
};

const sys_func_t math_func_table[] = {
    { sf_sin,      "sin",      3,  "d,d", NULL },
    { sf_cos,      "cos",      3,  "d,d", NULL },
    { sf_tan,      "tan",      3,  "d,d", NULL },
    { sf_asin,     "asin",     4,  "d,d", NULL },
    { sf_acos,     "acos",     4,  "d,d", NULL },
    { sf_atan,     "atan",     4,  "d,d", NULL },
    { sf_atan2,    "atan2",    5,  "d,d,d", NULL },
    { sf_sinh,     "sinh",     4,  "d,d", NULL },
    { sf_cosh,     "cosh",     4,  "d,d", NULL },
    { sf_tanh,     "tanh",     4,  "d,d", NULL },
    { sf_asinh,    "asinh",    5,  "d,d", NULL },
    { sf_acosh,    "acosh",    5,  "d,d", NULL },
    { sf_atanh,    "atanh",    5,  "d,d", NULL },
    { sf_exp,      "exp",      3,  "d,d", NULL },
    { sf_frexp,    "frexp",    5,  "d,d,i*", NULL },
    { sf_ldexp,    "ldexp",    5,  "d,d,i", NULL },
    { sf_log,      "log",      3,  "d,d", NULL },
    { sf_log10,    "log10",    5,  "d,d", NULL },
    { sf_modf,     "modf",     4,  "d,d,d*", NULL },
    { sf_pow,      "pow",      3,  "d,d,d", NULL },
    { sf_sqrt,     "sqrt",     4,  "d,d", NULL },
    { sf_ceil,     "ceil",     4,  "d,d", NULL },
    { sf_floor,    "floor",    5,  "d,d", NULL },
    { sf_fmod,     "fmod",     4,  "d,d,d", NULL },
    { sf_fabs,     "fabs",     4,  "d,d", NULL },
    
#ifndef __XC32__
    { sf_trunc,    "trunc",    5,  "d,d", NULL },
    { sf_round,    "round",    5,  "d,d", NULL },
    { sf_lroundl,  "lroundl",  7,  "d,d", NULL },
    { sf_fmin,     "fmin",     4,  "d,d,d", NULL },
    { sf_fmax,     "fmax",     4,  "d,d,d", NULL },
#endif
    
    {NULL, NULL, 0, NULL, NULL}
};


void sf_sin(void) {
    get_param(&d1, DT_DOUBLE, 0);
    fval(accN) = sin(d1.val.f);
	acc[accN].ind = 0;
	acc[accN].type = DT_DOUBLE;
}


void sf_cos(void) {
    get_param(&d1, DT_DOUBLE, 0);
    fval(accN) = cosl(d1.val.f);
	acc[accN].ind = 0;
	acc[accN].type = DT_DOUBLE;
}


void sf_tan(void) {
    get_param(&d1, DT_DOUBLE, 0);
    fval(accN) = tan(d1.val.f);
	acc[accN].ind = 0;
	acc[accN].type = DT_DOUBLE;
}


void sf_asin(void) {
    get_param(&d1, DT_DOUBLE, 0);
    fval(accN) = asin(d1.val.f);
	acc[accN].ind = 0;
	acc[accN].type = DT_DOUBLE;
}


void sf_acos(void) {
    get_param(&d1, DT_DOUBLE, 0);
    fval(accN) = acos(d1.val.f);
	acc[accN].ind = 0;
	acc[accN].type = DT_DOUBLE;
}


void sf_atan(void) {
    get_param(&d1, DT_DOUBLE, 0);
    fval(accN) = atan(d1.val.f);
	acc[accN].ind = 0;
	acc[accN].type = DT_DOUBLE;
}


void sf_atan2(void) {
    get_param(&d1, DT_DOUBLE, 0);
    get_comma();
    get_param(&d2, DT_DOUBLE, 0);
    fval(accN) = atan2(d1.val.f, d2.val.f);
	acc[accN].ind = 0;
	acc[accN].type = DT_DOUBLE;
}


void sf_sinh(void) {
    get_param(&d1, DT_DOUBLE, 0);
    fval(accN) = sinh(d1.val.f);
	acc[accN].ind = 0;
	acc[accN].type = DT_DOUBLE;
}


void sf_cosh(void) {
    get_param(&d1, DT_DOUBLE, 0);
    fval(accN) = cosh(d1.val.f);
	acc[accN].ind = 0;
	acc[accN].type = DT_DOUBLE;
}


void sf_tanh(void) {
    get_param(&d1, DT_DOUBLE, 0);
    fval(accN) = tanh(d1.val.f);
	acc[accN].ind = 0;
	acc[accN].type = DT_DOUBLE;
}


void sf_asinh(void) {
    get_param(&d1, DT_DOUBLE, 0);
    fval(accN) = asinh(d1.val.f);
	acc[accN].ind = 0;
	acc[accN].type = DT_DOUBLE;
}


void sf_acosh(void) {
    get_param(&d1, DT_DOUBLE, 0);
    fval(accN) = acosh(d1.val.f);
	acc[accN].ind = 0;
	acc[accN].type = DT_DOUBLE;
}


void sf_atanh(void) {
    get_param(&d1, DT_DOUBLE, 0);
    fval(accN) = atanh(d1.val.f);
	acc[accN].ind = 0;
	acc[accN].type = DT_DOUBLE;
}


void sf_exp(void) {
    get_param(&d1, DT_DOUBLE, 0);
    fval(accN) = exp(d1.val.f);
	acc[accN].ind = 0;
	acc[accN].type = DT_DOUBLE;
}


void sf_frexp(void) {
    get_param(&d1, DT_DOUBLE, 0);
    get_comma();
    get_param(&d2, DT_INT, 1);
    acc[accN].ind = 0;
    fval(accN) = frexp(d1.val.f, (int *) (uintptr_t) d2.val.i);
	acc[accN].ind = 0;
	acc[accN].type = DT_DOUBLE;
}


void sf_ldexp(void) {
    get_param(&d1, DT_DOUBLE, 0);
    get_comma();
    get_param(&d2, DT_INT, 0);
    fval(accN) = ldexp(d1.val.f, d2.val.i);
	acc[accN].ind = 0;
	acc[accN].type = DT_DOUBLE;
}


void sf_log(void) {
    get_param(&d1, DT_DOUBLE, 0);
    fval(accN) = log(d1.val.f);
	acc[accN].ind = 0;
	acc[accN].type = DT_DOUBLE;
}


void sf_log10(void) {
    get_param(&d1, DT_DOUBLE, 0);
    fval(accN) = log10(d1.val.f);
	acc[accN].ind = 0;
	acc[accN].type = DT_DOUBLE;
}


void sf_modf(void) {
    get_param(&d1, DT_DOUBLE, 0);
    get_comma();
    get_param(&d2, DT_DOUBLE, 1);
    fval(accN) = modf(d1.val.f, (double *) (uintptr_t) d2.val.i);
	acc[accN].ind = 0;
	acc[accN].type = DT_DOUBLE;
}


void sf_pow(void) {
    get_param(&d1, DT_DOUBLE, 0);
    get_comma();
    get_param(&d2, DT_DOUBLE, 0);
    fval(accN) = pow(d1.val.f, d2.val.f);
	acc[accN].ind = 0;
	acc[accN].type = DT_DOUBLE;
}


void sf_sqrt(void) {
    get_param(&d1, DT_DOUBLE, 0);
    fval(accN) = sqrt(d1.val.f);
	acc[accN].ind = 0;
	acc[accN].type = DT_DOUBLE;
}


void sf_ceil(void) {
    get_param(&d1, DT_DOUBLE, 0);
    fval(accN) = ceil(d1.val.f);
	acc[accN].ind = 0;
	acc[accN].type = DT_DOUBLE;
}


void sf_floor(void) {
    get_param(&d1, DT_DOUBLE, 0);
    fval(accN) = floor(d1.val.f);
	acc[accN].ind = 0;
	acc[accN].type = DT_DOUBLE;
}


void sf_fmod(void) {
    get_param(&d1, DT_DOUBLE, 0);
    get_comma();
    get_param(&d2, DT_DOUBLE, 0);
    fval(accN) = fmod(d1.val.f, d2.val.f);
	acc[accN].ind = 0;
	acc[accN].type = DT_DOUBLE;
}


void sf_fabs(void) {
    get_param(&d1, DT_DOUBLE, 0);
    fval(accN) = fabs(d1.val.f);
	acc[accN].ind = 0;
	acc[accN].type = DT_DOUBLE;
}


#ifndef __XC32__    /* these are apparently not supported by the XC32 compiler */

void sf_trunc(void) {
    get_param(&d1, DT_DOUBLE, 0);
    fval(accN) = trunc(d1.val.f);
	acc[accN].ind = 0;
	acc[accN].type = DT_DOUBLE;
}


void sf_round(void) {
    get_param(&d1, DT_DOUBLE, 0);
    fval(accN) = round(d1.val.f);
	acc[accN].ind = 0;
	acc[accN].type = DT_DOUBLE;
}


void sf_lroundl(void) {
    get_param(&d1, DT_DOUBLE, 0);
    acc[accN].type = DT_LLONG;
    ival(accN) = lroundl(d1.val.f);
	acc[accN].ind = 0;
	acc[accN].type = DT_LLONG;
}


void sf_fmin(void) {
    get_param(&d1, DT_DOUBLE, 0);
    get_comma();
    get_param(&d2, DT_DOUBLE, 0);
    fval(accN) = fmin(d1.val.f, d2.val.f);
	acc[accN].ind = 0;
	acc[accN].type = DT_DOUBLE;
}


void sf_fmax(void) {
    get_param(&d1, DT_DOUBLE, 0);
    get_comma();
    get_param(&d2, DT_DOUBLE, 0);
    fval(accN) = fmax(d1.val.f, d2.val.f);
	acc[accN].ind = 0;
	acc[accN].type = DT_DOUBLE;
}

#endif

#endif
