#ifdef CIMPL

#include <stdlib.h>
#include "../../fatfs/source/ff.h"
#include "l_fatfs.h"
#include "l_stdio.h"	/* using ff_core() for f_printf() */

/* system function handlers take parameters from *prog and return result in token.data */

const sys_const_t fatfs_const_table[] = {
	{ { { .i = FF_MIN_SS },  (FT_CONST | FT_UNSIGNED | DT_INT), 0, {0} },   "FF_MIN_SS",     9 },
	{ { { .i = FF_MAX_SS },  (FT_CONST | FT_UNSIGNED | DT_INT), 0, {0} },   "FF_MAX_SS",     9 },
	{ { { .i = FF_SFN_BUF }, (FT_CONST | FT_UNSIGNED | DT_INT), 0, {0} },   "FF_SFN_BUF",    10 },
	{ { { .i = FF_LFN_BUF }, (FT_CONST | FT_UNSIGNED | DT_INT), 0, {0} },	"FF_LFN_BUF",    10 },
    { { {0}, 0, 0, {0} }, NULL, 0 }	/* must be final in this array */
};

const sys_func_t fatfs_func_table[] = {
	{ sf_f_open,      	"f_open",			6,  ":FRESULT f_open(FIL *, const TCHAR *, BYTE)", NULL },
	{ sf_f_close,     	"f_close",			7,  ":FRESULT f_close(FIL *)", NULL },
	{ sf_f_read,      	"f_read",			6,  ":FRESULT f_read(FIL *, void *, UINT, UINT *)", NULL },
	{ sf_f_write,     	"f_write",			7,  ":FRESULT f_write(FIL *, const void *, UINT, UINT *)", NULL },
	{ sf_f_lseek,     	"f_lseek",			7,  ":FRESULT f_lseek(FIL *, FSIZE_t)", NULL },
	{ sf_f_truncate,  	"f_truncate",		10, ":FRESULT f_truncate(FIL *)", NULL },
	{ sf_f_sync,      	"f_sync",			6,  ":FRESULT f_sync(FIL *)", NULL },
	{ sf_f_expand,    	"f_expand",			8,  ":FRESULT f_expand(FIL *, FSIZE_t, BYTE)", NULL },
	{ sf_f_gets,      	"f_gets",			6,  ":TCHAR *f_gets(TCHAR *, int, FIL *)", NULL },
	{ sf_f_putc,      	"f_putc",			6,  ":int f_putc(TCHAR, FIL *)", NULL },
	{ sf_f_puts,      	"f_puts",			6,  ":int f_puts(const TCHAR *, FIL *)", NULL },
	{ sf_f_printf,    	"f_printf",			8,  ":int f_printf(FIL *, const TCHAR *, ...)", NULL },
	{ sf_f_tell,      	"f_tell",			6,  ":FSIZE_t f_tell(FIL *)", NULL },
	{ sf_f_eof,       	"f_eof",			5,  ":int f_eof(FIL *)", NULL },
	{ sf_f_size,      	"f_size",			6,  ":FSIZE_t f_size(FIL *)", NULL },
	{ sf_f_error,     	"f_error",			7,  ":int f_error(FIL *)", NULL },
	{ sf_f_opendir,   	"f_opendir",		9,  ":FRESULT f_opendir(DIR *, const TCHAR *)", NULL },
	{ sf_f_closedir,  	"f_closedir",		10, ":FRESULT f_closedir(DIR *)", NULL },
	{ sf_f_readdir,   	"f_readdir",		9,  ":FRESULT f_readdir(DIR *, FILINFO *)", NULL },
	{ sf_f_findfirst, 	"f_findfirst",		11, ":FRESULT f_findfirst(DIR *, FILINFO *, const TCHAR *, const TCHAR *)", NULL },
	{ sf_f_findnext,  	"f_findnext",		10, ":FRESULT f_findnext(DIR *, FILINFO *)", NULL },
	{ sf_f_stat,      	"f_stat",			6,  ":FRESULT f_stat(const TCHAR *, FILINFO *)", NULL },
	{ sf_f_unlink,    	"f_unlink",			8,  ":FRESULT f_unlink(const TCHAR *)", NULL },
	{ sf_f_rename,    	"f_rename",			8,  ":FRESULT f_rename(const TCHAR *, const TCHAR *)", NULL },
	{ sf_f_chmod,     	"f_chmod",			7,  ":FRESULT f_chmod(const TCHAR *, BYTE, BYTE)", NULL },
	{ sf_f_utime,     	"f_utime",			7,  ":FRESULT f_utime(const TCHAR *, const FILINFO *)", NULL },
	{ sf_f_mkdir,     	"f_mkdir",			7,  ":FRESULT f_mkdir(const TCHAR *)", NULL },
	{ sf_f_chdir,     	"f_chdir",			7,  ":FRESULT f_chdir(const TCHAR *)", NULL },
	{ sf_f_chdrive,   	"f_chdrive",		9,  ":FRESULT f_chdrive(const TCHAR *)", NULL },
	{ sf_f_getcwd,    	"f_getcwd",			8,  ":FRESULT f_getcwd(TCHAR *, UINT)", NULL },
	{ sf_f_mount,     	"f_mount",			7,  ":FRESULT f_mount(FATFS *, const TCHAR *, BYTE)", NULL },
	{ sf_f_mkfs,      	"f_mkfs",			6,  ":FRESULT f_mkfs(const TCHAR *, BYTE, DWORD, void *, UINT)", NULL },
	{ sf_f_getfree,   	"f_getfree",		9,  ":FRESULT f_getfree(const TCHAR *, DWORD *, FATFS **)", NULL },
	{ sf_f_getlabel,  	"f_getlabel",		10, ":FRESULT f_getlabel(const TCHAR *, TCHAR *, DWORD *)", NULL },
	{ sf_f_setlabel,  	"f_setlabel",		10, ":FRESULT f_setlabel(const TCHAR *)", NULL },
    {NULL, NULL, 0, NULL, NULL}
};


