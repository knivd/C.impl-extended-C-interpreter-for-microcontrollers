#ifdef CIMPL

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "opr.h"
#include "cimpl.h"
#include "../xmem.h"


/* eqialise the data types of D1 and D2 by promoting one to the other, if necessary */
/* D1 is the passed result */
/* both input parameters must be of numeric type INT or FPN */
void promote(data_t *D1, data_t *D2) {
    if(!D1->ind && D2->ind) error(NOT_ALLOWED_WITH_PTR);        	/* can't promote to pointer */
    if(D1->ind && isFPN_t(D2->type)) error(NOT_ALLOWED_WITH_PTR);	/* can't promote pointer to FPN */
	if((D1->type & DT_MASK) > (D2->type & DT_MASK)) convert(D2, D1);
	else if((D2->type & DT_MASK) > (D1->type & DT_MASK)) convert(D1, D2);
	else {
		if((D1->type & FT_MASK) > (D2->type & FT_MASK)) convert(D2, D1);
		else if((D2->type & FT_MASK) > (D1->type & FT_MASK)) convert(D1, D2);
	}
}


/* unary operator */
void op_increment_pre(void) {
	if(isINT(accN)) ++ival(accN);
	else if(isFPN(accN)) ++fval(accN);
	else error(INVALID_DATA_TYPE);
	if(vpost) var_set(vpost, &vpost->data, 0);
	accN--;
}


/* unary operator */
void op_increment_post(void) {
	if(post_ix >= MAX_POST) error(TOO_MANY_POSTMODS);
	accN--;
	memcpy(&post[post_ix].d, &acc[accN], sizeof(data_t));
	post[post_ix].v = (vprec ? vprec : NULL);
	post[post_ix].opr = 1 * (indexed + 1);
	post_ix++;
}


/* unary operator */
void op_decrement_pre(void) {
	if(isINT(accN)) --ival(accN);
	else if(isFPN(accN)) --fval(accN);
	else error(INVALID_DATA_TYPE);
	if(vpost) var_set(vpost, &vpost->data, 0);
	accN--;
}


/* unary operator */
void op_decrement_post(void) {
	if(post_ix >= MAX_POST) error(TOO_MANY_POSTMODS);
	accN--;
	memcpy(&post[post_ix].d, &acc[accN], sizeof(data_t));
	post[post_ix].v = (vprec ? vprec : NULL);
	post[post_ix].opr = -1 * (indexed + 1);
	post_ix++;
}


/* unary operator */
void op_pointer(void) {
	accN--;
	acc[accN].type = acc[accN + 1].type;
	acc[accN].ind = acc[accN + 1].ind;
	if(++acc[accN].ind >= MAX_NESTED) error(MAXIMUM_NESTING);
	if(vpost) var_get(vpost);
	else {
		byte *p = (byte *) (uintptr_t) acc[accN].val.i;
		while(acc[accN].ind) p = *((byte **) p);
		switch(acc[accN].type & DT_MASK) {
			case DT_BOOL:
				acc[accN].val.i = *((uint8_t *) p);
				break;
			case DT_CHAR:
				if(acc[accN].type & FT_UNSIGNED) acc[accN].val.i = *((uint8_t *) p);
				else acc[accN].val.i = *((int8_t *) p);
				break;
			case DT_SHORT:
				if(acc[accN].type & FT_UNSIGNED) acc[accN].val.i = *((uint16_t *) p);
				else acc[accN].val.i = *((int16_t *) p);
				break;
			case DT_INT:
				if(acc[accN].type & FT_UNSIGNED) acc[accN].val.i = *((uint_t *) p);
				else acc[accN].val.i = *((int_t *) p);
				break;
			case DT_LONG:
				if(acc[accN].type & FT_UNSIGNED) acc[accN].val.i = *((uint32_t *) p);
				else acc[accN].val.i = *((int32_t *) p);
				break;
			case DT_LLONG:
				if(acc[accN].type & FT_UNSIGNED) acc[accN].val.i = *((uint64_t *) p);
				else acc[accN].val.i = *((int64_t *) p);
				break;
			case DT_FLOAT:
				acc[accN].val.f = *((float *) p);
				break;
			case DT_DOUBLE:
				acc[accN].val.f = *((double *) p);
				break;
			case DT_LDOUBLE:
				acc[accN].val.f = *((ldouble_t *) p);
				break;
			case DT_ENUM:
				acc[accN].val.i = *((int_t *) p);
				break;
			case DT_VA_LIST:
			case DT_FILE:
				acc[accN].val.i = (uintptr_t) *((byte **) p);
				break;
			case DT_STRUCT:
			case DT_UNION:
				acc[accN].val.i = (uintptr_t) p;
				break;
			default:
				error(SYNTAX);
		}
	}
}


