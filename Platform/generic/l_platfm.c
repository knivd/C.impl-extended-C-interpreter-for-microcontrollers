#include <string.h>
#include "l_platfm.h"
#include "platform.h"

const sys_const_t platform_const_table[] = {

    { { {0}, 0, 0, {0} }, NULL, 0 }	/* must be final in this array */
};


const sys_func_t platform_func_table[] = {

	/* system */
    { sf_delay_ms,      "delay_ms",     8,  "v,ul", NULL },
    { sf_set_timer,     "set_timer",    9,  "v,ul", NULL },
   
    {NULL, NULL, 0, NULL, NULL}
};


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
	if(token != FUNCTION || !f) error(SYNTAX);
    skip_spaces(0);
    if(*prog != ')') error(CL_PAREN_EXPECTED);
    prog++;
    unsigned char t;
    for(t = 0; t < MAX_TIMERS && timers[t].reload && timers[t].handler != f; t++);
    if(t >= MAX_TIMERS) error(INSUFFICIENT_RESOURCE);
    timers[t].handler = (d1.val.i ? f : NULL);
    timers[t].counter = timers[t].reload = d1.val.i;
}