void sf_f_open(void) {
	get_param(&d1, DT_VOID, 1);					/* FIL* fp */
	get_comma();
	get_param(&d2, (FT_CONST | DT_CHAR), 1);	/* const TCHAR* path */
	get_comma();
	get_param(&d3, (FT_UNSIGNED | DT_CHAR), 0);	/* BYTE mode */
    acc[accN].ind = 0;
    acc[accN].type = DT_ENUM;
    ival(accN) = (uint_t) f_open((FIL *) (uintptr_t) d1.val.i, 
										(const TCHAR *) (uintptr_t) d2.val.i, 
										(BYTE) d3.val.i);
}


void sf_f_close(void) {
	get_param(&d1, DT_VOID, 1);					/* FIL* fp */
    acc[accN].ind = 0;
    acc[accN].type = DT_ENUM;
    ival(accN) = (uint_t) f_close((FIL *) (uintptr_t) d1.val.i);
}


void sf_f_read(void) {
	get_param(&d1, DT_VOID, 1);					/* FIL* fp */
	get_comma();
	get_param(&d2, DT_VOID, 1);					/* void* buff */
	get_comma();
	get_param(&d3, (FT_UNSIGNED | DT_INT), 0);	/* UINT btr */
	get_comma();
	get_param(&d4, (FT_UNSIGNED | DT_INT), 1);	/* UINT* br */
    acc[accN].ind = 0;
    acc[accN].type = DT_ENUM;
    ival(accN) = (uint_t) f_read((FIL *) (uintptr_t) d1.val.i, 
									   	(void *) (uintptr_t) d2.val.i, 
									   	(UINT) d3.val.i,
									   	(UINT *) (uintptr_t) d4.val.i);
}


void sf_f_write(void) {
	get_param(&d1, DT_VOID, 1);					/* FIL* fp */
	get_comma();
	get_param(&d2, DT_VOID, 1);					/* void* buff */
	get_comma();
	get_param(&d3, (FT_UNSIGNED | DT_INT), 0);	/* UINT btw */
	get_comma();
	get_param(&d4, (FT_UNSIGNED | DT_INT), 1);	/* UINT* bw */
    acc[accN].ind = 0;
    acc[accN].type = DT_ENUM;
    ival(accN) = (uint_t) f_write((FIL *) (uintptr_t) d1.val.i, 
									   	(void *) (uintptr_t) d2.val.i, 
									   	(UINT) d3.val.i,
									   	(UINT *) (uintptr_t) d4.val.i);
}


void sf_f_lseek(void) {
	get_param(&d1, DT_VOID, 1);					/* FIL* fp */
	get_comma();
	get_param(&d2, (FT_UNSIGNED | DT_LONG), 0);	/* FSIZE_t ofs */
    acc[accN].ind = 0;
    acc[accN].type = DT_ENUM;
    ival(accN) = (uint_t) f_lseek((FIL *) (uintptr_t) d1.val.i,
										(FSIZE_t) d2.val.i);
}


