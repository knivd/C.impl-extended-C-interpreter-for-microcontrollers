#ifdef CIMPL

#include <string.h>
#include "l_string.h"

/* system function handlers take parameters from *prog and return result in acc[accN] */

const sys_const_t string_const_table[] = {
    { { { .i = (uintptr_t) NULL },  (FT_CONST | FT_CONSTPTR | DT_VOID), 1, {0} },    "NULL", 4 },
    { { {0}, 0, 0, {0} }, NULL, 0 }	/* must be final in this array */
};

const sys_func_t string_func_table[] = {
    { sf_memcpy,   "memcpy",   6,  "v*,v*,Cv*,z", NULL },
    { sf_memset,   "memset",   6,  "v*,v*,i,z", NULL },
    { sf_memmove,  "memmove",  7,  "v*,v*,Cv*,z", NULL },
    { sf_memcmp,   "memcmp",   6,  "i,Cv*,Cv*,z", NULL },
    { sf_memchr,   "memchr",   6,  "Cv*,Cv*,i,z", NULL },
    { sf_strlen,   "strlen",   6,  "z,Cc*", NULL },
    { sf_strcpy,   "strcpy",   6,  "c*,c*,Cc*", NULL },
    { sf_strncpy,  "strncpy",  7,  "c*,c*,Cc*,z", NULL },
    { sf_strcat,   "strcat",   6,  "c*,c*,Cc*", NULL },
    { sf_strncat,  "strncat",  7,  "c*,c*,Cc*,z", NULL },
    { sf_strcmp,   "strcmp",   6,  "i,Cc*,Cc*", NULL },
    { sf_strncmp,  "strncmp",  7,  "i,Cc*,Cc*,z", NULL },
    { sf_strchr,   "strchr",   6,  "Cc*,Cc*,i", NULL },
    { sf_strrchr,  "strrchr",  7,  "Cc*,Cc*,i", NULL },
    { sf_strspn,   "strspn",   6,  "z,Cc*,Cc*", NULL },
    { sf_strcspn,  "strcspn",  7,  "z,Cc*,Cc*", NULL },
    { sf_strpbrk,  "strpbrk",  7,  "Cc*,Cc*,Cc*", NULL },
    { sf_strstr,   "strstr",   6,  "Cc*,Cc*,Cc*", NULL },
    { sf_strtok,   "strtok",   6,  "c*,c*,Cc*", NULL },
    {NULL, NULL, 0, NULL, NULL}
};


void sf_memset(void) {
    get_param(&d1, DT_VOID, 1);
    get_comma();
    get_param(&d2, DT_INT, 0);
    get_comma();
    get_param(&d3, DT_SIZE_T, 0);
    acc[accN].ind = 1;
    acc[accN].type = DT_VOID;
    ival(accN) = (uintptr_t) memset((void *) (uintptr_t) d1.val.i, d2.val.i, d3.val.i);
}


void sf_memcpy(void) {
    get_param(&d1, DT_VOID, 1);
    get_comma();
    get_param(&d2, (FT_CONST | DT_VOID), 1);
    get_comma();
    get_param(&d3, DT_SIZE_T, 0);
    acc[accN].ind = 1;
    acc[accN].type = DT_VOID;
    ival(accN) = (uintptr_t) memcpy((void *) (uintptr_t) d1.val.i, (const void *) (uintptr_t) d2.val.i, d3.val.i);
}


void sf_memmove(void) {
    get_param(&d1, DT_VOID, 1);
    get_comma();
    get_param(&d2, (FT_CONST | DT_VOID), 1);
    get_comma();
    get_param(&d3, DT_SIZE_T, 0);
    acc[accN].ind = 1;
    acc[accN].type = DT_VOID;
    ival(accN) = (uintptr_t) memmove((void *) (uintptr_t) d1.val.i, (const void *) (uintptr_t) d2.val.i, d3.val.i);
}


void sf_memcmp(void) {
    get_param(&d1, (FT_CONST | DT_VOID), 1);
    get_comma();
    get_param(&d2, (FT_CONST | DT_VOID), 1);
    get_comma();
    get_param(&d3, DT_SIZE_T, 0);
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
    ival(accN) = memcmp((const void *) (uintptr_t) d1.val.i, (const void *) (uintptr_t) d2.val.i, d3.val.i);
}


void sf_memchr(void) {
    get_param(&d1, DT_VOID, 1);
    get_comma();
    get_param(&d2, DT_INT, 0);
    get_comma();
    get_param(&d3, DT_SIZE_T, 0);
    acc[accN].ind = 1;
    acc[accN].type = DT_VOID;
    ival(accN) = (uintptr_t) memchr((void *) (uintptr_t) d1.val.i, d2.val.i, d3.val.i);
}


void sf_strlen(void) {
    get_param(&d1, (FT_CONST | DT_CHAR), 1);
    acc[accN].ind = 0;
    acc[accN].type = DT_SIZE_T;
    ival(accN) = strlen((const char *) (uintptr_t) d1.val.i);
}


