/* hardware-independent graphic functions */
/* in order to work, basic routines setPixel(x,y,c) and getPixel(x,y) are required from outside */
/* also required - Hres and Vres constants specifying the horizontal and vertical resolution */

#ifndef GRAPHICS_H
#define	GRAPHICS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>

#define DEFAULT_VMODE   0   // default video mode on start

#define COL_SOLID       INT_MAX
#define COL_NONE        0
#define COL_INVERT     -1
#define COL_TRANSP     -2
#define COL_GREY50     -3
#define COL_GREY25     -4
#define COL_HLN50      -5
#define COL_HLN33      -6
#define COL_HLN25      -7
#define COL_VLN50      -8
#define COL_VLN33      -9
#define COL_VLN25      -10
#define COL_DGL33      -12
#define COL_DGL25      -13
#define COL_DGR33      -15
#define COL_DGR25      -16
#define COL_SPL33      -17
#define COL_SPL25      -18
#define COL_SPR33      -19
#define COL_SPR25      -20

// font definition header
typedef struct {
	unsigned short start;		// code of the first character in the font
	unsigned short characters;	// number of character definitions in the font
	unsigned char width;		// font width
								// This parameter specifies the number of columns in the characters.
								// In fonts where the field (.width) is 0, every character definition
								// starts with a byte that defines how many columns are present in
								// this character. The actual number of bytes to follow depend on the
								// height of the font as well: for fonts with height 8 lines or less,
								// every byte represents one column, for fonts with height 16 lines
								// or less, every column takes two bytes, and so on
								// in fonts with fixed width where (.width) is greater than 0, the
								// leading width-specifying byte in every definition is missing since
								// the width is already know for all characters
	unsigned char height;		// character definition height in pixels
								// This parameter also automatically defines the number of
								// bytes needed for one column of the character
	unsigned char blankL;		// number of blank columns to add on the left side of every character
	unsigned char blankR;		// number of blank columns to add on the right side of every character
	unsigned char blankT;		// number of blank rows to add on the top side of every character
	unsigned char blankB;		// number of blank rows to add on the bottom side of every character
	char *name;					// optional font name as ASCIIZ string
} font_header_t;

typedef struct {
	font_header_t header;
	unsigned char definitions[];
} font_t;

extern const font_t sysFont0508;    // system font constant

font_t *font;               // pointer to the currently used font
uint8_t fontScale;          // font scaling factor
int32_t fontFcol, fontBcol; // font colours
int16_t posX, posY;         // virtual cursor position

#define TABSTR  "  "        // graphical representation of a tabulation

void drawChar(int ch);      // draw character at pixel coordinates (posX,posY) and modify them accordingly
void drawLine(int x1, int y1, int x2, int y2, int c);
void drawFrame(int x1, int y1, int x2, int y2, int c);
void drawRect(int x1, int y1, int x2, int y2, int c);
void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, int c);
void drawCircle(int x, int y, int r, int c);
void drawEllipse(int x0, int y0, int rx, int ry, double tiltAngle, int c);
void drawSector(int x, int y, int rx, int ry, double tiltAngle, double startAngle, double endAngle, int c);
void drawShape(int x, int y, double tiltAngle, char *def);
void floodFill(int x, int y, int c);
void getRect(void *gbuffer, int x1, int y1, int x2, int y2);
void putRect(void *gbuffer, int x1, int y1, int func);

#ifdef	__cplusplus
}
#endif

#endif
