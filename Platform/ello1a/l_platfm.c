#ifdef CIMPL

#include <string.h>
#include <xc.h>
#include "l_platfm.h"
#include "platform.h"
#include "../../ride/graphics.h"
#include "../../ride/kbdcodes.h"
#include "../../ride/ride.h"

const sys_const_t platform_const_table[] = {

    { { { .i = (uintptr_t) BASEA }, (FT_UNSIGNED | DT_INT), 1, {0} },	"BASEA", 5 },
    { { { .i = (uintptr_t) BASEB }, (FT_UNSIGNED | DT_INT), 1, {0} },	"BASEB", 5 },
    { { { .i = (uintptr_t) BASEC }, (FT_UNSIGNED | DT_INT), 1, {0} },	"BASEC", 5 },
    { { { .i = (uintptr_t) BASED }, (FT_UNSIGNED | DT_INT), 1, {0} },	"BASED", 5 },
    { { { .i = (uintptr_t) BASEE }, (FT_UNSIGNED | DT_INT), 1, {0} },	"BASEE", 5 },
    { { { .i = (uintptr_t) BASEF }, (FT_UNSIGNED | DT_INT), 1, {0} },	"BASEF", 5 },
    { { { .i = (uintptr_t) BASEG }, (FT_UNSIGNED | DT_INT), 1, {0} },	"BASEG", 5 },

    { { { .i = ANSEL },     (FT_UNSIGNED | DT_INT), 0, {0} },	"ANSEL",     5 },
    { { { .i = ANSELCLR },  (FT_UNSIGNED | DT_INT), 0, {0} },	"ANSELCLR",  8 },
    { { { .i = ANSELSET },  (FT_UNSIGNED | DT_INT), 0, {0} },	"ANSELSET",  8 },
    { { { .i = ANSELINV },  (FT_UNSIGNED | DT_INT), 0, {0} },	"ANSELINV",  8 },
    { { { .i = TRIS },      (FT_UNSIGNED | DT_INT), 0, {0} },	"TRIS",      4 },
    { { { .i = TRISCLR },   (FT_UNSIGNED | DT_INT), 0, {0} },	"TRISCLR",   7 },
    { { { .i = TRISSET },   (FT_UNSIGNED | DT_INT), 0, {0} },	"TRISSET",   7 },
    { { { .i = TRISINV },   (FT_UNSIGNED | DT_INT), 0, {0} },	"TRISINV",   7 },
    { { { .i = PORT },      (FT_UNSIGNED | DT_INT), 0, {0} },	"PORT",      4 },
    { { { .i = PORTCLR },   (FT_UNSIGNED | DT_INT), 0, {0} },	"PORTCLR",   7 },
    { { { .i = PORTSET },   (FT_UNSIGNED | DT_INT), 0, {0} },	"PORTSET",   7 },
    { { { .i = PORTINV },   (FT_UNSIGNED | DT_INT), 0, {0} },	"PORTINV",   7 },
    { { { .i = LAT },       (FT_UNSIGNED | DT_INT), 0, {0} },	"LAT",       3 },
    { { { .i = LATCLR },    (FT_UNSIGNED | DT_INT), 0, {0} },	"LATCLR",    6 },
    { { { .i = LATSET },    (FT_UNSIGNED | DT_INT), 0, {0} },	"LATSET",    6 },
    { { { .i = LATINV },    (FT_UNSIGNED | DT_INT), 0, {0} },	"LATINV",    6 },
    { { { .i = ODC },       (FT_UNSIGNED | DT_INT), 0, {0} },	"ODC",       3 },
    { { { .i = ODCCLR },    (FT_UNSIGNED | DT_INT), 0, {0} },	"ODCCLR",    6 },
    { { { .i = ODCSET },    (FT_UNSIGNED | DT_INT), 0, {0} },	"ODCSET",    6 },
    { { { .i = ODCINV },    (FT_UNSIGNED | DT_INT), 0, {0} },	"ODCINV",    6 },
    { { { .i = CNPU },      (FT_UNSIGNED | DT_INT), 0, {0} },	"CNPU",      4 },
    { { { .i = CNPUCLR },   (FT_UNSIGNED | DT_INT), 0, {0} },	"CNPUCLR",   7 },
    { { { .i = CNPUSET },   (FT_UNSIGNED | DT_INT), 0, {0} },	"CNPUSET",   7 },
    { { { .i = CNPUINV },   (FT_UNSIGNED | DT_INT), 0, {0} },	"CNPUINV",   7 },
    { { { .i = CNPD },      (FT_UNSIGNED | DT_INT), 0, {0} },	"CNPD",      4 },
    { { { .i = CNPDCLR },   (FT_UNSIGNED | DT_INT), 0, {0} },	"CNPDCLR",   7 },
    { { { .i = CNPDSET },   (FT_UNSIGNED | DT_INT), 0, {0} },	"CNPDSET",   7 },
    { { { .i = CNPDINV },   (FT_UNSIGNED | DT_INT), 0, {0} },	"CNPDINV",   7 },
    { { { .i = CNCON },     (FT_UNSIGNED | DT_INT), 0, {0} },	"CNCON",     5 },
    { { { .i = CNCONCLR },  (FT_UNSIGNED | DT_INT), 0, {0} },	"CNCONCLR",  8 },
    { { { .i = CNCONSET },  (FT_UNSIGNED | DT_INT), 0, {0} },	"CNCONSET",  8 },
    { { { .i = CNCONINV },  (FT_UNSIGNED | DT_INT), 0, {0} },	"CNCONINV",  8 },
    { { { .i = CNEN },      (FT_UNSIGNED | DT_INT), 0, {0} },	"CNEN",      4 },
    { { { .i = CNENCLR },   (FT_UNSIGNED | DT_INT), 0, {0} },	"CNENCLR",   7 },
    { { { .i = CNENSET },   (FT_UNSIGNED | DT_INT), 0, {0} },	"CNENSET",   7 },
    { { { .i = CNENINV },   (FT_UNSIGNED | DT_INT), 0, {0} },	"CNENINV",   7 },
    { { { .i = CNSTAT },    (FT_UNSIGNED | DT_INT), 0, {0} },	"CNSTAT",    6 },
    { { { .i = CNSTATCLR }, (FT_UNSIGNED | DT_INT), 0, {0} },   "CNSTATCLR", 9 },
    { { { .i = CNSTATSET }, (FT_UNSIGNED | DT_INT), 0, {0} },   "CNSTATSET", 9 },
    { { { .i = CNSTATINV }, (FT_UNSIGNED | DT_INT), 0, {0} },   "CNSTATINV", 9 },

    { { {0}, 0, 0, {0} }, NULL, 0 }	/* must be final in this array */
};

