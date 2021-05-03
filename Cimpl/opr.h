#ifdef CIMPL
#ifndef OPR_H
#define OPR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cimpl.h"

/* operators receive the first parameter in token.data and return the result back there */

uint8_t ptr_depth;
label_t goto_point;

#define LOOP_NONE	0
#define LOOP_DO		1
#define LOOP_WHILE	2
#define LOOP_FOR	3
#define LOOP_SWITCH	4

#define BRK_NONE	0
#define BRK_BREAK	1
#define BRK_CONT	2

uint8_t loop_type, brkcont_flag;
char *do_stack[MAX_NESTED];
uint8_t do_stack_ix;
char *while_stack[MAX_NESTED];
uint8_t while_stack_ix;
struct {
	char *cond;	/* condition block */
	char *mod;	/* modification block */
	char *exec;	/* execution block */
} for_stack[MAX_NESTED];
uint8_t for_stack_ix;
struct {
	char *exec;	/* main execution block */
	char *dflt;	/* 'default' block */
} switch_stack[MAX_NESTED];
uint8_t switch_stack_ix;
var_t *vprec;	/* variable preceding the current operator */
var_t *vpost;	/* variable following the current operator */

void promote(data_t *D1, data_t *D2);
void skip_till_cl_paren(void);

void op_increment_pre(void);
void op_increment_post(void);
void op_decrement_pre(void);
void op_decrement_post(void);
void op_pointer(void);
void op_address(void);
void op_sizeof(void);
void op_cast(void);
void op_unary_plus(void);
void op_unary_minus(void);
void op_logical_not(void);
void op_bitwise_not(void);
void op_plus(void);
void op_minus(void);
void op_multiplication(void);
void op_division(void);
void op_modulo(void);
void op_bitwise_and(void);
void op_bitwise_xor(void);
void op_bitwise_or(void);
void op_bitwise_shiftl(void);
void op_bitwise_shiftr(void);
void op_logical_and(void);
void op_logical_or(void);
void op_equal(void);
void op_not_equal(void);
void op_smaller(void);
void op_smaller_or_equal(void);
void op_greater(void);
void op_greater_or_equal(void);
void op_attribution(void);
void op_plus_attr(void);
void op_minus_attr(void);
void op_multiplication_attr(void);
void op_division_attr(void);
void op_modulo_attr(void);
void op_bitwise_or_attr(void);
void op_bitwise_xor_attr(void);
void op_bitwise_and_attr(void);
void op_bitwise_shiftr_attr(void);
void op_bitwise_shiftl_attr(void);
void op_bitwise_not_attr(void);
void op_question(void);
void op_ellipsis(void);
void op_dot(void);
void op_arrow(void);

void kw_if(void);
void kw_else(void);
void kw_for(void);
void kw_do(void);
void kw_while(void);
void kw_continue(void);
void kw_break(void);
void kw_switch(void);
void kw_case(void);
void kw_default(void);
void kw_return(void);
void kw_goto(void);
void kw_typedef(void);

#ifdef __cplusplus
}
#endif

#endif  /* OPR_H */
#endif
