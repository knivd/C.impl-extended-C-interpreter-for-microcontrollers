#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <setjmp.h>
#include <ctype.h>
#include <time.h>
#include "ride.h"
#include "fos.h"
#include <conio.h>          /* needed for getch() and kbhit() */
#include <time.h>           /* needed for the clock() function */

#ifndef ETX
#define ETX		((char) 3)	/* ASCII code of the End-Of-Text character */
#define ETXSTR	"\x03"		/* the same ETX character but in string form (needed for the libraries) */
#endif

/* these are the codes of all keys which need to be released by edit_line() for handling outside */
/* this array must finish with code 0 */
const int hl_keys[] = { KEY_UP, VTM_UP, WIN_UP, KEY_DOWN, VTM_DOWN, WIN_DOWN,
						KEY_PGUP, VTM_PGUP, WIN_PGUP, KEY_PGDN, VTM_PGDN, WIN_PGDN,
						KEY_F1, VTM_F1, WIN_F1, CTRL_F, CTRL_R, 0 };

/* history entry structure */
typedef struct {
	char *text;
	unsigned long id;  /* auto-incremental id to indicate the age of an entry */
} log_t;

char *TEXT = NULL;            	/* main text structure for the editor */
unsigned int x_window = 0;      /* start of horizontal window within the line */
unsigned int x_pos = 0;         /* internal horizontal window position */
unsigned int curline = 1;       /* current line in the editor */
unsigned int source = 1;        /* source line for copy/move operations */
unsigned int hpos = 0;          /* horizontal position within a line set by the .. command */
unsigned int helpln_counter;	/* counter of printed help lines */
int errf = 0;                   /* globally set error flag for command execution */
char *find_str = NULL, *repl_str = NULL;
char *filename = NULL;

#if LOG_ENTRIES > 0
unsigned long log_id = 0;       /* unique id for the next log entry */
unsigned char log_index = 0;    /* log store index */
log_t history[LOG_ENTRIES] = { { NULL, 0 } };   /* stored log entries from the previous edits */
#endif


int wait_for_brcont(void) {
    printf("\r\nPress key to continue... ");
    while(kbhit() > 0) getchx();
	int ch = getchx();
    printf("\r                         \r");
    return ch;
}


int help_line(const char *s) {
	printf("%s\r\n", s);
	if((int) ++helpln_counter >= ((int) settings.page_height - 4)) {
		helpln_counter = 0;
		if(wait_for_brcont() == settings.brk_code) { printf("\r\n^cancel\r\n\n"); return 1; }
	}
	return 0;
}


/* print help information */
void help(void) {
	helpln_counter = 0;
    printf("\r\n\n");
    help_line(">>> <Ctrl-Z> History          <Ctrl-N> Next line or add new");
    help_line(">>> <Ctrl-L> Clear full line  <Ctrl-Y> Clear to end of line");
	help_line(">>> <Ctrl-V> Clear screen                                  ");
    help_line(">>> '.' at start of a line marks a command line            ");
    help_line(">>> '@' represents the current line N     '#' source line N");
    help_line(">>> '!' is the number of lines incl current and to the end ");
    if(enable_flags & FLAG_PS2) {
    help_line(".K cntry   Set keyboard layout (US, UK, DE, FR)            ");
	}
    help_line(".^ code    Set keyboard break code (default 3)             ");
    help_line(".N pw, ph  Page width (12-999 chars) and height (2+ lines) ");
    help_line(".T width   TAB width (1-80)                                ");
    help_line(".~         Equivalent to <Del>        .\\  New line <Enter>");
    help_line(".* count   Repeat the following cmd for spec'd number times");
    help_line(".`text`    Insert txt at the cursor pos in the current line");
    help_line(".X [hpos]  Place cursor at spec pos within the current line");
    help_line(".[J] [< or >] [number]  Jump to line/prev/next; '.J' last N");
    help_line(".L [[P]count] List next N lines or until EOF [option Pause]");
    help_line(".P [[P]count] List previous N lines or from start   [Pause]");
    help_line(".I [count] Insert blank lines. One line if [cnt] is missing");
    help_line(".D [count] Delete lines. Only the current with no parameter");
    help_line(".S [line]  Set source line for copy and move ops     .H  .?");
    help_line(".C [count] Copy lines from source to current. Default num=1");
    help_line(".M [count] Move lines from source to current. Default num=1");
    help_line(".F [`text`]  Set text for find; w/o param, do find <Ctrl-F>");
    help_line(".R [`text`]  Set text for replace; w/o, find&repl  <Ctrl-R>");
    printf(".E file    Execute command script from file");

    #if NO_EXIT_RIDE != 0
    help_line("                ");
    #else
    help_line("        ._  Exit");
    #endif

    #ifdef VM_H
    help_line(".= [file]  Run mem or file  .B file  Compile VM asm to file");
    #else
    help_line(".= [file]  Run source in memory or from file               ");
    #endif

    help_line(".O file    Open text file   .W file  Write  .A file  Append");

    #ifdef BOOTLOADER_APP
    help_line("./update      Invoke the bootloader for software update    ");
    #endif

    help_line("./init drv:   Initialise drive  ./lock pwd   Lock access   ");
    help_line("./dir [path][mask]  List files. Can do wildcards '?'and '*'");
    help_line("./chdir [drv:][path]  Change drive and/or dir; Show current");  /* 'cd' also works */
    help_line("./mkdir path  Make new dir      ./rmdir path  Remove empty ");
    help_line("./del mask    Delete files      ./ren  mask_old , mask_new ");
	help_line("./copy mask, mask   Copy files. Supports wildcards '*', '?'");
	help_line("./list file   List text file    ./blist file  List bin file");
	help_line("./date YYMMDD Set date          ./time HHMMSS Set time :24h");
    #ifdef CIMPL
	help_line("./lsl  <lib>  List consts & funcs in C.impl system library ");
    #endif
    help_line("./reset       System reset                                 ");
    printf("\n");
    while(kbhit() > 0) getchx();
}


/* release all previously allocated memory and re-initialise RIDE */
void RIDE_release_memory(void) {
    x_free((byte **) &TEXT);
    #if LOG_ENTRIES > 0
    int t;
    for(t = 0; t < LOG_ENTRIES; t++) {
        x_free((byte **) &history[t].text);
    }
    log_index = log_id = 0;
    #endif
    x_free((byte **) &find_str);
    x_free((byte **) &repl_str);
	x_defrag();
    curline = 1;
	memset(buffer, 0, sizeof(buffer));  /* clear the edit buffer */
}


/* store the settings into a hidden file */
void store_settings(void) {
	settings.checksum = hash((const char *) (&settings + sizeof(settings.checksum)), (sizeof(settings) - sizeof(settings.checksum)));
	f_chdrive(PWD_FILE);
	f_mount(&FatFs, PWD_FILE, 0);
	f_chdir(PWD_FILE);
	UINT wr;
	FRESULT fr = f_open(&File, PWD_FILE, (FA_CREATE_ALWAYS | FA_WRITE));
	if(fr == FR_OK) f_write(&File, &settings, sizeof(ride_settings_t), &wr);
	f_close(&File);
	f_chmod(PWD_FILE, (AM_SYS | AM_HID), (AM_SYS | AM_HID));
	if(fr != FR_OK) errf = errfile(fr, NULL);
}


