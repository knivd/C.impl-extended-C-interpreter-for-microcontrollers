/*

Platform-depended library for the C.impl interpreter

platform: ELLO 1A
(C) KnivD, 2020-2021

*/

#ifdef CIMPL
#ifndef LIB_PLATFORM_H
#define LIB_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "platform.h"
#include "../../cimpl/cimpl.h"

extern const sys_const_t platform_const_table[];
extern const sys_func_t platform_func_table[];

#define platform_src \
    "const char const *platform = \"PC\";\r\n" \
    ETXSTR

void pltfm_init(void);
void pltfm_call(void);

void sf_delay_ms(void);     /* void delay_ms(unsigned long milliseconds) */
void sf_set_timer(void);    /* void set_timer(unsigned long milliseconds, void (*intHandler)(void) */

#ifdef __cplusplus
}
#endif

#endif  /* LIB_PLATFORM_H */
#endif