/* unary operator */
void op_address(void) {
	if(!vpost) error(SYNTAX);
	uint8_t dims;
	uint8_t *p = calcAddr(vpost, &acc[accN], &dims);
	accN--;
	acc[accN].type = FT_UNSIGNED | DT_INT;
	acc[accN].ind = 0;
	ival(accN) = (uintptr_t) p;
}


/* unary operator */
void op_sizeof(void) {
	if(!vpost) {
		if(acc[accN].ind) ival(accN) = dt_size[DT_VOID];
		else if((((acc[accN].type & DT_MASK) > DT_VOID) && ((acc[accN].type & DT_MASK) <= DT_LDOUBLE)) ||
					isType(accN, DT_ENUM)) ival(accN) = dt_size[acc[accN].type & DT_MASK];
		else error(INVALID_DATA_TYPE);
	}
	else ival(accN) = vpost->size;
	accN--;
	memcpy(&acc[accN].val, &acc[accN + 1].val, sizeof(acc[accN].val));
	acc[accN].ind = 0;
	acc[accN].type = DT_INT;
}


/* unary operator */
void op_cast(void) {
	convert(&acc[accN], &acc[accN - 1]);
	accN--;
	memcpy(&acc[accN], &acc[accN + 1], sizeof(data_t));
}


/* unary operator */
void op_unary_plus(void) {
	accN--;
	memcpy(&acc[accN], &acc[accN + 1], sizeof(data_t));
	if(isINT(accN)) ival(accN) = +ival(accN);
	else if(isFPN(accN)) fval(accN) = +fval(accN);
	else error(INVALID_DATA_TYPE);
}


/* unary operator */
void op_unary_minus(void) {
	accN--;
	memcpy(&acc[accN], &acc[accN + 1], sizeof(data_t));
	if(isINT(accN)) ival(accN) = -ival(accN);
	else if(isFPN(accN)) fval(accN) = -fval(accN);
	else error(INVALID_DATA_TYPE);
}


/* unary operator */
void op_logical_not(void) {
	accN--;
	memcpy(&acc[accN], &acc[accN + 1], sizeof(data_t));
	if(isINT(accN)) ival(accN) = !ival(accN);
	else if(isFPN(accN)) fval(accN) = !fval(accN);
	else error(INVALID_DATA_TYPE);
}


/* unary operator */
void op_bitwise_not(void) {
	accN--;
	memcpy(&acc[accN], &acc[accN + 1], sizeof(data_t));
	if(isINT(accN)) ival(accN) = ~ival(accN);
	else error(INVALID_DATA_TYPE);
}


void op_attribution(void) {
	if(!vprec) error(SYNTAX);
	convert(&acc[accN], &acc[accN - 1]);
	accN--;
	memcpy(&acc[accN].val, &acc[accN + 1].val, sizeof(acc[accN].val));
	uint8_t ixt = indexed;
	indexed = indexed_attr;
	var_set(vprec, &acc[accN], 0);
	indexed = ixt;
}


void op_plus_attr(void) {
	if(!vprec) error(SYNTAX);
	uint8_t i = acc[accN - 1].ind;
	op_plus();
	acc[accN].ind = i;
	var_set(vprec, &acc[accN], 0);
}


void op_minus_attr(void) {
	if(!vprec) error(SYNTAX);
	uint8_t i = acc[accN - 1].ind;
	op_minus();
	acc[accN].ind = i;
	var_set(vprec, &acc[accN], 0);
}


void op_multiplication_attr(void) {
	if(!vprec) error(SYNTAX);
	uint8_t i = acc[accN - 1].ind;
	op_multiplication();
	acc[accN].ind = i;
	var_set(vprec, &acc[accN], 0);
}


void op_division_attr(void) {
	if(!vprec) error(SYNTAX);
	uint8_t i = acc[accN - 1].ind;
	op_division();
	acc[accN].ind = i;
	var_set(vprec, &acc[accN], 0);
}