const sys_func_t platform_func_table[] = {

	/* system */
    { sf_reset,         "reset",        5,  "v,v", NULL },
    { sf_delay_ms,      "delay_ms",     8,  "v,ul", NULL },
    { sf_set_timer,     "set_timer",    9,  ":void set_timer(unsigned long, void (*f)(void))", NULL },

    /* communications */
    { sf_spiOpen,       "spiOpen",      7,  "i,i,i,i,i", NULL },
    { sf_spiClose,      "spiClose",     8,  "i", NULL },
    { sf_spiByte,       "spiByte",      7,  "uc,i,uc", NULL },
    { sf_spiBlock,      "spiBlock",     8,  "v,i,uc*,z", NULL },
    { sf_i2cInit,       "i2cInit",      7,  "v,i", NULL },
    { sf_i2cStart,      "i2cStart",     8,  "v,v", NULL },
    { sf_i2cRepStart,   "i2cRepStart",  11, "v,v", NULL },
    { sf_i2cStop,       "i2cStop",      7,  "v,v", NULL },
    { sf_i2cSend,       "i2cSend",      7,  "i,uc", NULL },
    { sf_i2cRecv,       "i2cRecv",      7,  "uc,i", NULL },
	{ sf_comOpen,       "comOpen",      7,  "i,i,i,i,i,i", NULL },
    { sf_comClose,      "comClose",     8,  "i", NULL },
    { sf_comPeek,       "comPeek",      7,  "i,i", NULL },
    { sf_comBuff,       "comBuff",      7,  "i,i", NULL },
    { sf_comTx,         "comTx",        5,  "v,i,i,uc*", NULL },
    { sf_comRx,         "comRx",        5,  "i,i,i,uc*", NULL },
    { sf_comRxCall,     "comRxCall",    9,  ":void comRxCall(int, void (*f)(void))", NULL },

    /* keyboard */
    { sf_setKbdLayout,  "setKbdLayout", 12, "v,c", NULL },
    { sf_getKbdLayout,  "getKbdLayout", 12, "c,v", NULL },
    { sf_setBrkCode,    "setBrkCode",   10, "v,c", NULL },
    { sf_getBrkCode,    "getBrkCode",   10, "c,v", NULL },

    /* sound */
    { sf_beep,          "beep",         4,  "v,v", NULL },
    { sf_sound,         "sound",        5,  "v,i,i", NULL },

    /* video */
    { sf_initVideo,     "initVideo",    9,  "v,i", NULL },
    { sf_getVmode,      "getVmode",     8,  "i,v", NULL },
    { sf_Hres,          "Hres",         4,  "i,v", NULL },
    { sf_Vres,          "Vres",         4,  "i,v", NULL },
    { sf_clearScreen,   "clearScreen",  11, "v,i", NULL },
    { sf_setPixel,      "setPixel",     8,  "v,i,i,i", NULL },
    { sf_getPixel,      "getPixel",     8,  "i,i,i", NULL },

    {NULL, NULL, 0, NULL, NULL}
};

