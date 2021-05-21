#ifdef CIMPL
#ifndef LIB_GRAPHICS_H
#define LIB_GRAPHICS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../cimpl.h"

extern const sys_const_t graphics_const_table[];
extern const sys_func_t graphics_func_table[];

#define graphics_src \
    "typedef struct FONT_HDR {\r\n" \
    "  unsigned short start;\r\n" \
    "  unsigned short characters;\r\n" \
    "  unsigned char width;\r\n" \
    "  unsigned char height;\r\n" \
    "  unsigned char blankL;\r\n" \
    "  unsigned char blankR;\r\n" \
    "  unsigned char blankT;\r\n" \
    "  unsigned char blankB;\r\n" \
    "  char *name;\r\n" \
    "};\r\n" \
    "\r\n" \
    "typedef struct FONT {\r\n" \
    "  FONT_HDR header;\r\n" \
    "  unsigned char definitions[];\r\n" \
    "};\r\n" \
    ETXSTR

void graph_init(void);

void sf_drawLine(void);     /* void drawLine(int x1, int y1, int x2, int y2, int c) */
void sf_drawFrame(void);    /* void drawFrame(int x1, int y1, int x2, int y2, int c) */
void sf_drawRect(void);     /* void drawRect(int x1, int y1, int x2, int y2, int c) */
void sf_drawTriangle(void); /* void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, int c) */
void sf_drawCircle(void);   /* void drawCircle(int x, int y, int r, int c) */
void sf_drawEllipse(void);  /* void drawEllipse(int x0, int y0, int rx, int ry, double tiltAngle, int c) */
void sf_drawSector(void);   /* void drawSector(int x, int y, int rx, int ry, double tiltAngle,
                             *                                  double startAngle, double endAngle, int c) */
void sf_drawShape(void);    /* void drawShape(int x, int y, double tiltAngle, char *def) */
void sf_drawChar(void);     /* void drawChar(int ch) */
void sf_floodFill(void);    /* void floodFill(int x, int y, int c) */

void sf_getRect(void);      /* void getRect(void *buffer, int x1, int y1, int x2, int y2) */
void sf_putRect(void);      /* void putRect(void *buffer, int x, int y, int func) */

void sf_lockScroll(void);   /* lock screen scroll when printing characters */
void sf_unlockScroll(void); /* unlock screen scroll when printing characters */

void sf_posX(void);         /* void posX(int x)             set */
                            /* int posX(void)               get */
void sf_posY(void);         /* void posY(int x)             set */
                            /* int posY(void)               get */
void sf_fontScale(void);    /* void fontScale(int factor)   set */
                            /* int fontScale(void)          get */
void sf_fontFcol(void);     /* void fontFcol(int col)       set */
                            /* int fontFcol(void)           get */
void sf_fontBcol(void);     /* void fontBcol(int col)       set */
                            /* int fontBcol(void)           set */
void sf_fontSet(void);      /* void fontSet(font_t *font) */

#ifdef __cplusplus
}
#endif

#endif  /* LIB_GRAPHICS_H */
#endif