void op_modulo_attr(void) {
	if(!vprec) error(SYNTAX);
	uint8_t i = acc[accN - 1].ind;
	op_modulo();
	acc[accN].ind = i;
	var_set(vprec, &acc[accN], 0);
}


void op_bitwise_or_attr(void) {
	if(!vprec) error(SYNTAX);
	uint8_t i = acc[accN - 1].ind;
	op_bitwise_or();
	acc[accN].ind = i;
	var_set(vprec, &acc[accN], 0);
}


void op_bitwise_xor_attr(void) {
	if(!vprec) error(SYNTAX);
	uint8_t i = acc[accN - 1].ind;
	op_bitwise_xor();
	acc[accN].ind = i;
	var_set(vprec, &acc[accN], 0);
}


void op_bitwise_and_attr(void) {
	if(!vprec) error(SYNTAX);
	uint8_t i = acc[accN - 1].ind;
	op_bitwise_and();
	acc[accN].ind = i;
	var_set(vprec, &acc[accN], 0);
}


void op_bitwise_shiftr_attr(void) {
	if(!vprec) error(SYNTAX);
	uint8_t i = acc[accN - 1].ind;
	op_bitwise_shiftr();
	acc[accN].ind = i;
	var_set(vprec, &acc[accN], 0);
}


void op_bitwise_shiftl_attr(void) {
	if(!vprec) error(SYNTAX);
	uint8_t i = acc[accN - 1].ind;
	op_bitwise_shiftl();
	acc[accN].ind = i;
	var_set(vprec, &acc[accN], 0);
}


void op_bitwise_not_attr(void) {
	if(!vprec) error(SYNTAX);
	uint8_t i = acc[accN - 1].ind;
	op_bitwise_not();
	acc[accN].ind = i;
	var_set(vprec, &acc[accN], 0);
}


void op_plus(void) {
	accN--;
	promote(&acc[accN], &acc[accN + 1]);
	if(isINT(accN)) ival(accN) += ival(accN + 1);
	else if(isFPN(accN)) fval(accN) += fval(accN + 1);
	else error(INVALID_DATA_TYPE);
}


void op_minus(void) {
	accN--;
	promote(&acc[accN], &acc[accN + 1]);
	if(isINT(accN)) ival(accN) -= ival(accN + 1);
	else if(isFPN(accN)) fval(accN) -= fval(accN + 1);
	else error(INVALID_DATA_TYPE);
}


void op_multiplication(void) {
	accN--;
	promote(&acc[accN], &acc[accN + 1]);
	if(isINT(accN)) ival(accN) *= ival(accN + 1);
	else if(isFPN(accN)) fval(accN) *= fval(accN + 1);
	else error(INVALID_DATA_TYPE);
}


void op_division(void) {
	accN--;
	promote(&acc[accN], &acc[accN + 1]);
	if(isINT(accN)) {
		if(ival(accN + 1) == 0) error(DIVISION_BY_ZERO);
		ival(accN) /= ival(accN + 1);
	}
	else if(isFPN(accN)) {
		/* if(fval(accN + 1) == 0.0) error(DIVISION_BY_ZERO); */
		fval(accN) /= fval(accN + 1);
	}
	else error(INVALID_DATA_TYPE);
}


void op_modulo(void) {
	accN--;
	promote(&acc[accN], &acc[accN + 1]);
	if(isINT(accN)) {
		if(ival(accN + 1) == 0) error(DIVISION_BY_ZERO);
		ival(accN) %= ival(accN + 1);
	}
	else error(INVALID_DATA_TYPE);
}


void op_bitwise_and(void) {
	accN--;
	promote(&acc[accN], &acc[accN + 1]);
	if(isINT(accN)) ival(accN) &= ival(accN + 1);
	else error(INVALID_DATA_TYPE);
}


void op_bitwise_or(void) {
	accN--;
	promote(&acc[accN], &acc[accN + 1]);
	if(isINT(accN)) ival(accN) |= ival(accN + 1);
	else error(INVALID_DATA_TYPE);
}


void op_bitwise_xor(void) {
	accN--;
	promote(&acc[accN], &acc[accN + 1]);
	if(isINT(accN)) ival(accN) ^= ival(accN + 1);
	else error(INVALID_DATA_TYPE);
}