data_t d5;	/* common data accumulators used in the library functions (additional to the already defined d1..d4) */
func_t *comRxHandler[COM_PORTS] = { NULL };
int comRxBytes[COM_PORTS] = { 0 };
callback_t cb = { NULL, NULL };


void pltfm_init(void) {
	if(cb.call == NULL) {
		cb.call = pltfm_call;
		cb.next = callbacks;
		callbacks = &cb;        /* add to the execution chain */
	}
}


void pltfm_call(void) {
	// nothing needed for now
}


void sf_reset(void) {
    SoftReset();
}


void sf_delay_ms(void) {
    get_param(&d1, (FT_UNSIGNED | DT_LONG), 0);
    unsigned long ce = clock() + d1.val.i;
    while((unsigned long) clock() > ce) wait_break();	/* to cover for a (rare) case of counter roll-over */
    while((unsigned long) clock() < ce) wait_break();
}


void sf_set_timer(void) {
    get_param(&d1, (FT_UNSIGNED | DT_LONG), 0); /* milliseconds */
    get_comma();
    func_t *f = get_token();
	if(token != NUMBER) {
		if(token != FUNCTION || !f) error(SYNTAX);
		skip_spaces(0);
		if(*prog != ')') error(CL_PAREN_EXPECTED);
		prog++;
	}
	else f = (func_t *) (uintptr_t) acc[accN].val.i;
    unsigned char t;
    for(t = 0; t < MAX_TIMERS && timers[t].reload; t++);
    if(t >= MAX_TIMERS) error(INSUFFICIENT_RESOURCE);
    timers[t].handler = f;
    timers[t].counter = timers[t].reload = d1.val.i;
}


void sf_spiOpen(void) {
    get_param(&d1, DT_INT, 0);  /* channel */
    get_comma();
    get_param(&d2, DT_INT, 0);  /* mode */
    get_comma();
    get_param(&d3, DT_INT, 0);  /* baudrate */
	d1.val.i--; // internal port numbering starts from 0
    acc[accN].val.i = openSPI(d1.val.i, d2.val.i, 8, d3.val.i);
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
}


void sf_spiClose(void) {
    get_param(&d1, DT_INT, 0);  /* channel */
	d1.val.i--; // internal port numbering starts from 0
    acc[accN].val.i = openSPI(d1.val.i, 0, 0, 0);
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
}


