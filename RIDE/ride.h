#ifndef RIDE_H
#define	RIDE_H

/*
 * --------------------------------------------------------------------------------------
 *
 * Rationally Integrated Development Environment
 * written by Konstantin Dimitrov, version 3, 2020
 *
 * --------------------------------------------------------------------------------------
 *  LICENSE INFORMATION
 *   1. This code is "linkware". It is free for use, modification, and distribusion
 *      without any limitations. The only requirement is to link with the author on
 *      LinkedIn: https://www.linkedin.com/in/knivd/ or follow on Twitter: @knivd
 *   2. The author assumes no responsibility for any loss or damage caused by this code
 * --------------------------------------------------------------------------------------
*/

/* ======================================================================================

Written and debugged with Dev-C++ v5.11
NOTE: for best source readability, please use a monospace font (eg. Consolas)

RIDE is a simple text editor and shell, originally written for wrapping around
an external interpreter, but otherwise completely independent from it

*/

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "../xmem.h"
#include "graphics.h"
#include "kbdcodes.h"

void RIDE(void);
void RIDE_release_memory(void);
int getchx(void);
int edit_line(char *line, unsigned int line_number, unsigned int x);
int wait_for_brcont(void);
void execute_cmd_line(char *buf);
void execute_cmd_fos(char *pcmd);
unsigned long run_file(char *fn);
void store_settings(void);
uint32_t hash(const char *data, int len);

/* defines the width of the output in characters */
#define PAGE_WIDTH      80          /* default value for initialisation */

/* defines the height of a "screen page" in rows; use a large number to make it "unlimited" */
#define PAGE_HEIGHT     29          /* default value for initialisation */

/* define the number of spaces that represent a single tabulation character */
#define TAB_WIDTH       4           /* default value for initialisation */

/* defines the maximum length in characters of a single line */
#define LINE_WIDTH      255

/* number of saved strings in the entry log; maximum 255 */
#define LOG_ENTRIES     5

/* any value different from 0 will disable exiting from RIDE */
#define NO_EXIT_RIDE    1

/* file name for automated execution */
#define AUTORUN_FILE    "ifs:/autorun"

/* alternative file name for automated execution */
#define AUTORUN_FILE_SD "sd1:/autorun"

/* file name with the hashed access password */
#define PWD_FILE		"ifs:/$.key"

char buffer[LINE_WIDTH + 1];    /* line buffer */

typedef struct {
	uint32_t checksum;
    uint32_t stored_password;   /* stored 32-bit password hash */
    uint8_t kbd_layout;         /* keyboard layout: 1:US; 0:UK; 1:DE; 2:FR; 3:IT; 4:ES; 5:BE */
    int8_t brk_code;            /* break code */
    uint16_t page_width;        /* page width in number of columns */
    uint32_t page_height;       /* page height in number of rows */
    uint8_t tab_width;          /* number of spaces in a tab */
} ride_settings_t;
ride_settings_t settings;       /* stored settings */
unsigned char passw_mode;       /* flag for password mode in edit_line(); not displaying characters */

#ifndef MSK_WAIT_MS
#define MSK_WAIT_MS     15      /* give this many milliseconds time for the next key in escape sequences */
#endif

/* basic keyboard codes */
#define KEY_BREAK   '\003'  /* Ctrl-C used for terminating execution */
#define KEY_ENT     '\r'
#define KEY_TAB     '\t'
#define KEY_BSP     '\b'
#define KEY_NLINE   '\n'
#define KEY_BEEP    '\a'
#define KEY_ESC     0x1B  	/* cancel and return with the original line */

#define CTRL_AT     0x00	/* Ctrl-@ */
#define CTRL_A      0x01
#define CTRL_B      0x02
#define CTRL_C      0x03
#define CTRL_D      0x04
#define CTRL_E      0x05
#define CTRL_F      0x06
#define CTRL_G      0x07
#define CTRL_H      0x08
#define CTRL_I      0x09
#define CTRL_J      0x0A
#define CTRL_K      0x0B
#define CTRL_L      0x0C	/* Ctrl-L clear the entire line */
#define CTRL_M      0x0D
#define CTRL_N      0x0E
#define CTRL_O      0x0F
#define CTRL_P      0x10
#define CTRL_Q      0x11
#define CTRL_R      0x12
#define CTRL_S      0x13
#define CTRL_T      0x14
#define CTRL_U      0x15
#define CTRL_V      0x16
#define CTRL_W      0x17
#define CTRL_X      0x18
#define CTRL_Y      0x19	/* Ctrl-Y clear to the end of the line */
#define CTRL_Z      0x1A	/* Ctrl-Z access the log entries */
#define CTRL_OPBRKT 0x1B	/* Ctrl-[ */
#define CTRL_BSLASH 0x1C	/* Ctrl-\ */
#define CTRL_CLBRKT 0x1D	/* Ctrl-] */
#define CTRL_CARET  0x1E	/* Ctrl-^ */
#define CTRL_UNDSCR 0x1F	/* Ctrl-_ */

#define MULPS2      0x0Ful