void op_bitwise_shiftl(void) {
	accN--;
	promote(&acc[accN], &acc[accN + 1]);
	if(isINT(accN)) ival(accN) <<= ival(accN + 1);
	else error(INVALID_DATA_TYPE);
}


void op_bitwise_shiftr(void) {
	accN--;
	promote(&acc[accN], &acc[accN + 1]);
	if(isINT(accN)) ival(accN) >>= ival(accN + 1);
	else error(INVALID_DATA_TYPE);
}


void op_logical_and(void) {
	accN--;
	promote(&acc[accN], &acc[accN + 1]);
	if(isINT(accN)) ival(accN) = ival(accN) && ival(accN + 1);
	else error(INVALID_DATA_TYPE);
}


void op_logical_or(void) {
	accN--;
	promote(&acc[accN], &acc[accN + 1]);
	if(isINT(accN)) ival(accN) = ival(accN) || ival(accN + 1);
	else error(INVALID_DATA_TYPE);
}


void op_equal(void) {
	accN--;
	promote(&acc[accN], &acc[accN + 1]);
	if(isINT(accN)) ival(accN) = (ival(accN) == ival(accN + 1));
	else if(isFPN(accN)) ival(accN) = (fval(accN) == fval(accN + 1));
	else error(INVALID_DATA_TYPE);
	acc[accN].type = DT_INT;
	acc[accN].ind = 0;
}


void op_not_equal(void) {
	accN--;
	promote(&acc[accN], &acc[accN + 1]);
	if(isINT(accN)) ival(accN) = (ival(accN) != ival(accN + 1));
	else if(isFPN(accN)) ival(accN) = (fval(accN) != fval(accN + 1));
	else error(INVALID_DATA_TYPE);
	acc[accN].type = DT_INT;
	acc[accN].ind = 0;
}


void op_smaller(void) {
	accN--;
	promote(&acc[accN], &acc[accN + 1]);
	if(isINT(accN)) ival(accN) = (ival(accN) < ival(accN + 1));
	else if(isFPN(accN)) ival(accN) = (fval(accN) < fval(accN + 1));
	else error(INVALID_DATA_TYPE);
	acc[accN].type = DT_INT;
	acc[accN].ind = 0;
}


void op_smaller_or_equal(void) {
	accN--;
	promote(&acc[accN], &acc[accN + 1]);
	if(isINT(accN)) ival(accN) = (ival(accN) <= ival(accN + 1));
	else if(isFPN(accN)) ival(accN) = (fval(accN) <= fval(accN + 1));
	else error(INVALID_DATA_TYPE);
	acc[accN].type = DT_INT;
	acc[accN].ind = 0;
}


void op_greater(void) {
	accN--;
	promote(&acc[accN], &acc[accN + 1]);
	if(isINT(accN)) ival(accN) = (ival(accN) > ival(accN + 1));
	else if(isFPN(accN)) ival(accN) = (fval(accN) > fval(accN + 1));
	else error(INVALID_DATA_TYPE);
	acc[accN].type = DT_INT;
	acc[accN].ind = 0;
}


void op_greater_or_equal(void) {
	accN--;
	promote(&acc[accN], &acc[accN + 1]);
	if(isINT(accN)) ival(accN) = (ival(accN) >= ival(accN + 1));
	else if(isFPN(accN)) ival(accN) = (fval(accN) >= fval(accN + 1));
	else error(INVALID_DATA_TYPE);
	acc[accN].type = DT_INT;
	acc[accN].ind = 0;
}


void op_ellipsis(void) {
	error(SYNTAX);
}


void kw_else(void) {
	error(UNEXPECTED_ELSE);
}


void kw_case(void) {
	if(!switch_stack_ix) error(UNEXPECTED_CASE);
	get_value(1);	/* get the case value */
	skip_spaces(0);
	if(*prog != ':') error(COLON_EXPECTED);
	skip_spaces(1);	/* skip the ':' */
	token = SEMICOLON;
}


void kw_default(void) {
	if(!switch_stack_ix) error(UNEXPECTED_DEFAULT);
	skip_spaces(0);
	if(*prog != ':') error(COLON_EXPECTED);
	skip_spaces(1);	/* skip the ':' */
	token = SEMICOLON;
}


