#ifdef CIMPL

#include "l_graph.h"
#include "../../config.h"
#include "../../ride/graphics.h"
#include "../../ride/kbdcodes.h"

#define SYS_FONT_NAME       "sysFont0508"
#define SYS_FONT_NAME_LEN   11

/* system function handlers take parameters from *prog and return result in acc[accN] */

const sys_const_t graphics_const_table[] = {
    
    /* fonts */
    /* note: the actual values are loaded in the graph_init() function */
    { { { .i = 0 }, (FT_CONST | DT_VOID), 1, {0} },  SYS_FONT_NAME, SYS_FONT_NAME_LEN },

    /* drawing colours */
    { { { .i = INT_MAX },  (FT_CONST | DT_INT), 0, {0} }, "COL_SOLID", 9 }, /* main colour */
    { { { .i = 0 },  (FT_CONST | DT_INT), 0, {0} },     "COL_NONE", 8 },    /* no colour (black) */
    { { { .i = -1 },  (FT_CONST | DT_INT), 0, {0} },    "COL_INVERT", 10 }, /* invert colour */
    { { { .i = -2 },  (FT_CONST | DT_INT), 0, {0} },    "COL_TRANSP", 10 }, /* transparent (no draw) */
    { { { .i = -3 },  (FT_CONST | DT_INT), 0, {0} },    "COL_GREY50", 10 }, /* 50% duty */
    { { { .i = -4 },  (FT_CONST | DT_INT), 0, {0} },    "COL_GREY25", 10 }, /* 25% duty */
    { { { .i = -5 },  (FT_CONST | DT_INT), 0, {0} },    "COL_HLN50", 9 },   /* horizontal lines 50% */
    { { { .i = -6 },  (FT_CONST | DT_INT), 0, {0} },    "COL_HLN33", 9 },   /* horizontal lines 33% */
    { { { .i = -7 },  (FT_CONST | DT_INT), 0, {0} },    "COL_HLN25", 9 },   /* horizontal lines 25% */
    { { { .i = -8 },  (FT_CONST | DT_INT), 0, {0} },    "COL_VLN50", 9 },   /* vertical lines 50% */
    { { { .i = -9 },  (FT_CONST | DT_INT), 0, {0} },    "COL_VLN33", 9 },   /* vertical lines 33% */
    { { { .i = -10 }, (FT_CONST | DT_INT), 0, {0} },    "COL_VLN25", 9 },   /* vertical lines 25% */
    { { { .i = -12 }, (FT_CONST | DT_INT), 0, {0} },    "COL_DGL33", 9 },   /* diagonal to left lines 33% */
    { { { .i = -13 }, (FT_CONST | DT_INT), 0, {0} },    "COL_DGL25", 9 },   /* diagonal to left lines 25% */
    { { { .i = -15 }, (FT_CONST | DT_INT), 0, {0} },    "COL_DRG33", 9 },   /* diagonal to right lines 33% */
    { { { .i = -16 }, (FT_CONST | DT_INT), 0, {0} },    "COL_DRG25", 9 },   /* diagonal to right lines 25% */
    { { { .i = -17 }, (FT_CONST | DT_INT), 0, {0} },    "COL_SPL33", 9 },   /* special pattern left 33% */
    { { { .i = -18 }, (FT_CONST | DT_INT), 0, {0} },    "COL_SPL25", 9 },   /* special pattern left 25% */
    { { { .i = -19 }, (FT_CONST | DT_INT), 0, {0} },    "COL_SPR33", 9 },   /* special pattern right 33% */
    { { { .i = -20 }, (FT_CONST | DT_INT), 0, {0} },    "COL_SPR25", 9 },   /* special pattern right 25% */
    
    /* putRect() operation constants */
    { { { .i = 0 },   (FT_CONST | DT_INT), 0, {0} },    "OPR_COPY", 8 },    /* "copy" operation for putRect() */
    { { { .i = 1 },   (FT_CONST | DT_INT), 0, {0} },    "OPR_OR", 6 },      /* "OR" operation for putRect() */
    { { { .i = 2 },   (FT_CONST | DT_INT), 0, {0} },    "OPR_XOR", 7 },     /* "XOR" operation for putRect() */
    { { { .i = 3 },   (FT_CONST | DT_INT), 0, {0} },    "OPR_AND", 7 },     /* "AND" operation for putRect() */
    { { { .i = 4 },   (FT_CONST | DT_INT), 0, {0} },    "OPR_NOT", 7 },     /* "NOT" operation for putRect() */

    { { {0}, 0, 0, {0} }, NULL, 0 }	/* must be final in this array */
};