void sf_strcpy(void) {
    get_param(&d1, DT_CHAR, 1);
    get_comma();
    get_param(&d2, (FT_CONST | DT_CHAR), 1);
    acc[accN].ind = 1;
    acc[accN].type = DT_CHAR;
    ival(accN) = (uintptr_t) strcpy((char *) (uintptr_t) d1.val.i, (const char *) (uintptr_t) d2.val.i);
}


void sf_strncpy(void) {
    get_param(&d1, DT_CHAR, 1);
    get_comma();
    get_param(&d2, (FT_CONST | DT_CHAR), 1);
    get_comma();
    get_param(&d3, DT_SIZE_T, 0);
    acc[accN].ind = 1;
    acc[accN].type = DT_CHAR;
    ival(accN) = (uintptr_t) strncpy((char *) (uintptr_t) d1.val.i, (const char *) (uintptr_t) d2.val.i, d3.val.i);
}


void sf_strcat(void) {
    get_param(&d1, DT_CHAR, 1);
    get_comma();
    get_param(&d2, (FT_CONST | DT_CHAR), 1);
    acc[accN].ind = 1;
    acc[accN].type = DT_CHAR;
    ival(accN) = (uintptr_t) strcat((char *) (uintptr_t) d1.val.i, (const char *) (uintptr_t) d2.val.i);
}


void sf_strncat(void) {
    get_param(&d1, DT_CHAR, 1);
    get_comma();
    get_param(&d2, (FT_CONST | DT_CHAR), 1);
    get_comma();
    get_param(&d3, DT_SIZE_T, 0);
    acc[accN].ind = 1;
    acc[accN].type = DT_CHAR;
    ival(accN) = (uintptr_t) strncat((char *) (uintptr_t) d1.val.i, (const char *) (uintptr_t) d2.val.i, d3.val.i);
}


void sf_strcmp(void) {
    get_param(&d1, DT_CHAR, 1);
    get_comma();
    get_param(&d2, (FT_CONST | DT_CHAR), 1);
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
    ival(accN) = strcmp((char *) (uintptr_t) d1.val.i, (const char *) (uintptr_t) d2.val.i);
}


void sf_strncmp(void) {
    get_param(&d1, DT_CHAR, 1);
    get_comma();
    get_param(&d2, (FT_CONST | DT_CHAR), 1);
    get_comma();
    get_param(&d3, DT_SIZE_T, 0);
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
    ival(accN) = strncmp((char *) (uintptr_t) d1.val.i, (const char *) (uintptr_t) d2.val.i, d3.val.i);
}


void sf_strchr(void) {
    get_param(&d1, DT_CHAR, 1);
    get_comma();
    get_param(&d2, DT_INT, 0);
    acc[accN].ind = 1;
    acc[accN].type = DT_CHAR;
    ival(accN) = (uintptr_t) strchr((char *) (uintptr_t) d1.val.i, d2.val.i);
}


void sf_strrchr(void) {
    get_param(&d1, DT_CHAR, 1);
    get_comma();
    get_param(&d2, DT_INT, 0);
    acc[accN].ind = 1;
    acc[accN].type = DT_CHAR;
    ival(accN) = (uintptr_t) strrchr((char *) (uintptr_t) d1.val.i, d2.val.i);
}


void sf_strspn(void) {
    get_param(&d1, (FT_CONST | DT_CHAR), 1);
    get_comma();
    get_param(&d2, (FT_CONST | DT_CHAR), 1);
    acc[accN].ind = 0;
    acc[accN].type = DT_SIZE_T;
    ival(accN) = strspn((const char *) (uintptr_t) d1.val.i, (const char *) (uintptr_t) d2.val.i);
}


void sf_strcspn(void) {
    get_param(&d1, (FT_CONST | DT_CHAR), 1);
    get_comma();
    get_param(&d2, (FT_CONST | DT_CHAR), 1);
    acc[accN].ind = 0;
    acc[accN].type = DT_SIZE_T;
    ival(accN) = strcspn((const char *) (uintptr_t) d1.val.i, (const char *) (uintptr_t) d2.val.i);
}


void sf_strpbrk(void) {
    get_param(&d1, (FT_CONST | DT_CHAR), 1);
    get_comma();
    get_param(&d2, (FT_CONST | DT_CHAR), 1);
    acc[accN].ind = 1;
    acc[accN].type = DT_CHAR;
    ival(accN) = (uintptr_t) strpbrk((char *) (uintptr_t) d1.val.i, (const char *) (uintptr_t) d2.val.i);
}


void sf_strstr(void) {
    get_param(&d1, DT_CHAR, 1);
    get_comma();
    get_param(&d2, (FT_CONST | DT_CHAR), 1);
    acc[accN].ind = 1;
    acc[accN].type = DT_CHAR;
    ival(accN) = (uintptr_t) strstr((char *) (uintptr_t) d1.val.i, (const char *) (uintptr_t) d2.val.i);
}


void sf_strtok(void) {
    get_param(&d1, DT_CHAR, 1);
    get_comma();
    get_param(&d2, (FT_CONST | DT_CHAR), 1);
    acc[accN].ind = 1;
    acc[accN].type = DT_CHAR;
    ival(accN) = (uintptr_t) strtok((char *) (uintptr_t) d1.val.i, (const char *) (uintptr_t) d2.val.i);
}

#endif