void sf_f_truncate(void) {
	get_param(&d1, DT_VOID, 1);					/* FIL* fp */
    acc[accN].ind = 0;
    acc[accN].type = DT_ENUM;
    ival(accN) = (uint_t) f_truncate((FIL *) (uintptr_t) d1.val.i);
}


void sf_f_sync(void) {
	get_param(&d1, DT_VOID, 1);					/* FIL* fp */
    acc[accN].ind = 0;
    acc[accN].type = DT_ENUM;
    ival(accN) = (uint_t) f_sync((FIL *) (uintptr_t) d1.val.i);
}


void sf_f_expand(void) {
	get_param(&d1, DT_VOID, 1);					/* FIL* fp */
	get_comma();
	get_param(&d2, (FT_UNSIGNED | DT_LONG), 0);	/* FSIZE_t fsz */
	get_comma();
	get_param(&d3, (FT_UNSIGNED | DT_CHAR), 0);	/* BYTE opt */
    acc[accN].ind = 0;
    acc[accN].type = DT_ENUM;
    ival(accN) = (uint_t) f_expand((FIL *) (uintptr_t) d1.val.i, 
									   	(FSIZE_t) d2.val.i, 
									   	(BYTE) d3.val.i);
}


void sf_f_gets(void) {
	get_param(&d1, DT_CHAR, 1);					/* TCHAR* buff */
	get_comma();
	get_param(&d2, DT_INT, 0);					/* int len */
	get_comma();
	get_param(&d3, DT_VOID, 1);					/* FIL* fp */
    acc[accN].ind = 1;
    acc[accN].type = DT_CHAR;
    ival(accN) = (uintptr_t) f_gets((TCHAR *) (uintptr_t) d1.val.i, 
									   		(int_t) d2.val.i, 
									   		(FIL *) (uintptr_t) d3.val.i);
}


void sf_f_putc(void) {
	get_param(&d1, DT_CHAR, 0);					/* TCHAR c */
	get_comma();
	get_param(&d2, DT_VOID, 1);					/* FIL* fp */
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
    ival(accN) = (int_t) f_putc((TCHAR) d1.val.i, 
							   			(FIL *) (uintptr_t) d2.val.i);
}


void sf_f_puts(void) {
	get_param(&d1, DT_CHAR, 1);					/* TCHAR* str */
	get_comma();
	get_param(&d2, DT_VOID, 1);					/* FIL* fp */
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
    ival(accN) = (int_t) f_puts((TCHAR *) (uintptr_t) d1.val.i, 
							   			(FIL *) (uintptr_t) d2.val.i);
}


void sf_f_printf(void) {
    get_param(&d1, DT_VOID, 1);					/* FIL* fp */
    get_comma();
    ff_core((FIL *) (uintptr_t) d1.val.i, FPRINTF_TYPE);
}


void sf_f_tell(void) {
	get_param(&d1, DT_VOID, 1);					/* FIL* fp */
    acc[accN].ind = 0;
    acc[accN].type = (FT_UNSIGNED | DT_LONG);
    ival(accN) = (FSIZE_t) f_tell((FIL *) (uintptr_t) d1.val.i);
}


void sf_f_eof(void) {
	get_param(&d1, DT_VOID, 1);					/* FIL* fp */
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
    ival(accN) = (int_t) f_eof((FIL *) (uintptr_t) d1.val.i);
}


void sf_f_size(void) {
	get_param(&d1, DT_VOID, 1);					/* FIL* fp */
    acc[accN].ind = 0;
    acc[accN].type = (FT_UNSIGNED | DT_LONG);
    ival(accN) = (FSIZE_t) f_size((FIL *) (uintptr_t) d1.val.i);
}


void sf_f_error(void) {
	get_param(&d1, DT_VOID, 1);					/* FIL* fp */
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
    ival(accN) = (int_t) f_error((FIL *) (uintptr_t) d1.val.i);
}


void sf_f_opendir(void) {
	get_param(&d1, DT_VOID, 1);					/* DIR* dp */
	get_comma();
	get_param(&d2, (FT_CONST | DT_CHAR), 1);	/* const TCHAR* path */
    acc[accN].ind = 0;
    acc[accN].type = DT_ENUM;
    ival(accN) = (uint_t) f_opendir((DIR *) (uintptr_t) d1.val.i,
											(const TCHAR *) (uintptr_t) d2.val.i);
}


void sf_f_closedir(void) {
	get_param(&d1, DT_VOID, 1);					/* DIR* dp */
    acc[accN].ind = 0;
    acc[accN].type = DT_ENUM;
    ival(accN) = (uint_t) f_closedir((DIR *) (uintptr_t) d1.val.i);
}