const sys_func_t graphics_func_table[] = {
    { sf_drawLine,      "drawLine",     8, "v,i,i,i,i,i", NULL },
    { sf_drawFrame,     "drawFrame",    9, "v,i,i,i,i,i", NULL },
    { sf_drawRect,      "drawRect",     8, "v,i,i,i,i,i", NULL },
    { sf_drawTriangle,  "drawTriangle", 12, "v,i,i,i,i,i,i,i", NULL },
    { sf_drawCircle,    "drawCircle",   10, "v,i,i,i,i", NULL },
    { sf_drawEllipse,   "drawEllipse",  11, "v,i,i,i,i,d,i", NULL },
    { sf_drawSector,    "drawSector",   10, "v,i,i,i,i,d,d,d,i", NULL },
    { sf_drawShape,     "drawShape",    9, "v,i,i,d,c*", NULL },
    { sf_drawChar,      "drawChar",     8, "v,i", NULL },
    { sf_floodFill,     "floodFill",    9, "v,i,i,i", NULL },
    { sf_getRect,       "getRect",      7, "v,v*,i,i,i,i", NULL },
    { sf_putRect,       "putRect",      7, "v,v*,i,i,i", NULL },
    { sf_posX,          "posX",         4, ":void posX(int) _or_ int posX(void)", NULL },
    { sf_posY,          "posY",         4, ":void posY(int) _or_ int posY(void)", NULL },
    { sf_fontScale,     "fontScale",    9, ":void fontScale(int) _or_ int fontScale(void)", NULL },
    { sf_fontFcol,      "fontFcol",     8, ":void fontFcol(int) _or_ int fontFcol(void)", NULL },
    { sf_fontBcol,      "fontBcol",     8, ":void fontBcol(int) _or_ int fontBcol(void)", NULL },
    { sf_fontSet,       "fontSet",      7, ":void fontSet(font_t *)", NULL },
    { sf_lockScroll,    "lockScroll",   10, "v,v", NULL },
    { sf_unlockScroll,  "unlockScroll", 12, "v,v", NULL },
    {NULL, NULL, 0, NULL, NULL}
};

data_t d5, d6, d7, d8;  /* extra parameter accumulators needed for some functions */


/* initialisation code */
void graph_init(void) {
    var_t *v = vars;
	while(v && (SYS_FONT_NAME_LEN != v->nlen || strncmp(SYS_FONT_NAME, v->name, v->nlen))) v = v->next;
    if(v) v->data.val.i = (uintptr_t) &sysFont0508;
}


void sf_drawLine(void) {
    get_param(&d1, DT_INT, 0);  /* x1 */
    get_comma();
    get_param(&d2, DT_INT, 0);  /* y1 */
    get_comma();
    get_param(&d3, DT_INT, 0);  /* x2 */
    get_comma();
    get_param(&d4, DT_INT, 0);  /* y2 */
    get_comma();
    get_param(&d5, DT_INT, 0);  /* colour */
    drawLine(d1.val.i, d2.val.i, d3.val.i, d4.val.i, d5.val.i);
}