/* PS/2 key codes */
#define KEY_LEFT    (int) ((MULPS2 << 8) + 0x6B)
#define KEY_RIGHT   (int) ((MULPS2 << 8) + 0x74)
#define KEY_UP      (int) ((MULPS2 << 8) + 0x75)
#define KEY_DOWN    (int) ((MULPS2 << 8) + 0x72)
#define KEY_HOME    (int) ((MULPS2 << 8) + 0x6C)
#define KEY_END     (int) ((MULPS2 << 8) + 0x69)
#define KEY_DEL     (int) ((MULPS2 << 8) + 0x71)
#define KEY_INS     (int) ((MULPS2 << 8) + 0x70)
#define KEY_PGUP    (int) ((MULPS2 << 8) + 0x7D)
#define KEY_PGDN    (int) ((MULPS2 << 8) + 0x7A)
#define KEY_SCRLOCK (int) ((MULPS2 << 8) + 0x7E)
#define KEY_ALT     (int) ((MULPS2 << 8) + 0x8B)
#define KEY_F1      (int) ((MULPS2 << 8) + 0x91)
#define KEY_F2      (int) ((MULPS2 << 8) + 0x92)
#define KEY_F3      (int) ((MULPS2 << 8) + 0x93)
#define KEY_F4      (int) ((MULPS2 << 8) + 0x94)
#define KEY_F5      (int) ((MULPS2 << 8) + 0x95)
#define KEY_F6      (int) ((MULPS2 << 8) + 0x96)
#define KEY_F7      (int) ((MULPS2 << 8) + 0x97)
#define KEY_F8      (int) ((MULPS2 << 8) + 0x98)
#define KEY_F9      (int) ((MULPS2 << 8) + 0x99)
#define KEY_F10     (int) ((MULPS2 << 8) + 0x9A)
#define KEY_F11     (int) ((MULPS2 << 8) + 0x9B)
#define KEY_F12     (int) ((MULPS2 << 8) + 0x9C)

/* VT100-compatible key codes */
#define VTM_BACKSPC 0x7F
#define VTM_LEFT    (int) (((uint32_t) KEY_ESC << 16) + ((uint16_t) '[' << 8) + (uint8_t) 'D')
#define VTM_RIGHT   (int) (((uint32_t) KEY_ESC << 16) + ((uint16_t) '[' << 8) + (uint8_t) 'C')
#define VTM_UP      (int) (((uint32_t) KEY_ESC << 16) + ((uint16_t) '[' << 8) + (uint8_t) 'A')
#define VTM_DOWN    (int) (((uint32_t) KEY_ESC << 16) + ((uint16_t) '[' << 8) + (uint8_t) 'B')
#define VTM_HOME    (int) (((uint32_t) KEY_ESC << 24) + ((uint32_t) '[' << 16) + ((uint16_t) '1' << 8) + (uint8_t) '~')
#define VTM_END     (int) (((uint32_t) KEY_ESC << 24) + ((uint32_t) '[' << 16) + ((uint16_t) '4' << 8) + (uint8_t) '~')
#define VTM_DEL     (int) (((uint32_t) KEY_ESC << 24) + ((uint32_t) '[' << 16) + ((uint16_t) '3' << 8) + (uint8_t) '~')
#define VTM_INS     (int) (((uint32_t) KEY_ESC << 24) + ((uint32_t) '[' << 16) + ((uint16_t) '2' << 8) + (uint8_t) '~')
#define VTM_PGUP    (int) (((uint32_t) KEY_ESC << 24) + ((uint32_t) '[' << 16) + ((uint16_t) '5' << 8) + (uint8_t) '~')
#define VTM_PGDN    (int) (((uint32_t) KEY_ESC << 24) + ((uint32_t) '[' << 16) + ((uint16_t) '6' << 8) + (uint8_t) '~')
#define VTM_F1      (int) (((uint32_t) KEY_ESC << 16) + ((uint16_t) 'O' << 8) + (uint8_t) 'P')
#define VTM_F2      (int) (((uint32_t) KEY_ESC << 16) + ((uint16_t) 'O' << 8) + (uint8_t) 'Q')
#define VTM_F3      (int) (((uint32_t) KEY_ESC << 16) + ((uint16_t) 'O' << 8) + (uint8_t) 'R')
#define VTM_F4      (int) (((uint32_t) KEY_ESC << 16) + ((uint16_t) 'O' << 8) + (uint8_t) 'S')

/* alternative key codes (confirmed in Windows 10) */
#define WIN_BACKSPC 0x7F
#define WIN_LEFT    (int) (((uint16_t) KEY_ESC << 8) + (uint8_t) 'K')
#define WIN_RIGHT   (int) (((uint16_t) KEY_ESC << 8) + (uint8_t) 'M')
#define WIN_UP      (int) (((uint16_t) KEY_ESC << 8) + (uint8_t) 'H')
#define WIN_DOWN    (int) (((uint16_t) KEY_ESC << 8) + (uint8_t) 'P')
#define WIN_HOME    (int) (((uint16_t) KEY_ESC << 8) + (uint8_t) 'G')
#define WIN_END     (int) (((uint16_t) KEY_ESC << 8) + (uint8_t) 'O')
#define WIN_DEL     (int) (((uint16_t) KEY_ESC << 8) + (uint8_t) 'S')
#define WIN_INS     (int) (((uint16_t) KEY_ESC << 8) + (uint8_t) 'R')
#define WIN_PGUP    (int) (((uint16_t) KEY_ESC << 8) + (uint8_t) 'I')
#define WIN_PGDN    (int) (((uint16_t) KEY_ESC << 8) + (uint8_t) 'Q')
#define WIN_F1      (int) (((uint16_t) KEY_ESC << 8) + (uint8_t) ';')
#define WIN_F2      (int) (((uint16_t) KEY_ESC << 8) + (uint8_t) '<')
#define WIN_F3      (int) (((uint16_t) KEY_ESC << 8) + (uint8_t) '=')
#define WIN_F4      (int) (((uint16_t) KEY_ESC << 8) + (uint8_t) '>')

#ifdef	__cplusplus
}
#endif

#endif	/* RIDE_H */