void sf_spiByte(void) {
    get_param(&d1, DT_INT, 0);  /* channel */
    get_comma();
    get_param(&d2, (FT_UNSIGNED | DT_LONG), 0); /* data to send */
	d1.val.i--; // internal port numbering starts from 0
    acc[accN].val.i = xchgSPI(d1.val.i, d2.val.i);
    acc[accN].ind = 0;
    acc[accN].type = (FT_UNSIGNED | DT_LONG);
}


void sf_spiBlock(void) {
    get_param(&d1, DT_INT, 0);  /* channel */
    get_comma();
    get_param(&d2, (FT_UNSIGNED | DT_CHAR), 1); /* buffer */
    get_comma();
    get_param(&d3, DT_SIZE_T, 0);   /* len */
	d1.val.i--; // internal port numbering starts from 0
    unsigned char *p = (unsigned char *) (uintptr_t) d2.val.i;
    for( ; d3.val.i > 0; d3.val.i--, p++) *p = xchgSPI(d1.val.i, *p);
}


void sf_comOpen(void) {
    get_param(&d1, DT_INT, 0);  /* channel */
    get_comma();
    get_param(&d2, DT_INT, 0);  /* mode */
    get_comma();
    get_param(&d3, DT_INT, 0);  /* protocol */
    get_comma();
    get_param(&d4, DT_INT, 0);  /* baudrate */
    get_comma();
    get_param(&d5, DT_INT, 0);  /* Rx buffer size */
    d1.val.i--; // internal port numbering starts from 0
    if(d1.val.i >= UART1 && d1.val.i < COM_PORTS) {
        comRxHandler[(unsigned int) d1.val.i] = NULL;
        comRxBytes[(unsigned int) d1.val.i] = 0;
    }
    acc[accN].val.i = openCOM(d1.val.i, d5.val.i, d4.val.i, d2.val.i, d3.val.i);
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
}


void sf_comClose(void) {
    get_param(&d1, DT_INT, 0);  /* channel */
    d1.val.i--; // internal port numbering starts from 0
    if(d1.val.i >= 0 && d1.val.i < COM_PORTS) {
        comRxHandler[(unsigned int) d1.val.i] = NULL;
        comRxBytes[(unsigned int) d1.val.i] = 0;
    }
    acc[accN].val.i = openCOM(d1.val.i, 0, 0, 0, 0);
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
}


void sf_comPeek(void) {
    get_param(&d1, DT_INT, 0);  /* channel */
    d1.val.i--; // internal port numbering starts from 0
    acc[accN].val.i = UART_peek(d1.val.i);
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
}


void sf_comBuff(void) {
    get_param(&d1, DT_INT, 0);  /* channel */
    d1.val.i--; // internal port numbering starts from 0
    acc[accN].val.i = UART_buffer(d1.val.i);
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
}


void sf_comTx(void) {
    get_param(&d1, DT_INT, 0);  /* channel */
    get_comma();
    get_param(&d2, DT_INT, 0);  /* bytes */
    get_comma();
    get_param(&d3, (FT_UNSIGNED | DT_CHAR), 0); /* buffer */
    d1.val.i--; // internal port numbering starts from 0
    if(d1.val.i >= 0 && d1.val.i < COM_PORTS) {
        while(d2.val.i-- > 0) {
            UART_tx(d1.val.i, *((unsigned char *) (uintptr_t) d3.val.i));
            d3.val.i++;
        }
    }
}


void sf_comRx(void) {
    get_param(&d1, DT_INT, 0);  /* channel */
    get_comma();
    get_param(&d2, DT_INT, 0);  /* bytes */
    get_comma();
    get_param(&d3, (FT_UNSIGNED | DT_CHAR), 0); /* buffer */
    unsigned long c = 0;
    d1.val.i--; // internal port numbering starts from 0
    if(d1.val.i >= 0 && d1.val.i < COM_PORTS) {
        while(d2.val.i-- > 0 && UART_peek(d1.val.i) >= 0) {
            *((unsigned char *) (uintptr_t) d3.val.i++) = UART_rx(d1.val.i);
            c++;
        }
    }
    acc[accN].val.i = c;
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
}