/* simple hash function */
/* credit: http://www.azillionmonkeys.com/qed/hash.html */
#define get16bits(d) ((((uint32_t) (((const uint8_t *) (d))[1])) << 8) + \
                        (uint32_t) (((const uint8_t *) (d))[0]))
uint32_t hash(const char *data, int len) {
	uint32_t hash = len, tmp;
	int rem;
    if(len <= 0 || data == NULL) return 0;
    rem = len & 3;
    len >>= 2;
    for( ; len > 0; len--) {	/* main loop */
        hash += get16bits(data);
        tmp   = (get16bits(data+2) << 11) ^ hash;
        hash  = (hash << 16) ^ tmp;
        data += 2 * sizeof(uint16_t);
        hash += hash >> 11;
    }
    switch(rem) {	/* handle end cases */
        case 3: hash += get16bits(data);
                hash ^= hash << 16;
                hash ^= ((signed char) data[sizeof(uint16_t)]) << 18;
                hash += hash >> 11;
                break;
        case 2: hash += get16bits(data);
                hash ^= hash << 11;
                hash += hash >> 17;
                break;
        case 1: hash += (signed char) *data;
                hash ^= hash << 10;
                hash += hash >> 1;
        default: break;
    }
	/* force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;
    return hash;
}


/* function "What???" */
static void what(void) {
    printf(" ???\r\n");
    errf = 1;
}


/* show memory allocation error message */
static void errmem(void) {
    printf("\r\n\n>>> ERROR: Unable to allocate memory\r\n\n");
    errf = 1;
}


/* calculate the printable length of a number */
unsigned int plen(unsigned int num) {
    unsigned int v = 0;
    do {
        v++;
        num /= 10;
    } while(num);
    return v;
}


/* store buffer[] into the log */
void store_in_log(void) {
	if(passw_mode) return;
    #if LOG_ENTRIES > 0
    if(*buffer) {
        int t, r;
        for(t = 0; t < LOG_ENTRIES; t++) {  /* check if this string is already in the log */
            if(history[t].text && !strcmp(history[t].text, buffer)) break;
        }
        if(t >= LOG_ENTRIES) {  /* not found - store it */
            unsigned long oldest = 0;
            for(r = t = 0; t < LOG_ENTRIES; t++) {
                if(history[t].text == NULL) { r = t; break; }
                if(history[t].id <= oldest) { r = t; oldest = history[t].id; }
            }
            if(x_malloc((byte **) &history[r].text, strlen(buffer) + 1)) {
                history[r].id = ++log_id;
                strcpy(history[r].text, buffer);    /* will not generate an error if unable to store it */
            }
        }
    }
    #endif
}


/* helper function for edit_line() */
/* print line in buffer[] according to 'x_pos' and 'x_window' */
void edit_line_update(unsigned int line_number, char show_xpos) {
    unsigned int t, slen = 0, max_width = settings.page_width;
    printf("\r");
	if(passw_mode) {
		printf("password:    \b\b\b");
		return;
	}
    slen = strlen(buffer);
    if(line_number) {
        if(show_xpos && (x_window + x_pos)) {
            max_width -= printf("%3d\\ ", (x_window + x_pos + 1));
            if(line_number > 999) { printf(" "); max_width--; }
            if(line_number > 9999) { printf(" "); max_width--; }
            if(line_number > 99999) { printf(" "); max_width--; }
            if(line_number > 999999) { printf(" "); max_width--; }
        }
        else max_width -= printf("%3d: ", line_number);
    }
    for(t = 0; t < max_width; t++) {
        if((x_window + t) < slen) printf("%c", buffer[x_window + t]); else break;
    }
    while(t < max_width) { printf(" "); t++; }  /* fill any remaining space in the line */
    printf("\r");
    if(line_number) {
        if(show_xpos && (x_window + x_pos)) {
            printf("%3d\\ ", (x_window + x_pos + 1));
            if(line_number > 999) printf(" ");
            if(line_number > 9999) printf(" ");
            if(line_number > 99999) printf(" ");
            if(line_number > 999999) printf(" ");
        }
        else printf("%3d: ", line_number);
    }
	for(t = 0; t < x_pos; t++) printf("%c", buffer[x_window + t]);
	if(t >= max_width) printf("\b");
}


/* true integer getch() */
int getchx(void) {
    int ch = (unsigned char) getch();
    if(ch != EOF) {
        if((ch & 0xFF000000) == 0xFF000000) ch &= ~0xFF000000;
        if((ch & 0xFF0000) == 0xFF0000) ch &= ~0xFF0000;
        if((ch & 0xFF00) == 0xFF00) ch &= ~0xFF00;
    }
    if(ch == KEY_ESC || ch == 0 || ch == MULPS2) {  /* Escape sequences passed through the standard byte-long getch() */
        char r = 20;
        while(r-- && kbhit() <= 0) mSec(1);
        if(ch != KEY_ESC) {     /* PS/2 scan codes are two bytes and start with code 0 or MULPS2 */
			if(ch == 0) ch = KEY_ESC;
            ch = (ch << 8) + (unsigned char) getch();
        }
        else if(kbhit() > 0) {  /* multibyte escape codes (> 2 bytes) */
            int c;
            do {
                c = (unsigned char) getch();
                if(c) ch = (ch << 8) + c;
                r = 5;
                while(r-- && kbhit() <= 0) mSec(1);
            } while(c && kbhit() > 0);
        }
    }
    return ch;
}


/* line editor */
/* if *line is specified, the text from it is first copied into the line buffer */
/* if 'line_number' is supplied (greater than zero), it is printed in front of the line */
/* 'x' will position the cursor at specified location within the line */
/* the function returns the new line in buffer[] */
/* exit code is the last pressed key */
int edit_line(char *line, unsigned int line_number, unsigned int x) {
    int ch;
    const int *chh;
    unsigned int max_width = settings.page_width;
    unsigned int x_org = x;
    x_window = 0;
    memset(buffer, 0, sizeof(buffer));
    if(line) strncpy(buffer, line, LINE_WIDTH);
    printf("\r");
    if(line_number) max_width -= printf("%3d: ", line_number);
    x_pos = 0;
    while((x_window + x_pos) < x_org) {
        if((x_window + x_pos + 1) < strlen(buffer)) {
            if((x_pos + 1) < max_width) x_pos++; else x_window++;
        }
        else break;
    }

    while(1) {
        edit_line_update(((*buffer == '.') ? 0 : line_number), (x == 0));
        x = 0; ch = getchx();

        /* check in the key codes which are to be handled at higher level */
        for(chh = hl_keys; *chh; chh++) {
            if(ch == *chh) return ch;
        }

        /* [Esc] */
		if(ch == KEY_ESC) {
            store_in_log();
            memset(buffer, 0, sizeof(buffer));
            if(line) strncpy(buffer, line, LINE_WIDTH); /* restore the original line */
            printf("\\");
            break;
		}

        /* [Enter] */
		if(ch == KEY_ENT || ch == KEY_NLINE || ch == CTRL_N) {
            store_in_log();
            x_pos = x_window = 0;
            break;
        }

        /* [BckSpc] */
        else if(ch == KEY_BSP || ch == VTM_BACKSPC || ch == WIN_BACKSPC) {
            char t, cnt = 1;
            if((x_window + x_pos) >= settings.tab_width) {
                for(cnt = settings.tab_width, t = 1; t <= settings.tab_width && cnt == settings.tab_width; t++) {
                    if(buffer[x_window + x_pos - t] != ' ') cnt = 1;
                }
            }
            while(cnt-- && (x_window + x_pos)) {
                memmove(&buffer[x_window + x_pos - 1], &buffer[x_window + x_pos], (strlen(&buffer[x_window + x_pos]) + 1));
                if(x_pos) x_pos--; else x_window--;
            }
        }

        /* [Del] */
        else if(ch == KEY_DEL || ch == VTM_DEL || ch == WIN_DEL) {
            memmove(&buffer[x_window + x_pos], &buffer[x_window + x_pos + 1], strlen(&buffer[x_window + x_pos]));
        }

		/* [Home] key will move the cursor to the beginning of the text or beginning of the line */
		else if(ch == KEY_HOME || ch == VTM_HOME || ch == WIN_HOME) {
			unsigned int wp0 = x_window, xp0 = x_pos;
			x_pos = x_window = 0;
			while(buffer[x_window + x_pos] == ' ') {
				if((x_window + x_pos + 1) <= strlen(buffer)) {
	                if((x_pos + 1) < max_width) x_pos++; else x_window++;
            	}
			}
			if(x_window == wp0 && x_pos == xp0) x_pos = x_window = 0;
		}

		/* [End] key will move the cursor to the end of the text in the line */
		else if(ch == KEY_END || ch == VTM_END || ch == WIN_END) {
            x_window = 0;
            x_pos = strlen(buffer);
            if(x_pos >= LINE_WIDTH) x_pos = LINE_WIDTH - 1;
            while(x_pos >= max_width) { x_window++; x_pos--; }
		}

        /* left arrow */
		else if(ch == KEY_LEFT || ch == VTM_LEFT || ch == WIN_LEFT) {
			if(x_pos) x_pos--;
            else if(x_window) x_window--;
		}

		/* right arrow */
		else if(ch == KEY_RIGHT || ch == VTM_RIGHT || ch == WIN_RIGHT) {
            if((x_window + x_pos + 1) >= LINE_WIDTH) printf("\a");
            if((x_window + x_pos + 1) <= strlen(buffer)) {
                if((x_pos + 1) < max_width) x_pos++; else x_window++;
            }
		}

        /* tabulation */
        else if(ch == KEY_TAB) {
            int t, tf = 0;
            for(t = 0; (t < settings.tab_width) && ((strlen(buffer) % settings.tab_width) || !t); t++) {
                if((strlen(buffer) + 5) >= LINE_WIDTH && !tf) tf = printf("\a");
                if(strlen(buffer) < LINE_WIDTH) {   /* prevent loss of data at the end of very long strings */
                    memmove(&buffer[x_window + x_pos + 1], &buffer[x_window + x_pos], (LINE_WIDTH - (x_window + x_pos) - 1));
                    buffer[x_window + x_pos] = ' ';
                    if((x_window + x_pos + 1) < LINE_WIDTH) {
                        if((x_pos + 1) < max_width) x_pos++; else x_window++;
                    }
                }
            }
        }

        /* access to log entries */
		else if(ch == CTRL_Z) {
            #if LOG_ENTRIES > 0
            int t;
            for(t = 0; t < LOG_ENTRIES; t++) {
                if(++log_index >= LOG_ENTRIES) log_index = 0;
                if(history[log_index].text) break;
            }
            if(t < LOG_ENTRIES) {   /* found something else in the log */
                t = log_index;
                if(*buffer == '\0') *buffer = '\r';
                store_in_log();
                memset(buffer, 0, sizeof(buffer));
                strcpy(buffer, history[t].text);
                if(*buffer == '\r') *buffer = '\0';
                x_pos = x_window = 0;
            }
            #endif
		}

        /* clear to the end of the line */
        else if(ch == CTRL_Y) {
            memset(&buffer[x_window + x_pos], 0, strlen(&buffer[x_window + x_pos]));
        }

        /* clear the entire line */
        else if(ch == CTRL_L) {
            memset(buffer, 0, sizeof(buffer));
            x_pos = x_window = 0;
        }

        /* all text characters */
        else if((ch >= ' ' && ch != 0x7F && ch < 0x100) || ch < 0) {
            ch = (unsigned char) ch;
            if((strlen(buffer) + 5) >= LINE_WIDTH) printf("\a");
            if(strlen(buffer) < LINE_WIDTH) {   /* prevent loss of data at the end of very long strings */
                memmove(&buffer[x_window + x_pos + 1], &buffer[x_window + x_pos], (LINE_WIDTH - (x_window + x_pos) - 1));
                buffer[x_window + x_pos] = ch;
                if((x_window + x_pos + 1) < LINE_WIDTH) {
                    if((x_pos + 1) < max_width) x_pos++; else x_window++;
                }
                if(*buffer == '.') {    /* clear the line when receiving editor commands */
                    memset(&buffer[x_window + x_pos], 0, strlen(&buffer[x_window + x_pos]));
                }
            }
        }

		/* character codes that will be handled outside */
		else if(ch == CTRL_V) break;

    }
    return ch;
}


/* find and return pointer to line with specified number */
char *find_line(signed long line_number) {
    char *b = TEXT;
	if(line_number < 1) line_number = 1;
	while(*b != ETX && --line_number > 0) {
        int v = (strlen(b) + 1);
        while(*b != ETX && v--) b++;
    }
	return b;
}


/* print specified number of lines */
void print_lines(unsigned int start_line, unsigned int count, unsigned char pause_flag) {
	uint8_t pc = pause_flag;
    char *b = find_line(start_line);
	while(*b != ETX && count--) {
		if(*b || *(b + 1) != ETX) printf("%3d: %s\r\n", start_line++, b);
		b += strlen(b) + 1;
		int k = 0;
		while(kbhit() > 0 || (k == ' ')) {
			k = getchx();
			while(kbhit() > 0) getchx();
			if(k == settings.brk_code) break;
		}
		if(k) {
			if(k == settings.brk_code) {
				printf("\r\n^cancel\r\n");
				break;
			}
			pc = pause_flag;
		}
		if(pc && --pc == 0) {
			if(wait_for_brcont() == settings.brk_code) break;
			pc = pause_flag;
		}
	}
}


/* get number and modify the buffer pointer */
/* will return 0 if it was unable to get a number (or the number itself was 0) */
unsigned int get_number(char **bptr) {
    char *s = *bptr;
    unsigned int number = 0;
    while(*s == ' ') s++;   /* skip leading spaces */
    if(*s == '@') { s++; number = curline; }
    else if(*s == '#') { s++; number = source; }
    else if(*s == '!') {
        s++;
        char *c = find_line(curline);
        while(*c != ETX) {	/* count the number of lines from current to the end */
			number++;
			c += strlen(c) + 1;
		}
    }
    else {
        while(*s >= '0' && *s <= '9') number = ((10 * number) + (*(s++) - '0'));
    }
    *bptr = s;
    return number;
}


/* calculate the size of text from a given line number to the end */
unsigned int text_size(unsigned int lnum) {
	char *b, *d;
	b = d = find_line(lnum);
	while(*b != ETX) b += (strlen(b) + 1);		/* calculate the size of the text */
	return (b - d) + 4;	/* there are four ETX characters at the end of the text (why four??) */
}


/* insert line with given number and text */
/* return pointer to the new line or NULL */
char *insert_line(unsigned int lnum, char *line_text) {
	unsigned int lt = text_size(1);				/* calculate the size of the entire text */
	unsigned int ll = strlen(line_text) + 1;	/* include the trailing 0 */
	if(!x_malloc((byte **) &TEXT, (lt + ll))) return NULL;	/* try to expand for the new line */
	char *b = find_line(lnum);
    lt = text_size(lnum);       /* calculate the size only of the lines from (lnum) till the end */
	memmove((b + ll), b, lt);
	memmove(b, line_text, ll);
    return b;
}


/* delete line with given number */
/* will return -1 if unable to delete, or 0 with success */
int delete_line(unsigned int lnum) {
    unsigned int lt = text_size(1);			/* calculate the size of the entire text */
	unsigned int ln = text_size(lnum + 1);	/* calculate the size only of the lines from (lnum+1) till the end */
	char *b = find_line(lnum);
	if(*b == ETX) return -1;
	unsigned int ll = strlen(b) + 1;	/* calculate the size of the line to be deleted */
	memmove(b, (b + ll), ln);
	return (x_malloc((byte **) &TEXT, (lt - ll)) ? 0 : -1);	/* shrink the text file */
}


/* get file name from text and moves the pointer after it, then opens the file */
FRESULT open_file(char *buf, char cmd) {
    FRESULT fr = FR_OK;
    while(*buf == ' ') buf++;
    char *s = buf + strlen(buf) - 1;
    while(*s && (*s <= ' ' || *s == 0x7F)) *(s--) = '\0';
    if(*buf == '\0' && filename) buf = filename;
    if(*buf) {
        if(buf != filename) {
            x_free((byte **) &filename);
            x_malloc((byte **) &filename, strlen(buf) + 1);
            if(filename) strcpy(filename, buf);
        }
        if(cmd == 'A' || cmd == 'a') fr = f_open(&File, buf, (FA_OPEN_APPEND | FA_WRITE));  /* append */
        else if(cmd == 'W' || cmd == 'w' || cmd == '\002')
                    fr = f_open(&File, buf, (FA_CREATE_ALWAYS | FA_WRITE));                 /* write */
        else fr = f_open(&File, buf, (FA_OPEN_EXISTING | FA_READ));							/* read */
        if(fr != FR_OK) errf = errfile(fr, NULL);
    }
    else if(!filename) what();
    return fr;
}


/* execute file (when 'fn' is supplied) or the source in memory (when 'fn' is NULL) */
/* return >=0 for success; -1 when unable to allocate memory; -2 if there was a file error */
unsigned long run_file(char *fn) {
	unsigned long result = 0;
    file_to_run = fn;
    uint16_t flags_temp = 0;

    if(enable_flags & FLAG_VIDEO) {
        flags_temp = (enable_flags & FLAG_NO_ECHO);
        enable_flags &= ~FLAG_NO_ECHO;
        fontScale = 1; fontBcol = 0; fontFcol = 0xFFFFFF;
        font = (font_t *) &sysFont0508;
    }

    do {
        byte *code = NULL;
        if(file_to_run && *file_to_run) {
            FRESULT fr = f_open(&File, file_to_run, (FA_OPEN_EXISTING | FA_READ));
            if(fr != FR_OK) { f_close(&File); errfile(fr, NULL); return -2; }
            uint32_t fs = f_size(&File);
            RIDE_release_memory();
            x_malloc((byte **) &code, (fs + 4));
            if(!code) { f_close(&File); errmem(); return -1; }
            UINT read;
            fr = f_read(&File, code, fs, &read);
            if(fr != FR_OK || read != fs) { f_close(&File); errfile(fr, NULL); return -2; }
            f_close(&File);
			memset((code + fs), ETX, 4);
        }

		while(kbhit() > 0) getchx();    /* clear the keyboard buffer */

        /* TODO: run (*code) with an external interpreter. This is the only place with a reference outside of RIDE */
        /* the interpreter must clear (*file_to_run) after execution, or have it loaded it with a new file to run */

		printf("\r\n");

        //###vm_run(code);

        #if defined(CIMPL)
        	result = Cimpl(code ? (char *) code : TEXT);

        #else
        	printf("\r\nERROR: Missing code execution engine\r\n");
        	file_to_run = NULL;

        #endif

        x_free((byte **) &code);
    } while(file_to_run);

    if(enable_flags & FLAG_VIDEO) {
        if(flags_temp) enable_flags |= FLAG_NO_ECHO;
        else enable_flags &= ~FLAG_NO_ECHO;
        fontScale = 1; fontBcol = 0; fontFcol = 0xFFFFFF;
        font = (font_t *) &sysFont0508;
    }

    return result;
}


/* execute command line in buf[] */
/* the global flag 'errf' will indicate about errors after execution  */
void execute_cmd_line(char *buf) {
    char *b = buf + 1;  /* position after the leading '.' */
    char *repb = NULL;
    unsigned int count = 0;
    for(errf = 0; !errf; ) {
        while(*b == ' ') b++;

        /* initialise the repeat counter */
        if(*b == '*') {
            if(count > 1) { count--; b = repb; continue; }  /* complete the current loop first */
            b++;
            count = get_number(&b);
            repb = b;
        }

        /* set page width and height */
        else if(*b == 'N' || *b == 'n') {
            b++;
            unsigned int pw = get_number(&b);
            while(*b == ' ') b++;
            if(*b != ',') what();
            b++;    /* skip the comma character */
            unsigned int ph = get_number(&b);
            if(pw < 12 || pw > 999 || ph < 2) what();
            settings.page_width = pw;
            printf("\r\n>>> Page width is set to %u characters", settings.page_width);
            settings.page_height = ph;
            printf("\r\n>>> Page height is set to %u characters", settings.page_height);
            store_settings();
        }

        /* set tabulation width */
        else if(*b == 'T' || *b == 't') {
            b++;
            unsigned int n = get_number(&b);
            if(n >= 1 && n <= 80) {
                settings.tab_width = n;
                printf("\r\n>>> Tabulation width is set to %u characters", settings.tab_width);
                store_settings();
            }
            else what();
        }

        /* jump to line number, to next/previous line, or to the end of the text */
        else if(*b == 'J' || *b == 'j' || *b == '<' || *b == '>' || (*b >= '0' && *b <= '9')) {
            unsigned int n;
            while(1) {
                if(*b >= '1' && *b <= '9') {
                    n = get_number(&b);
                    curline = 1;
                    while(*(find_line(curline + 1)) != ETX && curline < n) curline++;
                }
                else if(*b == '<') {
                    b++;
                    n = get_number(&b);
                    if(n > 0) {
                        while(n-- && curline > 1) curline--;
                    }
                    else {
                        if(curline > 1) curline--;
                    }
                }
                else if(*b == '>') {
                    b++;
                    n = get_number(&b);
                    if(n > 0) {
                        while(n-- && *(find_line(curline + 1)) != ETX) curline++;
                    }
                    else {
                        if(*(find_line(curline + 1)) != ETX) curline++;
                    }
                }
                else if(*b == 'J' || *b == 'j' || *b == '0') {
                    b++;    /* only 'J' without number will jump to the last line in the text */
                    if(*b == 'J' || *b == 'j' || *b == '<' || *b == '>' || (*b >= '1' && *b <= '9')) continue;
                    get_number(&b); /* just ignore */
                    curline = 1;
                    while(*(find_line(curline + 1)) != ETX) curline++;
                }
                break;
            }
        }

        /* list next lines */
        else if(*b == 'L' || *b == 'l') {
            b++;
			while(*b == ' ') b++;   /* skip leading spaces */
			unsigned char pause_flag = 0;
			if(*b == 'P' || *b == 'p') {
                b++;
                pause_flag = ((settings.page_height > 4) ? (settings.page_height - 4) : settings.page_height);
            }
            unsigned int n = get_number(&b);
            if(n == 0) n = (unsigned int) -1;
            printf("\r\n\n");
            print_lines(curline, n, pause_flag);
			while(n-- > 0 && *(find_line(curline + 1)) != ETX) curline++;
        }

        /* list previous lines */
        else if(*b == 'P' || *b == 'p') {
            b++;
			while(*b == ' ') b++;   /* skip leading spaces */
			unsigned char pause_flag = 0;
			if(*b == 'P' || *b == 'p') {
                b++;
                pause_flag = ((settings.page_height > 4) ? (settings.page_height - 4) : settings.page_height);
            }
            unsigned int s, n = get_number(&b);
            if(n == 0) s = 1;   /* list from the start */
            else s = ((curline >= n) ? (curline - n + 1) : 1);  /* list 'n' lines back from the current line */
            printf("\r\n\n");
            print_lines(s, (curline - s), pause_flag);
        }

        /* insert lines from 'curline' onwards */
        else if(*b == 'I' || *b == 'i') {
            b++;
            unsigned int n = get_number(&b);
            if(n < 1) n = 1;
            while(n--) {
				if(!insert_line(curline, "")) break;
			}
        }

        /* delete lines from 'curline' onwards */
        else if(*b == 'D' || *b == 'd') {
            b++;
            unsigned int n = get_number(&b);
            if(n < 1) n = 1;
            while(n--) {
                if(delete_line(curline)) break;
            }
        }

        /* set 'source' line */
        else if(*b == 'S' || *b == 's') {
            b++;
            source = get_number(&b);
        }

        /* copy from 'source' to 'curline' */
        else if(*b == 'C' || *b == 'c') {
            b++;
            unsigned int n = get_number(&b);
            unsigned int t = curline, s = source;
			char *lbuf = NULL;
			x_malloc((byte **) &lbuf, (strlen(buffer) + 1));
			if(!lbuf) errmem();
			strcpy(lbuf, buffer);
            char *ld, *ls;
            while(n--) {
				ls = find_line(source);
                if(*ls == ETX) break;
				strcpy(buffer, ls);
                ld = insert_line(curline, buffer);
                if(!ld) break;
                if(source >= curline) { source++; s++; }
                curline++; source++;
            }
			strcpy(buffer, lbuf);
			x_free((byte **) &lbuf);
            curline = t; source = s;
        }

        /* move from 'source' to 'curline' */
        else if(*b == 'M' || *b == 'm') {
            b++;
            unsigned int n = get_number(&b);
            unsigned int t = curline;
			char *lbuf = NULL;
			x_malloc((byte **) &lbuf, (strlen(buffer) + 1));
			if(!lbuf) errmem();
			strcpy(lbuf, buffer);
            char *ls, *ld;
            while(n--) {
				ls = find_line(source);
                if(*ls == ETX) break;
				strcpy(buffer, ls);
                ld = insert_line(curline, buffer);
                if(!ld) break;
                if(source >= curline) source++;
                curline++;
                if(delete_line(source)) break;
                if(source <= curline) curline--;
            }
			strcpy(buffer, lbuf);
			x_free((byte **) &lbuf);
            source = curline = t;
        }

        /* horizontal coordinate */
        else if(*b == 'X' || *b == 'x') {
            b++;
            hpos = get_number(&b);
            if(hpos) hpos--;    /* the user operates with positions from 1 and above */
        }

        /* break code */
        else if(*b == '^') {
            b++;
            settings.brk_code = get_number(&b);
            store_settings();
        }

        /* keyboard layout */
        else if(*b == 'K' || *b == 'k') {
            b++;
            while(*b == ' ') b++;   /* skip leading spaces */
            char set = 0;
            if(toupper(*b) == 'U' && toupper(*(b+1)) == 'S') { set = 1; settings.kbd_layout = LAYOUT_US; }
			if(toupper(*b) == 'U' && toupper(*(b+1)) == 'K') { set = 1; settings.kbd_layout = LAYOUT_UK; }
            if(toupper(*b) == 'D' && toupper(*(b+1)) == 'E') { set = 1; settings.kbd_layout = LAYOUT_DE; }
			if(toupper(*b) == 'F' && toupper(*(b+1)) == 'R') { set = 1; settings.kbd_layout = LAYOUT_FR; }
            if(*b) b++;
            if(*b) b++;
            if(set) store_settings();
            else what();
        }

        /* <Enter> */
        else if(*b == '\\') {
            b++;
            char *c = find_line(curline + 1);
            if(*c == ETX) c = insert_line(curline + 1, "");
            if(c) curline++;
        }

        /* insert/delete text */
        else if(*b == '`' || *b == '~') {
            char *lt = NULL;
            char *nl = NULL;
            x_malloc((byte **) &nl, (LINE_WIDTH + 1));
            unsigned int max_width = settings.page_width;
            max_width -= plen(curline);
            if(nl) {
                memset(nl, 0, (LINE_WIDTH + 1));
                lt = find_line(curline);
                if(*lt != ETX) strcpy(nl, lt);
                x_window = x_pos = 0;
                while((x_window + x_pos) < hpos) {
                    if((x_window + x_pos + 1) < strlen(nl)) {
                        if((x_pos + 1) < max_width) x_pos++; else x_window++;
                    }
                    else break;
                }
            }

            if(*b == '~') { /* delete characters */
                do {
                    if(nl) memmove(&nl[x_window + x_pos], &nl[x_window + x_pos + 1], strlen(&nl[x_window + x_pos]));
                } while(*(++b) == '~');
            }

            else {  /* add characters */
                b++;
                while(*b && *b != '`') {
                    int ch = *(b++);
                    if(nl && ch >= ' ' && ch != 0x7F) {
                        if((strlen(nl) + 5) >= LINE_WIDTH) printf("\a");
                        if(strlen(nl) < LINE_WIDTH) {   /* prevent loss of data at the end of very long strings */
                            memmove(&nl[x_window + x_pos + 1], &nl[x_window + x_pos], (LINE_WIDTH - (x_window + x_pos) - 1));
                            nl[x_window + x_pos] = ch;
                            if((x_window + x_pos + 1) < LINE_WIDTH) {
                                if((x_pos + 1) < max_width) x_pos++; else x_window++;
                            }
                        }
                    }
                }
                if(*b == '`') b++;
            }

            if(nl && lt) {
				delete_line(curline);
				char *c = insert_line(curline, nl);
				if(!c) errmem();
                x_free((byte **) &nl);
            }
            else errmem();
        }

        /* find and find/replace string */
        else if(*b == 'F' || *b == 'f' || *b == 'R' || *b == 'r') {
            char cmd = *(b++);
            char *lz, *p, *s = NULL;
            while(*b == ' ') b++;

            if(*b == '`') { /* define string to search for */
                b++;
                char *bb = b;
                while(*b && *b != '`') b++;
                if(cmd == 'F' || cmd == 'f') {
                    x_free((byte **) &find_str);
                    x_malloc((byte **) &find_str, (b - bb + 1));
                    memset(find_str, 0, (b - bb + 1));
                    memcpy(find_str, bb, (b - bb));
                    printf("\r\n>>> Defined `%s` to find", find_str);
                }
                else {
                    x_free((byte **) &repl_str);
                    x_malloc((byte **) &repl_str, (b - bb + 1));
                    memset(repl_str, 0, (b - bb + 1));
                    memcpy(repl_str, bb, (b - bb));
                    printf("\r\n>>> Defined `%s` to replace", repl_str);
                }
                if(*b == '`') b++; else what();
            }

            else if(find_str && *find_str) {    /* perform search */
                lz = find_line(curline);
                if(*lz != ETX) { 	/* search in the current line */
                    p = &lz[x_window + x_pos + 1];
                    s = strstr(p, find_str);
                    if(s) hpos = x_window + x_pos + 1 + (s - p);
                }
                while(*lz != ETX && !s) {
                    lz = find_line(curline + 1);
                    if(*lz != ETX) {
                        curline++;
                        if(lz) s = strstr(lz, find_str);
                        if(s) hpos = s - lz;
                    }
                }
                if(!s) printf("\r\n>>> `%s` was not found", find_str);
            }

            if(s && (cmd == 'R' || cmd == 'r')) {   /* when there is a string found and needs to be replaced */
                unsigned int newlen = strlen(lz) - strlen(find_str);  /* 'lz' is already known here */
                unsigned int rlen = ((repl_str) ? strlen(repl_str) : 0);
                if((newlen + rlen) > LINE_WIDTH) rlen = LINE_WIDTH - newlen;
                newlen += rlen;
                char *newstr = NULL;
                x_malloc((byte **) &newstr, (newlen + 1));
                if(newstr) {
                    memset(newstr, 0, newlen + 1);
                    strcpy(newstr, lz);
                    s = &newstr[hpos];  /* 'hpos' was already set by the 'find' function */
                    memmove(s, (s + strlen(find_str)), (strlen(s + strlen(find_str)) + 1)); /* delete the 'find' string */
                    if(rlen) {
                        memmove((s + rlen), s, (strlen(s) + 1));    /* make room for the 'replace' string */
                        memcpy(s, repl_str, rlen);  /* insert the 'replace' string */
                    }
					delete_line(curline);
					if(!insert_line(curline, newstr)) errmem();
                    x_free((byte **) &newstr);
                }
                else errmem();
            }

        }

        /* write file or append to file */
        else if(*b == 'W' || *b == 'w' || *b == 'A' || *b == 'a' || *b == '\002') { /* code 2 is writing without messages */
            char cmd = *(b++);
			while(*b == ' ') b++;
			if(*b == '\0') {
				if(filename && *filename) b = filename;
				else { what(); break; }
			}
            if(strchr(b, ':')) {
                f_chdrive(b);
                f_mount(&FatFs, b, 0);
            }
        	f_chdir(b);
            FRESULT fr = open_file(b, cmd);
            if(fr == FR_OK) {
                if(cmd != '\002') printf("\r\n>>> Writing to file `%s` ... ", (filename ? filename : ""));
                unsigned long total = 0;
				unsigned int wc, line = 1;
				char *lz = TEXT, *lz_next = TEXT;
				while(lz && *lz != ETX && fr == FR_OK) {
					wc = f_puts(lz, &File);
					wc += f_puts("\n", &File);
					if(wc != strlen(lz) + 1) { fr = FR_DISK_ERR; break; }
					total += wc;
					lz = find_line(++line);
                    lz_next = find_line(line + 1);  /* look ahead for ETX */
                    if(lz && *lz == '\0' && (!lz_next || *lz_next == ETX)) break;
				}
                if(fr != FR_OK) errf = errfile(fr, NULL);
                if(cmd != '\002') printf("\r\n>>> %lu lines, %lu bytes written\r\n", (unsigned long) (line - 1), total);
            }
            f_close(&File);
            break;  /* no more commands are executed in this line since the file name was to the end of it */
        }

        /* open file */
        else if(*b == 'O' || *b == 'o' || *b == '\001') {   /* code 1 is a special case of the command without messages */
            char cmd = *(b++);
			while(*b == ' ') b++;
			if(*b == '\0') {
				if(filename && *filename) b = filename;
				else { what(); break; }
			}
            if(strchr(b, ':')) {
                f_chdrive(b);
                f_mount(&FatFs, b, 0);
            }
        	f_chdir(b);
            FRESULT fr = open_file(b, cmd);
            if(fr == FR_OK) {
                if(cmd != '\001') printf("\r\n>>> Reading from file `%s` ...", (filename ? filename : ""));
                RIDE_release_memory();
				x_malloc((byte **) &TEXT, 4);
				if(!TEXT) { errmem(); break; }
				memset(TEXT, ETX, 4);
                curline = 1;
                unsigned long total = 0;
				unsigned long trimmed = 0;
                char *lz = TEXT;
                char *stbuff = NULL;
                x_malloc((byte **) &stbuff, LINE_WIDTH + 1);    /* temporary buffer for reading from the file */
                if(!stbuff) errmem();
                while(stbuff && !f_eof(&File) && !f_error(&File)) {
                    memset(stbuff, 0, (LINE_WIDTH + 1));
                    f_gets(stbuff, LINE_WIDTH, &File);
					if(*stbuff) {
	                    char *s = stbuff + strlen(stbuff) - 1;
	                    while(*s && *s != '\t' && (*s <= ' ' || *s == 0x7F)) {
							if(*s != '\n') trimmed++;
							*(s--) = '\0';
						}
	                    s = stbuff;
	                    do {
	                        if(*s && *s != '\t' && (*s < ' ' || *s == 0x7F)) *s = ' ';  /* replace non-printables with space */
	                    } while(*(++s));
					}
                    lz = insert_line(curline++, stbuff);
                    if(!lz) { errmem(); break; }
					total += strlen(stbuff) + 1;
                }
                if(fr != FR_OK) errf = errfile(fr, NULL);
                x_free((byte **) &stbuff);
                if(cmd != '\001') {
					printf("\r\n>>> %lu lines, %lu bytes read", (unsigned long) (curline ? (curline - 1) : 0), total);
					if(trimmed) printf(", %lu bytes trimmed", trimmed);
					printf("\r\n");
				}
                if(!curline) curline++;
            }
            f_close(&File);
            break;  /* no more commands are executed in this line since the file name was to the end of it */
        }

        /* execute command script file */
        else if(*b == 'E' || *b == 'e') {
            b++;
			while(*b == ' ') b++;
			if(*b == '\0') { what(); break; }
            if(strchr(b, ':')) {
                f_chdrive(b);
                f_mount(&FatFs, b, 0);
            }
        	f_chdir(b);
            FRESULT fr = open_file(b, 'R');
            if(fr == FR_OK) {
                printf("\r\n>>> Executing command script file `%s` ... \r\n", (filename ? filename : ""));
                char *stbuff = NULL;
                x_malloc((byte **) &stbuff, LINE_WIDTH + 1);    /* temporary buffer for reading from the file */
                if(!stbuff) errmem();
                while(stbuff && !f_eof(&File) && !f_error(&File)) {
                    memset(stbuff, 0, (LINE_WIDTH + 1));
                    *stbuff = '.';  /* pre-load the command character to make sure only commands are being executed */
                    f_gets((stbuff + 1), (LINE_WIDTH - 1), &File);
                    if(*(stbuff +1) == '.') *(stbuff +1) = ' ';
                    char *s = stbuff + strlen(stbuff) - 1;
                    while(*s && *s != '\t' && (*s <= ' ' || *s == 0x7F)) *(s--) = '\0';
                    s = stbuff;
                    do {
                        if(*s && *s != '\t' && (*s < ' ' || *s == 0x7F)) *s = ' ';  /* replace non-printables with space */
                    } while(*(++s));
                    execute_cmd_line(stbuff);
                }
                if(fr != FR_OK) errf = errfile(fr, NULL);
                x_free((byte **) &stbuff);
            }
            f_close(&File);
            break;  /* no more commands are executed in this line since the file name was to the end of it */
        }

        /* run file */
        else if(*b == '=') {
            b++;
            while(*b == ' ') b++;
            if(!errf) {
                printf("\r\n");
                run_file(b);	/* run the source in memory */
            }
			break;
        }

		/* compile and create an executable binary file */
		#ifdef VM_H
        else if(*b == 'B' || *b == 'b') {
            b++;
			while(*b == ' ') b++;
			if(*b == '\0') { what(); break; }
            if(strchr(b, ':')) {
                f_chdrive(b);
                f_mount(&FatFs, b, 0);
            }
        	f_chdir(b);
            FRESULT fr = open_file(b, 'W');
            if(fr == FR_OK) {
                printf("\r\n>>> Compiling to file `%s` ... \r\n", (filename ? filename : ""));

				// ### TODO: compiler?

            }
            f_close(&File);
            break;  /* no more commands are executed in this line since the file name was to the end of it */
        }
		#endif

        /* exit */
        #if NO_EXIT_RIDE == 0
        else if(*b == '_') {
            printf("\r\n\n");
            /* this is what is necessary to execute in order to release ALL allocated memory
            RIDE_release_memory();
            x_free((byte **) &filename);
            */
            return;
        }
        #endif

        /* help */
        else if(*b == 'H' || *b == 'h') {
            b++;
            help();
        }

        /* information */
        else if(*b == '?') {
            b++;
            printf("\r\n>>> HINT: Use command .H for help\r\n\n");
			printf(" Free memory: %lu bytes\r\n", ((unsigned long) x_total() >> 7) << 7);
            time_t t0 = time(NULL);
			printf("Current time: %s\r", ctime(&t0));
            execute_cmd_fos("\003d");	/* show the current path */
            printf(".... File name: `%s`\r\n", (filename ? filename : ""));
            printf(".. Find string: `%s`\r\n", (find_str ? find_str : ""));
            printf("Replace string: `%s`\r\n", (repl_str ? repl_str : ""));
            unsigned int line = 1;
            unsigned long total = 0;
            char *lz = TEXT;
            while(lz && *lz != ETX) {
                lz = find_line(line);
                if(*lz != ETX) { line++; total += strlen(lz) + 1; }
            }
            printf("%lu lines, %lu bytes\r\n\n", (unsigned long) line - 1, total);
        }
        else if(*b) { what(); break; }

        /* reached the end of command string */
        if(*b == '\0') {
            if(count > 1) { count--; b = repb; }    /* execute repeat */
            else break;
        }

    }
    printf("\r\n");
}


/* interpret a single help string up to a comma or end */
char *interpret_help(char *help_str, char *str) {
	char *c = help_str;
	while(c && *c && *c != ',') {
		char nosp = 0;
		if(*c == '*') { nosp = 1; sprintf(&str[strlen(str)], "*"); }
		if(*c == 'v') sprintf(&str[strlen(str)], "void");
		if(*c == 'B') sprintf(&str[strlen(str)], "BOOL");
		if(*c == 'c') sprintf(&str[strlen(str)], "char");
		if(*c == 's') sprintf(&str[strlen(str)], "short");
		if(*c == 'i') sprintf(&str[strlen(str)], "int");
		if(*c == 'l') sprintf(&str[strlen(str)], "long");
		if(*c == 'L') sprintf(&str[strlen(str)], "long long");
		if(*c == 'f') sprintf(&str[strlen(str)], "float");
		if(*c == 'd') sprintf(&str[strlen(str)], "double");
		if(*c == 'D') sprintf(&str[strlen(str)], "long double");
		if(*c == 'F') sprintf(&str[strlen(str)], "FILE");
		if(*c == 'z') sprintf(&str[strlen(str)], "size_t");
		if(*c == 'C') sprintf(&str[strlen(str)], "const");
		if(*c == 'u') sprintf(&str[strlen(str)], "unsigned");
		if(*c == '.') sprintf(&str[strlen(str)], "...");
		if(*c == 't') sprintf(&str[strlen(str)], "time_t");
		if(*c == 'm') sprintf(&str[strlen(str)], "struct tm");
		c++;
		if(*c && *c != ',' && !nosp) sprintf(&str[strlen(str)], " ");
	}
	return c;
}


/* main RIDE function */
void RIDE(void) {
	int ch, bkk = settings.brk_code;

	/* main loop in RIDE */
    printf("Use .? for information\r\n\n\n");
    for(file_to_run = NULL; ; settings.brk_code = bkk) {

		if(!TEXT) {	/* initially allocate text for the end of file */
			x_malloc((byte **) &TEXT, 4);
			if(!TEXT) { errmem(); break; }
			memset(TEXT, ETX, 4);
		}

		while(file_to_run && *file_to_run) {
			if(run_file(file_to_run) != 0) break;	/* check if there is a file to run automatically */
		}

        char *lz = find_line(curline);
        if(*lz == ETX) lz = insert_line(curline, "");	/* don't allow unitialised text */
        ch = edit_line(lz, curline, hpos);
        hpos = 0;   /* hpos is only valid once */

        if(*buffer != '.') {    /* the first character in the line is not '.' so it is handled as text */

            if(lz == NULL || strcmp(lz, buffer)) {	/* update the text in this line, if necessary */
				delete_line(curline);
				char *c = insert_line(curline, buffer);
				if(!c) errmem();
            }

            if(ch == KEY_ENT || ch == KEY_NLINE || ch == CTRL_N) {
                edit_line_update(curline, 0);
				char *lzz = find_line(curline + 1);
                if(*lzz != ETX || *lz || ch == CTRL_N) { curline++; printf("\r\n"); }
            }

            else if(ch == KEY_DOWN || ch == VTM_DOWN || ch == WIN_DOWN) {
                if(*(find_line(curline + 1)) != ETX) curline++;
            }

    		else if(ch == KEY_UP || ch == VTM_UP || ch == WIN_UP) {
                if(curline > 1) curline--;
            }

            else if(ch == KEY_PGDN || ch == VTM_PGDN || ch == WIN_PGDN) {
                unsigned int n = settings.page_height;
                while(n--) {
                    if(*(find_line(curline + 1)) != ETX) curline++;
                }
                n = ((curline >= settings.page_height) ? (curline - settings.page_height + 1) : 1);
                printf("\r\n\n");
                print_lines(n, (curline - n), 0);
            }

            else if(ch == KEY_PGUP || ch == VTM_PGUP || ch == WIN_PGUP) {
                unsigned int n = settings.page_height;
                while(n--) {
                    if(curline > 1) curline--;
                }
                n = ((curline >= settings.page_height) ? (curline - settings.page_height + 1) : 1);
                printf("\r\n\n");
                print_lines(n, (curline - n), 0);
            }

            else if(ch == KEY_F1 || ch == VTM_F1 || ch == WIN_F1) {
                help();
            }

            else if(ch == CTRL_F) {
                char *cmd = ".F";
                execute_cmd_line(cmd);
            }

            else if(ch == CTRL_R) {
                char *cmd = ".R";
                execute_cmd_line(cmd);
            }

			else if(ch == CTRL_V) {
				printf("\r");
                ch = 100; while(ch--) printf("\n");
            }

        }

        else {
            printf("\r%s", buffer);
            char *c = buffer + 1;
            while(*c == ' ') c++;
            if(*c != '/') execute_cmd_line(buffer); /* parse and execute command string */
            else {	/* ./ commands */

				if(!strncmp(c, "/lock", 5)) {		/* ./lock command */
					c += 5;							/* unlocking is ./lock with no parameters */
					while(*c == ' ') c++;
					if(*c) settings.stored_password = hash(c, strlen(c));
					else settings.stored_password = 0;
            		store_settings();
				}

				else if(!strncmp(c, "/reset", 6)) {	/* ./reset command */
					c += 6;
					printf("\r\n>>> Resetting... ");
					mSec(500);
					reset();
				}

				else if(!strncmp(c, "/date", 5)) {	/* ./date command */
					c += 5;
                    printf("\r\n");
					uint32_t dt = get_number(&c);
					int y = (dt / 10000) % 100;
					int m = (dt / 100) % 100;
					int d = dt % 100;
					if(m >= 1 && m <= 12 && d >= 1 && d <= 31) {
						struct tm *tt = localtime((const time_t *) &sUTime);
                        struct tm ttt;
                        memcpy(&ttt, tt, sizeof(struct tm));
						ttt.tm_year = 100 + y;
						ttt.tm_mon = m - 1;
						ttt.tm_mday = d;
                        ttt.tm_isdst = 0;
                        sUTime = mktime(&ttt);
                        enable_flags |= FLAG_RTC_UPDATE;
					}
					else what();
				}

				else if(!strncmp(c, "/time", 5)) {	/* ./time command */
					c += 5;
                    printf("\r\n");
					uint32_t dt = get_number(&c);
					int h = (dt / 10000) % 100;
					int m = (dt / 100) % 100;
					int s = dt % 100;
					if(h <= 23 && m <= 59 && s <= 59) {
						struct tm *tt = localtime((const time_t *) &sUTime);
                        struct tm ttt;
                        memcpy(&ttt, tt, sizeof(struct tm));
						ttt.tm_hour = h;
						ttt.tm_min = m;
						ttt.tm_sec = s;
                        ttt.tm_isdst = 0;
                        sUTime = mktime(&ttt);
                        enable_flags |= FLAG_RTC_UPDATE;
					}
					else what();
				}

                #ifdef CIMPL
				else if(!strncmp(c, "/lsl", 4)) {	/* ./lsl command */
					c += 4;							/* unlocking is ./lock with no parameters */
					while(*c == ' ') c++;
					helpln_counter = 0;
					char bf = 0;
					const system_library_t *sl = system_libs;
					if(*c) {
				        while(sl->name && strncmp(c, sl->name, strlen(sl->name))) sl++;
				        if(sl->name) {  /* found the library */
							char str[100];
							help_line("\r\n");
							if(sl->src && *sl->src && !bf) {
								unsigned int hcnt = 0;
								const char *s = sl->src;
								while(*s && *s != ETX && !bf) {
									printf("%c", *s);
									if(*(s++) == '\n' || hcnt >= settings.page_width) {
										hcnt = 0;
										if((int) ++helpln_counter >= ((int) settings.page_height - 2)) {
											helpln_counter = 0;
											if(wait_for_brcont() == settings.brk_code) { printf("\r\n^cancel"); break; }
										}
									}
								}
								if(!bf) help_line("");
							}
							const sys_const_t *cl = sl->const_table;
							while(cl && cl->nlen && !bf) {	/* list the library constants */
								sprintf(str, "const ");
								if(cl->data.type & FT_UNSIGNED) sprintf(&str[strlen(str)], "unsigned ");
								if((cl->data.type & DT_MASK) == DT_VOID) sprintf(&str[strlen(str)], "void ");
								if((cl->data.type & DT_MASK) == DT_BOOL) sprintf(&str[strlen(str)], "BOOL ");
								if((cl->data.type & DT_MASK) == DT_CHAR) sprintf(&str[strlen(str)], "char ");
								if((cl->data.type & DT_MASK) == DT_SHORT) sprintf(&str[strlen(str)], "short ");
								if((cl->data.type & DT_MASK) == DT_INT) sprintf(&str[strlen(str)], "int ");
								if((cl->data.type & DT_MASK) == DT_LONG) sprintf(&str[strlen(str)], "long ");
								if((cl->data.type & DT_MASK) == DT_LLONG) sprintf(&str[strlen(str)], "long long ");
								if((cl->data.type & DT_MASK) == DT_FLOAT) sprintf(&str[strlen(str)], "float ");
								if((cl->data.type & DT_MASK) == DT_DOUBLE) sprintf(&str[strlen(str)], "double ");
								if((cl->data.type & DT_MASK) == DT_LDOUBLE) sprintf(&str[strlen(str)], "long double ");
								if((cl->data.type & DT_MASK) == DT_FILE) sprintf(&str[strlen(str)], "FILE ");
								if((cl->data.type & DT_MASK) == DT_VA_LIST) sprintf(&str[strlen(str)], "va_list ");
								if(cl->data.type & FT_CONSTPTR) sprintf(&str[strlen(str)], "const ");
								unsigned int pp = cl->data.ind;
								while(pp--) sprintf(&str[strlen(str)], "*");
								sprintf(&str[strlen(str)], "%s", cl->name);
								for(pp = 0; pp < MAX_DIMENSIONS && cl->data.dim[pp]; pp++) {
									sprintf(&str[strlen(str)], "[%d]", cl->data.dim[pp]);
								}
								if((cl->data.ind == 1 || (cl->data.dim[0] && (cl->data.dim[1] == 0))) &&
										(cl->data.type & DT_MASK) == DT_CHAR) {
									sprintf(&str[strlen(str)], " = \"%s\"", (char *) (uintptr_t) cl->data.val.i);
								}
								bf = help_line(str);
						        cl++;
						    }
							if(!bf) help_line("");
	    					const sys_func_t *fl = sl->func_table;
							while(fl && fl->nlen && !bf) {	/* list the library functions */
								if(fl->help && *(fl->help)) {
									if(*fl->help != ':') {
										*str = '\0';
										char *s = interpret_help(fl->help, str);
										if(*str && str[strlen(str) - 1] != '*') sprintf(&str[strlen(str)], " ");
										sprintf(&str[strlen(str)], "%s(", fl->name);
										while(*s == ',') {
											s = interpret_help(++s, str);
											if(*s == ',') sprintf(&str[strlen(str)], ", ");
										}
										sprintf(&str[strlen(str)], ")");
										bf = help_line(str);
									}
									else bf = help_line(fl->help + 1);
								}
								else {
									sprintf(str, "%s()", fl->name);
									bf = help_line(str);
								}
						        fl++;
						    }
				        }
						else what();	/* unknown library */
					}
					else {	/* without parameter - list the built-in libraries */
						help_line("\r\n");
						while(sl->name) {
							if(help_line(sl->name)) break;
							sl++;
						}
					}
					if(!bf) help_line("");
				}
                #endif

				else execute_cmd_fos(c + 1);	/* execute file operating system command (only one per command line) */
			}
        }

    }   /* all allocated memory remains after exit from RIDE so it can be continued with a new call */
    x_free((byte **) &filename);
}

