#ifdef CIMPL
#ifndef LIB_MATH_H
#define LIB_MATH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../cimpl.h"

extern const sys_const_t math_const_table[];
extern const sys_func_t math_func_table[];

void sf_sin(void);
void sf_cos(void);
void sf_tan(void);
void sf_asin(void);
void sf_acos(void);
void sf_atan(void);
void sf_atan2(void);
void sf_sinh(void);
void sf_cosh(void);
void sf_tanh(void);
void sf_asinh(void);
void sf_acosh(void);
void sf_atanh(void);
void sf_exp(void);
void sf_frexp(void);
void sf_ldexp(void);
void sf_log(void);
void sf_log10(void);
void sf_modf(void);
void sf_pow(void);
void sf_sqrt(void);
void sf_ceil(void);
void sf_floor(void);
void sf_fmod(void);
void sf_fabs(void);

#ifndef __XC32__    /* it looks like XC32 does not support these */
void sf_trunc(void);
void sf_round(void);
void sf_lroundl(void);
void sf_fmin(void);
void sf_fmax(void);
#endif

#ifdef __cplusplus
}
#endif

#endif  /* LIB_MATH_H */
#endif
