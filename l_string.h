#ifdef CIMPL
#ifndef LIB_STRING_H
#define LIB_STRING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../cimpl.h"

extern const sys_const_t string_const_table[];
extern const sys_func_t string_func_table[];

void sf_memset(void);
void sf_memcpy(void);
void sf_memmove(void);
void sf_memcmp(void);
void sf_memchr(void);
void sf_strlen(void);
void sf_strcpy(void);
void sf_strncpy(void);
void sf_strcat(void);
void sf_strncat(void);
void sf_strcmp(void);
void sf_strncmp(void);
void sf_strchr(void);
void sf_strrchr(void);
void sf_strspn(void);
void sf_strcspn(void);
void sf_strpbrk(void);
void sf_strstr(void);
void sf_strtok(void);

#ifdef __cplusplus
}
#endif

#endif  /* LIB_STRING_H */
#endif