void sf_drawFrame(void) {
    get_param(&d1, DT_INT, 0);  /* x1 */
    get_comma();
    get_param(&d2, DT_INT, 0);  /* y1 */
    get_comma();
    get_param(&d3, DT_INT, 0);  /* x2 */
    get_comma();
    get_param(&d4, DT_INT, 0);  /* y2 */
    get_comma();
    get_param(&d5, DT_INT, 0);  /* colour */
    drawFrame(d1.val.i, d2.val.i, d3.val.i, d4.val.i, d5.val.i);
}


void sf_drawRect(void) {
    get_param(&d1, DT_INT, 0);  /* x1 */
    get_comma();
    get_param(&d2, DT_INT, 0);  /* y1 */
    get_comma();
    get_param(&d3, DT_INT, 0);  /* x2 */
    get_comma();
    get_param(&d4, DT_INT, 0);  /* y2 */
    get_comma();
    get_param(&d5, DT_INT, 0);  /* colour */
    drawRect(d1.val.i, d2.val.i, d3.val.i, d4.val.i, d5.val.i);
}


void sf_getRect(void) {
    get_param(&d1, DT_VOID, 1); /* buffer */
    get_comma();
    get_param(&d2, DT_INT, 0);  /* x1 */
    get_comma();
    get_param(&d3, DT_INT, 0);  /* y1 */
    get_comma();
    get_param(&d4, DT_INT, 0);  /* x2 */
    get_comma();
    get_param(&d5, DT_INT, 0);  /* y2 */
    getRect((void *) (uintptr_t) d1.val.i, d2.val.i, d3.val.i, d4.val.i, d5.val.i);
}


void sf_putRect(void) {
    get_param(&d1, DT_VOID, 1); /* buffer */
    get_comma();
    get_param(&d2, DT_INT, 0);  /* x */
    get_comma();
    get_param(&d3, DT_INT, 0);  /* y */
    get_comma();
    get_param(&d4, DT_INT, 0);  /* func */
    putRect((void *) (uintptr_t) d1.val.i, d2.val.i, d3.val.i, d4.val.i);
}


void sf_drawTriangle(void) {
    get_param(&d1, DT_INT, 0);  /* x1 */
    get_comma();
    get_param(&d2, DT_INT, 0);  /* y1 */
    get_comma();
    get_param(&d3, DT_INT, 0);  /* x2 */
    get_comma();
    get_param(&d4, DT_INT, 0);  /* y2 */
    get_comma();
    get_param(&d5, DT_INT, 0);  /* x3 */
    get_comma();
    get_param(&d6, DT_INT, 0);  /* y3 */
    get_comma();
    get_param(&d7, DT_INT, 0);  /* colour */
    drawTriangle(d1.val.i, d2.val.i, d3.val.i, d4.val.i, d5.val.i, d6.val.i, d7.val.i);
}


void sf_drawCircle(void) {
    get_param(&d1, DT_INT, 0);  /* x */
    get_comma();
    get_param(&d2, DT_INT, 0);  /* y */
    get_comma();
    get_param(&d3, DT_INT, 0);  /* r */
    get_comma();
    get_param(&d4, DT_INT, 0);  /* colour */
    drawCircle(d1.val.i, d2.val.i, d3.val.i, d4.val.i);
}


void sf_drawEllipse(void) {
    get_param(&d1, DT_INT, 0);  /* x */
    get_comma();
    get_param(&d2, DT_INT, 0);  /* y */
    get_comma();
    get_param(&d3, DT_INT, 0);  /* rx */
    get_comma();
    get_param(&d4, DT_INT, 0);  /* ry */
    get_comma();
    get_param(&d5, DT_DOUBLE, 0);   /* tilt angle */
    get_comma();
    get_param(&d6, DT_INT, 0);  /* colour */
    drawEllipse(d1.val.i, d2.val.i, d3.val.i, d4.val.i, d5.val.f, d6.val.i);
}


