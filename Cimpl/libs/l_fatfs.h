#ifdef CIMPL
#ifndef LIB_FATFS_H
#define LIB_FATFS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../cimpl.h"

extern const sys_const_t fatfs_const_table[];
extern const sys_func_t fatfs_func_table[];

#if DISABLE_LONG_LONG == 0
	#define fatfs_qw \
		"typedef uint64_t      QWORD;\r\n" \
		"typedef QWORD         FSIZE_t;\r\n" \
		"typedef QWORD         LBA_t;\r\n"
#else
	#define fatfs_qw \
		"typedef uint32_t      QWORD;\r\n" \
		"typedef DWORD         FSIZE_t;\r\n" \
		"typedef DWORD         LBA_t;\r\n"
#endif

#define fatfs_src \
	"#include <stdint.h>\r\n" \
	\
	"typedef unsigned int  UINT;\r\n" \
	"typedef unsigned char BYTE;\r\n" \
	"typedef uint16_t      WORD;\r\n" \
	"typedef uint32_t      DWORD;\r\n" \
	"typedef WORD          WCHAR;\r\n" \
	"typedef char          TCHAR;\r\n" \
	fatfs_qw \
	"\r\n" \
	"typedef struct FATFS {\r\n" \
	"  BYTE   fs_type;\r\n" \
	"  BYTE   pdrv;\r\n" \
	"  BYTE   n_fats;\r\n" \
	"  BYTE   wflag;\r\n" \
	"  BYTE   fsi_flag;\r\n" \
	"  WORD   id;\r\n" \
	"  WORD   n_rootdir;\r\n" \
	"  WORD   csize;\r\n" \
	"  WORD   ssize;\r\n" \
	"  WCHAR *lfnbuf;\r\n" \
	"  DWORD  last_clst;\r\n" \
	"  DWORD  free_clst;\r\n" \
	"  DWORD  cdir;\r\n" \
	"  DWORD  n_fatent;\r\n" \
	"  DWORD  fsize;\r\n" \
	"  LBA_t  volbase;\r\n" \
	"  LBA_t  fatbase;\r\n" \
	"  LBA_t  dirbase;\r\n" \
	"  LBA_t  database;\r\n" \
	"  LBA_t  winsect;\r\n" \
	"  BYTE   win[FF_MAX_SS];\r\n" \
	"};\r\n" \
	"\r\n" \
	"typedef struct FFOBJID {\r\n" \
	"  FATFS *fs;\r\n" \
	"  WORD   id;\r\n" \
	"  BYTE   attr;\r\n" \
	"  BYTE   stat;\r\n" \
	"  DWORD  sclust;\r\n" \
	"  FSIZE_t objsize;\r\n" \
	"};\r\n" \
	"\r\n" \
	"typedef struct FIL {\r\n" \
	"  FFOBJID obj;\r\n" \
	"  BYTE    flag;\r\n" \
	"  BYTE    err;\r\n" \
	"  FSIZE_t fptr;\r\n" \
	"  DWORD   clust;\r\n" \
	"  LBA_t   sect;\r\n" \
	"  LBA_t   dir_sect;\r\n" \
	"  BYTE   *dir_ptr;\r\n" \
	"  DWORD  *cltbl;\r\n" \
	"};\r\n" \
	"\r\n" \
	"typedef struct DIR {\r\n" \
	"  FFOBJID obj;\r\n" \
	"  DWORD  dptr;\r\n" \
	"  DWORD  clust;\r\n" \
	"  LBA_t  sect;\r\n" \
	"  BYTE  *dir;\r\n" \
	"  BYTE   fn[12];\r\n" \
	"  DWORD  blk_ofs;\r\n" \
	"  const TCHAR *pat;\r\n" \
	"};\r\n" \
	"\r\n" \
	"typedef struct FILINFO {\r\n" \
	"  FSIZE_t fsize;\r\n" \
	"  WORD    fdate;\r\n" \
	"  WORD    ftime;\r\n" \
	"  BYTE    fattrib;\r\n" \
	"  TCHAR   altname[FF_SFN_BUF + 1];\r\n" \
	"  TCHAR   fname[FF_LFN_BUF + 1];\r\n" \
	"};\r\n" \
	"\r\n" \
	"typedef struct MKFS_PARM {\r\n" \
	"  BYTE  fmt;\r\n" \
	"  BYTE  n_fat;\r\n" \
	"  UINT  align;\r\n" \
	"  UINT  n_root;\r\n" \
	"  DWORD au_size;\r\n" \
	"};\r\n" \
	"\r\n" \
	"typedef enum FRESULT {\r\n" \
	"  FR_OK = 0,\r\n" \
	"  FR_DISK_ERR,\r\n" \
	"  FR_INT_ERR,\r\n" \
	"  FR_NOT_READY,\r\n" \
	"  FR_NO_FILE,\r\n" \
	"  FR_NO_PATH,\r\n" \
	"  FR_INVALID_NAME,\r\n" \
	"  FR_DENIED,\r\n" \
	"  FR_EXIST,\r\n" \
	"  FR_INVALID_OBJECT,\r\n" \
	"  FR_WRITE_PROTECTED,\r\n" \
	"  FR_INVALID_DRIVE,\r\n" \
	"  FR_NOT_ENABLED,\r\n" \
	"  FR_NO_FILESYSTEM,\r\n" \
	"  FR_MKFS_ABORTED,\r\n" \
	"  FR_TIMEOUT,\r\n" \
	"  FR_LOCKED,\r\n" \
	"  FR_NOT_ENOUGH_CORE,\r\n" \
	"  FR_TOO_MANY_OPEN_FILES,\r\n" \
	"  FR_INVALID_PARAMETER\r\n" \
	"};\r\n" \
	ETXSTR

void sf_f_open(void);
void sf_f_close(void);
void sf_f_read(void);
void sf_f_write(void);
void sf_f_lseek(void);
void sf_f_truncate(void);
void sf_f_sync(void);
void sf_f_expand(void);
void sf_f_gets(void);
void sf_f_putc(void);
void sf_f_puts(void);
void sf_f_printf(void);
void sf_f_tell(void);
void sf_f_eof(void);
void sf_f_size(void);
void sf_f_error(void);
void sf_f_opendir(void);
void sf_f_closedir(void);
void sf_f_readdir(void);
void sf_f_findfirst(void);
void sf_f_findnext(void);
void sf_f_stat(void);
void sf_f_unlink(void);
void sf_f_rename(void);
void sf_f_chmod(void);
void sf_f_utime(void);
void sf_f_mkdir(void);
void sf_f_chdir(void);
void sf_f_chdrive(void);
void sf_f_getcwd(void);
void sf_f_mount(void);
void sf_f_mkfs(void);
void sf_f_getfree(void);
void sf_f_getlabel(void);
void sf_f_setlabel(void);

#ifdef __cplusplus
}
#endif

#endif  /* LIB_FATFS_H */
#endif