void op_question(void) {
	accN--;
	if(isNumeric(accN)) {
		if((isINT(accN) && ival(accN)) || (isFPN(accN) && fval(accN))) {
			memcpy(&acc[accN], &acc[accN + 1], sizeof(data_t));	/* we already have this value */
			skip_spaces(0);
			if(*prog != ':') error(COLON_EXPECTED);
			prog++;	/* skip the ':' character */
			skip_block();
			token_entry = --prog;
		}
		else {
			skip_spaces(0);	/* skip the terminating character */
			if(*prog != ':') error(COLON_EXPECTED);
			skip_spaces(1);	/* skip the ':' character */
			token_entry = prog;
			_get_value(QUESTION, NULL);
		}
	}
	else error(INVALID_DATA_TYPE);
}


void kw_if(void) {
	skip_spaces(0);
	if(*prog != '(') error(OP_PAREN_EXPECTED);
	get_value(0);
	if(isNumeric(accN)) {
		if((isINT(accN) && ival(accN)) || (isFPN(accN) && fval(accN))) {
			execute_block(NULL);
			skip_spaces(0);
			tcode_t tprev = token;
			char *p = prog;
			get_token();
			if(token == ELSE) skip_block();
			else { prog = p; token = tprev; }
		}
		else {
			skip_block();
			skip_spaces(1);
			tcode_t tprev = token;
			char *p = prog;
			get_token();
			if(token == ELSE) execute_block(NULL);
			else { prog = p; token = tprev; }
		}
	}
	else error(INVALID_DATA_TYPE);
}


/* helper function for 'for' to skip until the matching closing paren */
void skip_till_cl_paren(void) {
	int p = 0;
	while(*prog != ETX) {	/* skip it for now */
		if(*prog == ')') {
			if(p-- == 0) break;
			prog++;
		}
		else if(*prog == '(') { prog++; p++; }
		else if(*prog == '\"' || *prog == '\'') { get_token(); }	/* skipping over string constants */
		else skip_spaces(1);	/* skip this character */
	}
}


void kw_for(void) {
	if(for_stack_ix >= MAX_NESTED) error(MAXIMUM_NESTING);
	skip_spaces(0);
	if(*prog != '(') error(OP_PAREN_EXPECTED);
	char *bt = block;
	block = prog++;
	prototype = get_token();
	if(token == DATA_TYPE) new_var(NULL, NULL, 0, 1, 0); 	/* initialisation block */
	else { prog = token_entry; get_value(0); }
	if(token != SEMICOLON) error(SEMICOLON_EXPECTED);
	for_stack[for_stack_ix].cond = prog;	/* record the condition block */
	acc[accN].type = DT_INT;	/* preload condition result 'true' in case of a missing condition block */
	acc[accN].ind = 0;
	ival(accN) = 1;
	skip_spaces(0);
	if(*prog != ';') get_value(1);	/* check the initial condition */
	else prog++;
	if(token != SEMICOLON) error(SEMICOLON_EXPECTED);
	if(isNumeric(accN)) {
		if((isINT(accN) && ival(accN)) || (isFPN(accN) && fval(accN))) {}
		else {
			skip_till_cl_paren();
			if(*prog != ')') error(CL_PAREN_EXPECTED);
			skip_spaces(1);
			skip_block();
			block = bt;
			return;
		}
	}
	else error(INVALID_DATA_TYPE);
	for_stack[for_stack_ix].mod = prog;		/* record the modification block */
	skip_till_cl_paren();
	if(*prog != ')') error(CL_PAREN_EXPECTED);
	skip_spaces(1);
	for_stack[for_stack_ix++].exec = prog;	/* record the execution block */
	while(*prog != ETX) {
		prog = for_stack[for_stack_ix - 1].cond;
		acc[accN].type = DT_INT;	/* preload condition result 'true' in case of a missing condition block */
		acc[accN].ind = 0;
		ival(accN) = 1;
		skip_spaces(0);
		if(*prog != ';') get_value(1);	/* check the condition */
		if(isNumeric(accN)) {
			prog = for_stack[for_stack_ix - 1].exec;
			if((isINT(accN) && ival(accN)) || (isFPN(accN) && fval(accN))) {
				uint8_t t = loop_type;
				loop_type = LOOP_FOR;
				execute_block(NULL);
				loop_type = t;
				if(brkcont_flag) {
					prog = for_stack[for_stack_ix - 1].exec;
					skip_block();
					if(brkcont_flag == BRK_BREAK) { brkcont_flag = BRK_NONE; break; }
					brkcont_flag = BRK_NONE;
				}
				prog = for_stack[for_stack_ix - 1].mod;
				get_value(0);	/* perform the modification operations */
			}
			else { skip_block(); break; }	/* the condition is not satisfied */
		}
		else error(INVALID_DATA_TYPE);
	}
	--for_stack_ix;
	release_memory(block, NULL, 0);	/* release the local variables */
	block = bt;
}