void sf_f_readdir(void) {
	get_param(&d1, DT_VOID, 1);					/* DIR* dp */
	get_comma();
	get_param(&d2, DT_VOID, 1);					/* FILINFO* fno */
    acc[accN].ind = 0;
    acc[accN].type = DT_ENUM;
    ival(accN) = (uint_t) f_readdir((DIR *) (uintptr_t) d1.val.i,
											(FILINFO *) (uintptr_t) d2.val.i);
}


void sf_f_findfirst(void) {
	get_param(&d1, DT_VOID, 1);					/* DIR* dp */
	get_comma();
	get_param(&d2, DT_VOID, 1);					/* FILINFO* fno */
	get_comma();
	get_param(&d3, (FT_CONST | DT_CHAR), 1);	/* const TCHAR* path */
	get_comma();
	get_param(&d4, (FT_CONST | DT_CHAR), 1);	/* const TCHAR* pattern */
	acc[accN].ind = 0;
    acc[accN].type = DT_ENUM;
    ival(accN) = (uint_t) f_findfirst((DIR *) (uintptr_t) d1.val.i,
											(FILINFO *) (uintptr_t) d2.val.i,
											(const TCHAR *) (uintptr_t) d3.val.i,
											(const TCHAR *) (uintptr_t) d4.val.i);
}


void sf_f_findnext(void) {
	get_param(&d1, DT_VOID, 1);					/* DIR* dp */
	get_comma();
	get_param(&d2, DT_VOID, 1);					/* FILINFO* fno */
	acc[accN].ind = 0;
    acc[accN].type = DT_ENUM;
    ival(accN) = (uint_t) f_findnext((DIR *) (uintptr_t) d1.val.i,
											(FILINFO *) (uintptr_t) d2.val.i);
}


void sf_f_stat(void) {
	get_param(&d1, (FT_CONST | DT_CHAR), 1);	/* const TCHAR* path */
	get_comma();
	get_param(&d2, DT_VOID, 1);					/* FILINFO* fno */
    acc[accN].ind = 0;
    acc[accN].type = DT_ENUM;
    ival(accN) = (uint_t) f_stat((const TCHAR *) (uintptr_t) d1.val.i,
										(FILINFO *) (uintptr_t) d2.val.i);
}


void sf_f_unlink(void) {
	get_param(&d1, (FT_CONST | DT_CHAR), 1);	/* const TCHAR* path */
	acc[accN].ind = 0;
    acc[accN].type = DT_ENUM;
    ival(accN) = (uint_t) f_unlink((const TCHAR *) (uintptr_t) d1.val.i);
}


void sf_f_rename(void) {
	get_param(&d1, (FT_CONST | DT_CHAR), 1);	/* const TCHAR* path_old */
	get_comma();
	get_param(&d2, (FT_CONST | DT_CHAR), 1);	/* const TCHAR* path_new */
	acc[accN].ind = 0;
    acc[accN].type = DT_ENUM;
    ival(accN) = (uint_t) f_rename((const TCHAR *) (uintptr_t) d1.val.i,
											(const TCHAR *) (uintptr_t) d2.val.i);
}


void sf_f_chmod(void) {
	get_param(&d1, (FT_CONST | DT_CHAR), 1);	/* const TCHAR* path */
	get_comma();
	get_param(&d2, (FT_UNSIGNED | DT_CHAR), 0);	/* BYTE attr */
	get_comma();
	get_param(&d3, (FT_UNSIGNED | DT_CHAR), 0);	/* BYTE mask */
	acc[accN].ind = 0;
    acc[accN].type = DT_ENUM;
    ival(accN) = (uint_t) f_chmod((const TCHAR *) (uintptr_t) d1.val.i,
										(BYTE) d2.val.i,
										(BYTE) d3.val.i);
}


void sf_f_utime(void) {
	get_param(&d1, (FT_CONST | DT_CHAR), 1);	/* const TCHAR* path */
	get_comma();
	get_param(&d2, DT_VOID, 1);					/* FILINFO* fno */
	acc[accN].ind = 0;
    acc[accN].type = DT_ENUM;
    ival(accN) = (uint_t) f_utime((const TCHAR *) (uintptr_t) d1.val.i,
										(FILINFO *) (uintptr_t) d2.val.i);
}