void sf_drawSector(void) {
    get_param(&d1, DT_INT, 0);  /* x */
    get_comma();
    get_param(&d2, DT_INT, 0);  /* y */
    get_comma();
    get_param(&d3, DT_INT, 0);  /* rx */
    get_comma();
    get_param(&d4, DT_INT, 0);  /* ry */
    get_comma();
    get_param(&d5, DT_DOUBLE, 0);   /* tilt angle */
    get_comma();
    get_param(&d6, DT_DOUBLE, 0);   /* start angle */
    get_comma();
    get_param(&d7, DT_DOUBLE, 0);   /* end angle */
    get_comma();
    get_param(&d8, DT_INT, 0);  /* colour */
    drawSector(d1.val.i, d2.val.i, d3.val.i, d4.val.i, d5.val.f, d6.val.f, d7.val.f, d8.val.i);
}


void sf_drawShape(void) {
    get_param(&d1, DT_INT, 0);  /* x */
    get_comma();
    get_param(&d2, DT_INT, 0);  /* y */
    get_comma();
    get_param(&d3, DT_DOUBLE, 0);   /* tilt angle */
    get_comma();
    get_param(&d4, DT_INT, 0);  /* colour */
    drawShape(d1.val.i, d2.val.i, d3.val.f, (char *) (uintptr_t) d4.val.i);
}


void sf_drawChar(void) {
    uint16_t flags_temp = enable_flags & (FLAG_NO_CTRL | FLAG_NO_SCROLL);
    enable_flags |= (FLAG_NO_CTRL | FLAG_NO_SCROLL);
    get_param(&d1, DT_INT, 0);  /* ch */
    drawChar(d1.val.i);
    enable_flags &= ~(FLAG_NO_CTRL | FLAG_NO_SCROLL);
    enable_flags |= flags_temp;
}


void sf_lockScroll(void) {
    enable_flags |= FLAG_NO_SCROLL;
}


void sf_unlockScroll(void) {
    enable_flags &= ~FLAG_NO_SCROLL;
}


void sf_floodFill(void) {
    get_param(&d1, DT_INT, 0);  /* x */
    get_comma();
    get_param(&d2, DT_INT, 0);  /* y */
    get_comma();
    get_param(&d3, DT_INT, 0);  /* colour */
    floodFill(d1.val.i, d2.val.i, d3.val.i);   
}


void sf_posX(void) {
    skip_spaces(0);
    if(*prog != ')') {  /* set */
        get_param(&d1, DT_INT, 0);
        posX = d1.val.i;
    }
    else {  /* get */
        ival(accN) = posX;
        acc[accN].ind = 0;
        acc[accN].type = DT_INT;
    }
}


void sf_posY(void) {
    skip_spaces(0);
    if(*prog != ')') {  /* set */
        get_param(&d1, DT_INT, 0);
        posY = d1.val.i;
    }
    else {  /* get */
        ival(accN) = posY;
        acc[accN].ind = 0;
        acc[accN].type = DT_INT;
    }
}


void sf_fontScale(void) {
    skip_spaces(0);
    if(*prog != ')') {  /* set */
        get_param(&d1, DT_INT, 0);  /* factor */
        fontScale = d1.val.i;
    }
    else {  /* get */
        ival(accN) = fontScale;
        acc[accN].ind = 0;
        acc[accN].type = DT_INT;
    }
}


void sf_fontFcol(void) {
    skip_spaces(0);
    if(*prog != ')') {  /* set */
        get_param(&d1, DT_INT, 0);  /* colour */
        fontFcol = d1.val.i;
    }
    else {  /* get */
        ival(accN) = fontFcol;
        acc[accN].ind = 0;
        acc[accN].type = DT_INT;
    }
}


void sf_fontBcol(void) {
    skip_spaces(0);
    if(*prog != ')') {  /* set */
        get_param(&d1, DT_INT, 0);  /* colour */
        fontBcol = d1.val.i;
    }
    else {  /* get */
        ival(accN) = fontBcol;
        acc[accN].ind = 0;
        acc[accN].type = DT_INT;
    }
}


void sf_fontSet(void) {
    get_param(&d1, DT_VOID, 1); /* (*font) */
    font = (font_t *) (uintptr_t) d1.val.i;
}

#endif