void kw_do(void) {
	if(do_stack_ix >= MAX_NESTED) error(MAXIMUM_NESTING);
	skip_spaces(0);
	do_stack[do_stack_ix++] = prog;
	while(*prog != ETX) {
		uint8_t brf, t = loop_type;
		loop_type = LOOP_DO;
		execute_block(NULL);
		loop_type = t;
		brf = brkcont_flag;
		if(brf) {
			prog = do_stack[do_stack_ix - 1];
			skip_block();
			brkcont_flag = BRK_NONE;
		}
		get_token();
		if(token != WHILE) error(WHILE_EXPECTED);
		skip_spaces(0);
		if(*prog != '(') error(OP_PAREN_EXPECTED);
		get_value(0);
		if(brf == BRK_BREAK) break;
		if(isNumeric(accN)) {
			if((isINT(accN) && ival(accN)) || (isFPN(accN) && fval(accN))) prog = do_stack[do_stack_ix - 1];
			else break;
		}
		else error(INVALID_DATA_TYPE);
	}
	--do_stack_ix;
}


void kw_while(void) {
	if(while_stack_ix >= MAX_NESTED) error(MAXIMUM_NESTING);
	skip_spaces(0);
	if(*prog != '(') error(OP_PAREN_EXPECTED);
	while_stack[while_stack_ix++] = prog;
	while(*prog != ETX) {
		exit_depth = paren_depth;
		get_value(0);		/* get the condition */
		exit_depth = -1;
		if(isNumeric(accN)) {
			if((isINT(accN) && ival(accN)) || (isFPN(accN) && fval(accN))) {
				uint8_t t = loop_type;
				loop_type = LOOP_WHILE;
				execute_block(NULL);
				loop_type = t;
				prog = while_stack[while_stack_ix - 1];
				if(brkcont_flag == BRK_BREAK) {
					get_value(0);
					skip_block();
					brkcont_flag = BRK_NONE;
					break;
				}
				brkcont_flag = BRK_NONE;
			}
			else { skip_block(); break; }
		}
		else error(INVALID_DATA_TYPE);
	}
	--while_stack_ix;
}


void kw_continue(void) {
	if(loop_type == LOOP_NONE || loop_type == LOOP_SWITCH) error(UNEXPECTED_CONTINUE);
	brkcont_flag = BRK_CONT;
	token = SEMICOLON;
}


void kw_break(void) {
	if(loop_type == LOOP_NONE) error(UNEXPECTED_BREAK);
	brkcont_flag = BRK_BREAK;
	token = SEMICOLON;
}


void kw_return(void) {
	if(!current_f) error(UNEXPECTED_TOKEN);
	if((current_f->data.type & DT_MASK) != DT_VOID || current_f->data.ind) {	/* get the return value */
		_get_value(0, NULL);
		convert(&acc[accN], &current_f->data);
	}
	return_flag = 1;
	token = SEMICOLON;
}


void kw_goto(void) {
	label_t *l = get_token();
	if(token != LABEL || !l) error(UNKNOWN_LABEL);
	memcpy(&goto_point, l, sizeof(label_t));
	goto_point.name += goto_point.nlen;	/* move the execution point after the label */
}


