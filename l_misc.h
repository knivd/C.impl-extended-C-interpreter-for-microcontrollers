#ifdef CIMPL
#ifndef LIB_MISC_H
#define LIB_MISC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../cimpl.h"

/* stdbool.h */

extern const sys_const_t stdbool_const_table[];

/* limits.h */

extern const sys_const_t limits_const_table[];

/* stdint.h */

extern const sys_const_t stdint_const_table[];

#define stdint_src \
    "typedef signed long long int intmax_t;\r\n" \
    "typedef unsigned long long int uintmax_t;\r\n" \
    "typedef signed char int8_t;\r\n" \
    "typedef unsigned char uint8_t;\r\n" \
    "typedef signed short int int16_t;\r\n" \
    "typedef unsigned short int uint16_t;\r\n" \
    "typedef signed long int int32_t;\r\n" \
    "typedef unsigned long int uint32_t;\r\n" \
    "typedef signed long long int int64_t;\r\n" \
    "typedef unsigned long long int uint64_t;\r\n" \
    "typedef signed int intptr_t;\r\n" \
    "typedef unsigned int uintptr_t;\r\n" \
	ETXSTR

/* stdarg.h */

extern const sys_func_t stdarg_func_table[];

void sf_va_start(void);
void sf_va_end(void);
void sf_va_arg(void);

/* ctype.h */

extern const sys_func_t ctype_func_table[];

void sf_tolower(void);
void sf_toupper(void);
void sf_isalnum(void);
void sf_isalpha(void);
void sf_isblank(void);
void sf_iscntrl(void);
void sf_isdigit(void);
void sf_isgraph(void);
void sf_islower(void);
void sf_isprint(void);
void sf_ispunct(void);
void sf_isspace(void);
void sf_isupper(void);
void sf_isxdigit(void);

/* assert.h */

extern const sys_func_t assert_func_table[];

void sf_assert(void);
void sf_error(void);

/* time.h */

extern const sys_const_t time_const_table[];
extern const sys_func_t time_func_table[];

#define time_src \
    "typedef unsigned long int time_t;\r\n" \
    "typedef struct tm {\r\n" \
    "  int tm_sec;\r\n" \
    "  int tm_min;\r\n" \
    "  int tm_hour;\r\n" \
    "  int tm_mday;\r\n" \
    "  int tm_mon;\r\n" \
    "  int tm_year;\r\n" \
    "  int tm_wday;\r\n" \
    "  int tm_yday;\r\n" \
    "  int tm_isdst;\r\n" \
    "};\r\n" \
	ETXSTR

void sf_clock(void);
void sf_time(void);
void sf_difftime(void);
void sf_mktime(void);
void sf_asctime(void);
void sf_ctime(void);

#ifdef __cplusplus
}
#endif

#endif  /* LIB_MISC_H */
#endif