void sf_comRxCall(void) {
    get_param(&d1, DT_INT, 0);  /* channel */
    get_comma();
    get_param(&d2, DT_INT, 0);  /* number of bytes */
    get_comma();
    func_t *f = get_token();
    if(token != NUMBER) {
        if(token != FUNCTION || !f) error(SYNTAX);
        skip_spaces(0);
        if(*prog != ')') error(CL_PAREN_EXPECTED);
        prog++;
    }
    else f = (func_t *) (uintptr_t) acc[accN].val.i;
    d1.val.i--; // internal port numbering starts from 0
    if(d1.val.i >= 0 && d1.val.i < COM_PORTS) {
        comRxHandler[(unsigned int) d1.val.i] = f;
        comRxBytes[(unsigned int) d1.val.i] = d2.val.i;
    }
}


void sf_i2cInit(void) {
    get_param(&d1, DT_INT, 0);  /* baud rate */
    acc[accN].val.i = i2cInit(d1.val.i);
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
}


void sf_i2cStart(void) {
    i2cStart();
}


void sf_i2cRepStart(void) {
    i2cRepStart();
}


void sf_i2cStop(void) {
    i2cStop();
}


void sf_i2cSend(void) {
    get_param(&d1, (FT_UNSIGNED | DT_CHAR), 0); /* data8 */
    acc[accN].val.i = i2cSend(d1.val.i);
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
}


void sf_i2cRecv(void) {
    get_param(&d1, DT_INT, 0);                  /* ack */
    acc[accN].val.i = i2cRecv(d1.val.i);
    acc[accN].ind = 0;
    acc[accN].type = (FT_UNSIGNED | DT_CHAR);
}


void sf_initVideo(void) {
    get_param(&d1, DT_INT, 0);
    initVideo(d1.val.i);
}


void sf_clearScreen(void) {
    get_param(&d1, DT_INT, 0);
    int c = (int) d1.val.i;
    if(c == COL_NONE || c == COL_SOLID || c == 1 || c == COL_INVERT || c == COL_TRANSP) clearScreen(c);
    else drawRect(0, 0, Hres - 1, Vres - 1, c);
}


void sf_setPixel(void) {
    get_param(&d1, DT_INT, 0);  /* x */
    get_comma();
    get_param(&d2, DT_INT, 0);  /* y */
    get_comma();
    get_param(&d3, DT_INT, 0);  /* colour */
    setPixel(d1.val.i, d2.val.i, d3.val.i);
}


void sf_getPixel(void) {
    get_param(&d1, DT_INT, 0);  /* x */
    get_comma();
    get_param(&d2, DT_INT, 0);  /* y */
    ival(accN) = getPixel(d1.val.i, d2.val.i);
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
}


void sf_setKbdLayout(void) {
    get_param(&d1, DT_CHAR, 0); /* country code */
    settings.kbd_layout = d1.val.i;
}


void sf_getKbdLayout(void) {
    ival(accN) = settings.kbd_layout;
    acc[accN].ind = 0;
    acc[accN].type = DT_CHAR;
}


void sf_setBrkCode(void) {
    get_param(&d1, DT_CHAR, 0); /* break code */
    settings.brk_code = d1.val.i;
}


void sf_getBrkCode(void) {
    ival(accN) = settings.brk_code;
    acc[accN].ind = 0;
    acc[accN].type = DT_CHAR;
}


void sf_beep(void) {
    beep();
}


void sf_sound(void) {
    get_param(&d1, DT_INT, 0);  /* frequency in Hz */
    get_comma();
    get_param(&d2, DT_INT, 0);  /* volume (0..1000) */
    sound(d1.val.i, d2.val.i);
}


void sf_Hres(void) {
    ival(accN) = Hres;
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
}


void sf_Vres(void) {
    ival(accN) = Vres;
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
}


void sf_getVmode(void) {
    ival(accN) = Vmode;
    acc[accN].ind = 0;
    acc[accN].type = DT_INT;
}

#endif