void kw_switch(void) {
	if(switch_stack_ix >= MAX_NESTED) error(MAXIMUM_NESTING);
	skip_spaces(0);
	if(*prog != '(') error(OP_PAREN_EXPECTED);
	get_value(1);	/* get the switch parameter */
	if(!isINT(accN)) error(INVALID_DATA_TYPE);
	data_t cond;
	memcpy(&cond, &acc[accN], sizeof(data_t));	/* store the condition */
	skip_spaces(0);
	if(*prog != '{') error(OP_BRACE_EXPECTED);
	switch_stack[switch_stack_ix].dflt = NULL;
	switch_stack[switch_stack_ix++].exec = prog;	/* record the start of the switch() block */
	skip_spaces(1);	/* skip the opening brace */
	char casef = 0;
	do {
		get_token();
		if(*token_entry == '}' && token == UNKNOWN) { prog++; break; }
		if(token != CASE && token != DEFAULT) error(CASE_OT_DEFAULT_EXPECTED);
		if(token == CASE) {
			get_value(1);	/* get the case value */
			skip_spaces(0);
			if(*prog != ':') error(COLON_EXPECTED);
			skip_spaces(1);	/* skip the ':' */
			if(ival(accN) == cond.val.i) {		/* record a match */
				while(*prog != ETX && !brkcont_flag && !return_flag) {
					uint8_t lt = loop_type;
					loop_type = LOOP_SWITCH;
					execute_block(NULL);
					loop_type = lt;
				}
				if(brkcont_flag == BRK_CONT) error(UNEXPECTED_TOKEN);
				casef = 1;
				prog = switch_stack[switch_stack_ix - 1].exec;
				skip_block();
				prog--;
			}
			while(*prog != ETX && *prog != '}') {
				get_token();
				if(token == CASE || token == DEFAULT) { prog = token_entry; break; }
				skip_block();
				skip_spaces((*prog == ';' || *prog == ':') ? 1 : 0);
			}
		}
		else {	/* default statement - just record its location and skip it here */
			skip_spaces(0);
			if(*prog != ':') error(COLON_EXPECTED);
			skip_spaces(1);	/* skip the ':' */
			if(switch_stack[switch_stack_ix - 1].dflt) error(DUPLICATED_DEFAULT);
			switch_stack[switch_stack_ix - 1].dflt = prog;
			while(*prog != ETX && *prog != '}') {
				get_token();
				if(token == CASE || token == DEFAULT) { prog = token_entry; break; }
				skip_block();
				skip_spaces((*prog == ';' || *prog == ':') ? 1 : 0);
			}
		}
	} while(*prog != ETX && !casef);
	if(*prog == ETX) error(CL_BRACE_EXPECTED);
	prog++;
	if(!casef && switch_stack[switch_stack_ix - 1].dflt) { 	/* execute the default statement */
		char *ptemp = prog;
		prog = switch_stack[switch_stack_ix - 1].dflt;
		while(*prog != ETX && !brkcont_flag && !return_flag) {
			uint8_t lt = loop_type;
			loop_type = LOOP_SWITCH;
			execute_block(NULL);
			loop_type = lt;
		}
		if(brkcont_flag == BRK_CONT) error(UNEXPECTED_TOKEN);
		prog = ptemp;
	}
	if(brkcont_flag == BRK_CONT) error(UNEXPECTED_TOKEN);
	brkcont_flag = BRK_NONE;
	--switch_stack_ix;
	token = SEMICOLON;
}


void kw_typedef(void) {
	prototype = get_token();
	if(token != DATA_TYPE) error(DATA_TYPE_EXPECTED);
	if(isType(accN, DT_ENUM) || isType(accN, DT_STRUCT) || isType(accN, DT_UNION)) get_token();
	new_var(NULL, NULL, 0, 0, 1);
	token = SEMICOLON;
}


void op_dot(void) {
	if(!vpost || vpost->data.ind || (vpost->data.type != DT_STRUCT && vpost->data.type != DT_UNION)) error(SYNTAX);
	var_parent = vpost;
	uint8_t dims;
	parent_addr = calcAddr(var_parent, &acc[accN], &dims);
	var_t *v = get_token();
	if(!v) error(UNKNOWN_IDENTIFIER);
	vprec = v;
	if(accN >= MAX_TERMS) error(TOO_COMPLEX);
	memcpy(&acc[accN + 1], &acc[accN], sizeof(data_t));	/* preload indexes for a following operator */
}


void op_arrow(void) {
	if(!vpost || vpost->data.ind == 0 || (vpost->data.type != DT_STRUCT && vpost->data.type != DT_UNION)) error(SYNTAX);
	uint8_t t = vpost->data.type;
	var_get(vpost);
	vpost = vars;
	while(vpost) {
		if(vpost->data.ind == 0 && vpost->data.type == t && (uintptr_t) vpost->alloc == (uintptr_t) acc[accN].val.i) break;
		vpost = vpost->next;
	}
	op_dot();
}

#endif