void sf_f_mkdir(void) {
	get_param(&d1, (FT_CONST | DT_CHAR), 1);	/* const TCHAR* path */
	acc[accN].ind = 0;
    acc[accN].type = DT_ENUM;
    ival(accN) = (uint_t) f_mkdir((const TCHAR *) (uintptr_t) d1.val.i);
}


void sf_f_chdir(void) {
	get_param(&d1, (FT_CONST | DT_CHAR), 1);	/* const TCHAR* path */
	acc[accN].ind = 0;
    acc[accN].type = DT_ENUM;
    ival(accN) = (uint_t) f_chdir((const TCHAR *) (uintptr_t) d1.val.i);
}


void sf_f_chdrive(void) {
	get_param(&d1, (FT_CONST | DT_CHAR), 1);	/* const TCHAR* path */
	acc[accN].ind = 0;
    acc[accN].type = DT_ENUM;
    ival(accN) = (uint_t) f_chdrive((const TCHAR *) (uintptr_t) d1.val.i);
}


void sf_f_getcwd(void) {
	get_param(&d1, DT_CHAR, 1);					/* TCHAR* buff */
	get_comma();
	get_param(&d2, (FT_UNSIGNED | DT_INT), 0);	/* UINT len */
	acc[accN].ind = 0;
    acc[accN].type = DT_ENUM;
    ival(accN) = (uint_t) f_getcwd((TCHAR *) (uintptr_t) d1.val.i,
											(UINT) d2.val.i);
}


void sf_f_mount(void) {
	get_param(&d1, DT_VOID, 1);					/* FATFS* fs */
	get_comma();
	get_param(&d2, (FT_CONST | DT_CHAR), 1);	/* const TCHAR* path */
	get_comma();
	get_param(&d3, (FT_UNSIGNED | DT_CHAR), 0);	/* BYTE opt */
	acc[accN].ind = 0;
    acc[accN].type = DT_ENUM;
    ival(accN) = (uint_t) f_mount((FATFS *) (uintptr_t) d1.val.i,
										(const TCHAR *) (uintptr_t) d2.val.i,
										(BYTE) d3.val.i);
}


void sf_f_mkfs(void) {
	get_param(&d1, (FT_CONST | DT_CHAR), 1);	/* const TCHAR* path */
	get_comma();
	get_param(&d2, DT_VOID, 1);					/* const MKFS_PARM* opt */
	get_comma();
	get_param(&d3, DT_VOID, 1);					/* void* work */
	get_comma();
	get_param(&d4, (FT_UNSIGNED | DT_INT), 0);	/* UINT len */
	acc[accN].ind = 0;
    acc[accN].type = DT_ENUM;
    ival(accN) = (uint_t) f_mkfs((const TCHAR *) (uintptr_t) d1.val.i,
										(const MKFS_PARM *) (uintptr_t) d2.val.i,
										(void *) (uintptr_t) d3.val.i,
										(UINT) d4.val.i);
}


void sf_f_getfree(void) {
	get_param(&d1, (FT_CONST | DT_CHAR), 1);	/* const TCHAR* path */
	get_comma();
	get_param(&d2, (FT_UNSIGNED | DT_LONG), 1);	/* DWORD* nclst */
	get_comma();
	get_param(&d3, DT_VOID, 2);					/* FATFS** fatfs */
	acc[accN].ind = 0;
    acc[accN].type = DT_ENUM;
    ival(accN) = (uint_t) f_getfree((const TCHAR *) (uintptr_t) d1.val.i,
											(DWORD *) (uintptr_t) d2.val.i,
											(FATFS **) (uintptr_t) d3.val.i);
}


void sf_f_getlabel(void) {
	get_param(&d1, (FT_CONST | DT_CHAR), 1);	/* const TCHAR* path */
	get_comma();
	get_param(&d2, DT_CHAR, 1);					/* TCHAR* label */
	get_comma();
	get_param(&d3, (FT_UNSIGNED | DT_LONG), 1);	/* DWORD* vsn */
	acc[accN].ind = 0;
    acc[accN].type = DT_ENUM;
    ival(accN) = (uint_t) f_getlabel((const TCHAR *) (uintptr_t) d1.val.i,
											(TCHAR *) (uintptr_t) d2.val.i,
											(DWORD *) (uintptr_t) d3.val.i);
}


void sf_f_setlabel(void) {
	get_param(&d1, (FT_CONST | DT_CHAR), 1);	/* const TCHAR* label */
	acc[accN].ind = 0;
    acc[accN].type = DT_ENUM;
    ival(accN) = (uint_t) f_setlabel((const TCHAR *) (uintptr_t) d1.val.i);
}

#endif
