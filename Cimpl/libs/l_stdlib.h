#ifdef CIMPL
#ifndef LIB_STDLIB_H
#define LIB_STDLIB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../cimpl.h"

extern const sys_const_t stdlib_const_table[];
extern const sys_func_t stdlib_func_table[];

void sf_BIT(void);      /* unsigned long BIT(unsigned char n) */
void sf_exit(void);
void sf_rand(void);
void sf_srand(void);
void sf_atof(void);
void sf_atoi(void);
void sf_atol(void);
void sf_atoll(void);
void sf_strtod(void);
void sf_strtol(void);
void sf_strtoul(void);
void sf_malloc(void);
void sf_calloc(void);
void sf_realloc(void);
void sf_free(void);
void sf_abs(void);
void sf_labs(void);

#if DISABLE_LONG_LONG <= 0
void sf_llabs(void);
void sf_strtoll(void);
void sf_strtoull(void);
#endif

#if DISABLE_LONG_DOUBLE <= 0
void sf_strtold(void);
#endif

#ifdef __cplusplus
}
#endif

#endif  /* LIB_STDLIB_H */
#endif
