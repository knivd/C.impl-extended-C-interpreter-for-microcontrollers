#ifdef CIMPL

#include "cimpl.h"
#include "opr.h"
#include "../xmem.h"
#include "../config.h"
#include "../RIDE/ride.h"   /* for the settings{} structure */

/* libraries */
#include "libs/l_stdlib.h"
#include "libs/l_stdio.h"
#include "libs/l_string.h"
#include "libs/l_math.h"
#include "libs/l_misc.h"
#include "libs/l_conio.h"

#if DISABLE_FILE == 0
#include "libs/l_fatfs.h"
#endif

#include "libs/l_graph.h"

/* structure element for list to keep track of included libraries and other static strings */
typedef struct _included_libs_t {
	char *code;						/* pointer to the allocated memory block with the library */
	struct _included_libs_t *next;	/* next library in the list */
} included_libs_t;

included_libs_t *included_libs;
char *prog_begin;		/* start of the source code */
char *prog_end;			/* end of the source code */
uint8_t get_depth;		/* _get_value() nesting depth */
uint8_t initialised;	/* flag set by new_var() when the new variable was initialised with a value */
int_t dtype_dim[MAX_DIMENSIONS];		/* array dimensions filled by get_token() when taking a data type */
int_t dtype_dim_saved[MAX_DIMENSIONS];	/* storage array with the dimensions filled by get_token() when taking a data type */
uint32_t last_clock;    /* last recorded value from clock() */

const error_msg_t errors[] = {
	{ TERMINATED, "terminated by the user" },
	{ SYNTAX, "syntax error" },
	{ UNEXPECTED_END, "unexpected end of code" },
	{ INVALID_NUMBER, "invalid numeric constant" },
	{ INVALID_CHARACTER, "invalid character constant" },
	{ ID_TOO_LONG, "too long identifier" },
	{ DATA_TYPE_EXPECTED, "data type expected" },
	{ VALUE_EXPECTED, "return value expected" },
	{ IDENTIFIER_EXPECTED, "identifier name expected" },
	{ SEMICOLON_EXPECTED, "semicolon ';' expected" },
	{ COLON_EXPECTED, "colon ':' expected" },
	{ DQUOTE_EXPECTED, "double quote '\"' expected" },
	{ COMMA_EXPECTED, "comma ',' expected" },
	{ OP_PAREN_EXPECTED, "opening paren '(' expected" },
	{ CL_PAREN_EXPECTED, "closing paren ')' expected" },
	{ OP_BRACE_EXPECTED, "opening brace '{' expected" },
	{ CL_BRACE_EXPECTED, "closing brace '}' expected" },
	{ OP_BRACKET_EXPECTED, "opening bracket '[' expected" },
	{ CL_BRACKET_EXPECTED, "closing bracket ']' expected" },
	{ OP_BRACE_OR_QUOTE_EXPECTED, "'{' or '\"' expected" },
	{ CL_BRACE_OR_QUOTE_EXPECTED, "'}' or '\"' expected" },
	{ CASE_OT_DEFAULT_EXPECTED, "'case' or 'default' expected" },
	{ WHILE_EXPECTED, "a closing 'while()' statement expected" },
	{ UNEXPECTED_ELSE, "unexpected 'else' statement" },
	{ UNEXPECTED_CASE, "unexpected 'case' statement" },
	{ UNEXPECTED_DEFAULT, "unexpected 'default' statement" },
	{ UNEXPECTED_BREAK, "unexpected 'break'" },
	{ UNEXPECTED_CONTINUE, "unexpected 'continue'" },
	{ UNEXPECTED_TOKEN, "unexpected token" },
	{ MEMORY_ALLOCATION, "memory allocation failed" },
	{ DUPLICATED_FUNCTION, "duplicated function name" },
	{ DUPLICATED_VARIABLE, "duplicated variable name" },
	{ DUPLICATED_LABEL, "duplicated label" },
	{ DUPLICATED_DEFAULT, "duplicated 'default' clause" },
	{ DUPLICATED_ENUM, "duplicated enum {} definition" },
	{ TOO_COMPLEX, "expression is too complex" },
	{ MAXIMUM_NESTING, "maximum level of nesting reached" },
	{ DIVISION_BY_ZERO, "division by zero" },
	{ IMPOSSIBLE_CONVERSION, "impossible data convertion" },
	{ INVALID_DATA_TYPE, "data type invalid for the operation" },
	{ NOT_ALLOWED_WITH_PTR, "not allowed with pointer parameter" },
	{ TOO_MANY_DIMENSIONS, "too many dimensions" },
	{ TOO_MANY_PARAMETERS, "too many parameters in a function" },
	{ TOO_MANY_POSTMODS, "too many post-modifiers" },
	{ UNKNOWN_IDENTIFIER, "unknown identifier" },
	{ INVALID_ASSIGNMENT, "invalid data type assignment" },
	{ INVALID_POINTER, "invalid pointer reference" },
	{ WRITING_CONST_POINTER, "writing into constant pointer" },
	{ WRITING_CONST, "writing into constant data" },
	{ UNKNOWN_DIRECTIVE, "unknown directive" },
	{ UNKNOWN_LIBRARY, "unrecognised system library" },
	{ UNKNOWN_PRAGMA, "unrecognised #pragma option" },
	{ UNKNOWN_LABEL, "unknown label" },
	{ INVALID_ELLIPSIS, "invalid use of ellipsis" },
	{ UNABLE_TO_INCLUDE, "unable to include library" },
	{ ASSERTION_FAILED, "assertion failed" },
    { INSUFFICIENT_RESOURCE, "insufficient resource" },
{0, NULL}	/* must be present and last in the table */
};

const keyword_t keywords[] = {
	{ 0,  IF, "if", 2, kw_if },
    { 0,  ELSE, "else", 4, kw_else },
    { 0,  FOR, "for", 3, kw_for },
    { 0,  DO, "do", 2, kw_do },
    { 0,  WHILE, "while", 5, kw_while },
    { 0,  BREAK, "break", 5, kw_break },
    { 0,  CONTINUE, "continue",	8, kw_continue },
    { 0,  SWITCH, "switch", 6, kw_switch },
    { 0,  CASE, "case", 4, kw_case },
    { 0,  DEFAULT, "default", 7, kw_default },
    { 0,  RETURN, "return", 6, kw_return },
	{ 17, SIZEOF, "sizeof", 6, op_sizeof },		/* actually a unary operator but looks like a keyword */
	{ 0,  TYPEDEF, "typedef", 7, kw_typedef },
	{ 0,  GOTO, "goto", 4, kw_goto },
{0, 0, NULL, 0, NULL}	/* must be present and last in the table */
};

const keyword_t operators[] = {
	{ 22, INCREMENT, "++", 2, op_increment_pre },	/* ambiguity: pre/post increment */
	{ 1,  INCREMENT, "++", 2, op_increment_post },	/* ambiguity: pre/post increment (must follow pre++) */
	{ 6,  PLUS_ATTR, "+=", 2, op_plus_attr },
	{ 16, PLUS, "+", 1, op_plus },
	{ 21, UNARY_PLUS, "+", 1, op_unary_plus },		/* ambiguity with addition (must follow '+') */
	{ 22, DECREMENT, "--", 2, op_decrement_pre },	/* ambiguity: pre/post increment */
	{ 1,  DECREMENT, "--", 2, op_decrement_post },	/* ambiguity: pre/post increment (must follow pre--) */
	{ 6,  MINUS_ATTR, "-=", 2, op_minus_attr },
	{ 23, STRUCT_ARROW, "->", 2, op_arrow },
	{ 16, MINUS, "-", 1, op_minus },
	{ 21, UNARY_MINUS, "-", 1, op_unary_minus },	/* ambiguity with subtraction (must follow '-') */
	{ 7,  MULTIPLICATION_ATTR, "*=", 2, op_multiplication_attr },
	{ 17, MULTIPLICATION, "*", 1, op_multiplication },
	{ 19, POINTER, "*", 1, op_pointer },			/* ambiguity with multiplication (must follow '*') */
	{ 10, LOGICAL_AND, "&&", 2, op_logical_and },
	{ 3,  BITWISE_AND_ATTR, "&=", 2, op_bitwise_and_attr },
	{ 12, BITWISE_AND, "&", 1, op_bitwise_and },
	{ 19, ADDRESS, "&", 1, op_address },			/* ambiguity with bitwise AND (must follow '&') */
	{ 1,  ELLIPSIS, "...", 3, op_ellipsis },
	{ 23, STRUCT_DOT, ".", 1, op_dot },
	{ 7,  DIVISION_ATTR, "/=", 2, op_division_attr },
	{ 17, DIVISION, "/", 1, op_division },
	{ 13, EQUAL, "==", 2, op_equal },
	{ 2,  ATTRIBUTION, "=", 1, op_attribution },
	{ 13, NOT_EQUAL, "!=", 2, op_not_equal },
	{ 20, LOGICAL_NOT, "!", 1, op_logical_not },
	{ 4,  BITWISE_SHIFTL_ATTR, "<<=", 3, op_bitwise_shiftl_attr },
	{ 14, SMALLER_OR_EQUAL, "<=", 2, op_smaller_or_equal },
	{ 15, BITWISE_SHIFTL, "<<", 2, op_bitwise_shiftl },
	{ 14, SMALLER, "<", 1, op_smaller },
	{ 4,  BITWISE_SHIFTR_ATTR, ">>=", 3, op_bitwise_shiftr_attr },
	{ 14, GREATER_OR_EQUAL, ">=", 2, op_greater_or_equal },
	{ 15, BITWISE_SHIFTR, ">>", 2, op_bitwise_shiftr },
	{ 14, GREATER, ">", 1, op_greater },
	{ 3,  BITWISE_OR_ATTR, "|=", 2, op_bitwise_or_attr },
	{ 9,  LOGICAL_OR, "||", 2, op_logical_or },
	{ 10, BITWISE_OR, "|", 1, op_bitwise_or },
	{ 5,  BITWISE_NOT_ATTR, "~=", 2, op_bitwise_not_attr },
	{ 20, BITWISE_NOT, "~", 1, op_bitwise_not },
	{ 3,  BITWISE_XOR_ATTR, "^=", 2, op_bitwise_xor_attr },
	{ 11, BITWISE_XOR, "^", 1, op_bitwise_xor },
	{ 7,  MODULO_ATTR, "%=", 2, op_modulo_attr },
	{ 17, MODULO, "%", 1, op_modulo },
	{ 8,  QUESTION, "?", 1, op_question },
	{ 19, CAST, ")", 1, op_cast },
{0, 0, NULL, 0, NULL}	/* must be present and last in the table */
};

const keyword_t basic_types[] = {
	{ 0, DT_VOID, "void", 4, NULL },
	{ 0, DT_CHAR, "char", 4, NULL },
	{ 0, DT_SHORT, "short", 5, NULL },
	{ 0, DT_INT, "int", 3, NULL },
	{ 0, DT_LONG, "long", 4, NULL },
	{ 0, DT_FLOAT, "float", 5, NULL },
	{ 0, DT_DOUBLE, "double", 6, NULL },
	{ 0, FT_UNSIGNED, "unsigned", 8, NULL },
	{ 0, FT_SIGNED, "signed", 6, NULL },
	{ 0, FT_CONST, "const", 5, NULL },
	{ 0, FT_STATIC, "static", 6, NULL },
	{ 0, DT_ENUM, "enum", 4, NULL },
	{ 0, DT_STRUCT, "struct", 6, NULL },
	{ 0, DT_UNION, "union", 5, NULL },
	{ 0, DT_BOOL, "bool", 4, NULL },
	#if DISABLE_FILE == 0
	{ 0, DT_FILE, "FILE", 4, NULL },
	#endif
	{ 0, DT_VA_LIST, "va_list", 7, NULL },
	{ 0, FT_STATIC, "static", 6, NULL },
	{ 0, 0, "volatile", 8, NULL },	/* taken but ignored */
	{ 0, 0, "extern", 6, NULL },	/* taken but ignored */
{0, 0, NULL, 0, NULL}	/* must be present and last in the table */
};

const system_library_t system_libs[] = {
    { "<stdio.h>", stdio_const_table, stdio_func_table, NULL, stdio_init },
    { "<stdlib.h>", stdlib_const_table, stdlib_func_table, NULL, NULL },
    { "<string.h>", string_const_table, string_func_table, NULL, NULL },
    { "<math.h>", math_const_table, math_func_table, NULL, NULL },
    { "<stdint.h>", stdint_const_table, NULL, stdint_src, NULL },
    { "<stdbool.h>", stdbool_const_table, NULL, NULL, NULL },
    { "<limits.h>", limits_const_table, NULL, NULL, NULL },
    { "<stdarg.h>", NULL, stdarg_func_table, NULL, NULL },
    { "<ctype.h>", NULL, ctype_func_table, NULL, NULL },
    { "<time.h>", time_const_table, time_func_table, time_src, NULL },
	{ "<assert.h>", NULL, assert_func_table, NULL, NULL },
	{ "<conio.h>", NULL, conio_func_table, NULL, NULL },
    { "<graphics.h>", graphics_const_table, graphics_func_table, graphics_src, graph_init },   /* HW-independent graphics */
	{ "<platform.h>", platform_const_table, platform_func_table, platform_src, pltfm_init },   /* platform-dependent library */
    #if DISABLE_FILE == 0
    { "<fatfs.h>", fatfs_const_table, fatfs_func_table, fatfs_src, stdio_init },	/* FatFs; reusing stdio_init() */
    #endif
    { NULL, NULL, NULL, NULL, NULL }	/* must be the final element in this list */
};


int Cimpl(const char *source) {
    int err_code;
    err_code = setjmp(err_env);
    if(err_code) {
        if(err_code < 0) err_code++;    /* special cases of exit codes 0 and below */
        return err_code;  /* here the execution terminates with an error */
    }
    if(!source || *source == ETX) return 0;
	if(enable_flags & FLAG_VIDEO) {
        enable_flags &= ~FLAG_NO_SCROLL;    /* unlock screen scrolling */
	}

	memset(dt_size, 0, sizeof(dt_size));
    dt_size[DT_VOID] = sizeof(void *);
    dt_size[DT_BOOL] = sizeof(uint8_t);
    dt_size[DT_CHAR] = sizeof(int8_t);
    dt_size[DT_SHORT] = sizeof(int16_t);
    dt_size[DT_INT] = sizeof(int_t);
    dt_size[DT_LONG] = sizeof(int32_t);
    dt_size[DT_LLONG] = sizeof(llong_t);
    dt_size[DT_FLOAT] = sizeof(float);
    dt_size[DT_DOUBLE] = sizeof(double);
    dt_size[DT_LDOUBLE] = sizeof(ldouble_t);
    dt_size[DT_ENUM] = sizeof(int_t);
    dt_size[DT_FILE] = sizeof(void *);
    dt_size[DT_VA_LIST] = sizeof(char *);

	memset(&zero, 0, sizeof(data_t));
	memset(acc, 0, sizeof(acc));
	memset(dtype_dim,0, sizeof(dtype_dim));
	memset(&goto_point, 0, sizeof(label_t));
	memset(post, 0, sizeof(post));
	post_ix = 0;
	prog = prog_begin = (char *) source;
	prog_end = NULL; included_libs = NULL;
	vars = NULL; types = NULL; funcs = NULL; sys_funcs = NULL; labels = NULL;
    callbacks = NULL;
	paren_depth = get_depth = accN = 0;	/* zero depth, default accumulator */
	exit_depth = -1;
	token = UNKNOWN; block = NULL;
	current_f = NULL; exec_depth = return_flag = 0;
	indexed = curr_strbuf = 0;
	ellipsis_ptr = NULL;
	var_parent = NULL; parent_addr = NULL;
	last_clock = assert_flag = 0;

	loop_type = LOOP_NONE;
	brkcont_flag = BRK_NONE;
	memset(do_stack, 0, sizeof(do_stack));
	do_stack_ix = 0;
	memset(while_stack, 0, sizeof(while_stack));
	while_stack_ix = 0;
	memset(for_stack, 0, sizeof(for_stack));
	for_stack_ix = 0;
	memset(switch_stack, 0, sizeof(switch_stack));
	switch_stack_ix = 0;
	vprec = vpost = NULL;
    file_to_run = NULL;

	while(*prog != ETX) {	/* first pass scan to record the function entries */
		skip_spaces(0);
		if(*prog == ETX) break;
		if(*prog == ';' || *prog == ':') { prog++; continue; }
		if(*prog == '#') { process_directive(); continue; }	/* directive */
		void *v = get_token();

		if(token == DATA_TYPE) {	/* new variable or function */
			v = get_token();
			skip_spaces(0);

			if(token == FUNCTION) {
				if(v) {
					if(((func_t *) v)->data.type & FT_STATIC) v = NULL;
					else error(DUPLICATED_FUNCTION);
				}
				func_t *f = NULL;
				x_malloc((byte **) &f, sizeof(func_t));
				if(!f) error(MEMORY_ALLOCATION);
				memcpy(&f->data, &acc[accN], sizeof(data_t));
				f->name = (const char *) (uintptr_t) ival(accN);
				f->nlen = (ival(accN) ? idlen : 0);
				f->next = funcs; funcs = f;
				while(*prog != ETX && *prog != ')') {
					if(++f->pnum > MAX_PARAMS) error(TOO_MANY_PARAMETERS);
					skip_spaces(0);
					if(*prog == ')') break;
					get_token();	/* expecting data type or ellipsis */
					if(token == ELLIPSIS) {
						f->pnum--;
						f->ellipsis_flag = 1;
						skip_spaces(0);
						break;
					}
					else {
						if(token != DATA_TYPE) error(DATA_TYPE_EXPECTED);
						if(f->pnum == 1 && acc[accN].type == DT_VOID && !acc[accN].ind) {	/* (void) */
							skip_spaces(0);
							if(*prog == ')') { f->pnum = 0; break; }
						}
						get_token();	/* skip the identifier name for now */
						if(token != IDENTIFIER) error(IDENTIFIER_EXPECTED);
					}
					skip_spaces(0);
					if(*prog == ')') break;
					if(*prog != ',') error(SYNTAX);
					prog++;
				}
				if(*prog != ')') error(CL_PAREN_EXPECTED);
				prog++;
				skip_block();
				f->after = prog;
				continue;
			}

			else if(token == IDENTIFIER || *prog == '{') {
				new_var(NULL, (var_t *) v, 1, 1, 0);
				continue;
			}

			else {
				while(*prog != ETX) {
					skip_block();
					if(*(prog - 1) == ';') break;
				}
				if(*prog == ETX) error(SYNTAX);
				prog--;
			}
		}

		else {	/* everything else */
			if(token == SYS_FUNCTION) prog--;	/* return back to the opening '(' */
			else skip_spaces(0);
			if(*prog == '(') { prog++; skip_till_cl_paren(); prog++; }	/* skip () blocks after recognised keywords */
		}

		skip_block();
	}

	release_memory(NULL, NULL, 1);	/* release the allocated memory but retain the user's function allocations */
	memset(&acc, 0, sizeof(acc));
	paren_depth = get_depth = accN = post_ix = 0;
	prog_end = prog;
	prog = prog_begin;
	while(*prog != ETX) {
		execute_block(NULL);	/* main execution loop */
	}
	if(goto_point.name) error(UNKNOWN_LABEL);
	error(OK);	/* successful execution */
    return 0;   /* never comes here; only needed to prevent a compiler warning */
}


/* release all data memory allocated for block */
/* (blk) specifies the block to be released (NULL for all) */
/* (*pnt) is an internal recursive parameter; must be always NULL on function call */
/* (level) controls the level of release */
/* 				0: labels and variables, 1: plus static variables and system functions, 2: everything */
void release_memory(char *blk, var_t *pnt, uint8_t level) {
	uint8_t sb;
	for(sb = 0; sb < STR_BUFFERS; sb++) x_free((byte **) &strbuf[sb]);	/* release the string buffers */
	label_t *ln, *l = labels;	/* release the goto labels */
	while(l) {
		ln = l->next;
		if(l->block == blk || !blk) {
			if(labels == l) labels = ln;
			x_free((byte **) &l);
		}
		l = ln;
	}
	var_t *vn, *v = vars;		/* release the variables (except the static) */
	while(v) {
		vn = v->next;
		if(v->parent == pnt) {
			if((v->block == blk && v->depth == exec_depth) || !blk) {
				if((v->data.type & FT_STATIC) == 0 || pnt) {	/* keep static variables */
					if(pnt) release_memory(blk, pnt, level);	/* release structure members associated with this variable */
					if(vars == v) vars = vn;
					x_free((byte **) &v->alloc);
					x_free((byte **) &v);
				}
			}
		}
		v = vn;
	}
	v = types;			/* release the user-defined data types */
	while(v) {
		vn = v->next;
		if((v->block == blk && v->depth == exec_depth) || !blk) {
			if(types == v) types = vn;
			x_free((byte **) &v);
		}
		v = vn;
	}
	if(level < 1) return;
	while(vars) {		/* release any remaining (static) variables */
		vn = vars->next;
		x_free((byte **) &vars->alloc);
		x_free((byte **) &vars);
		vars = vn;
	}
	sys_func_t *sf;
	while(sys_funcs) {	/* release the loaded system functions */
		sf = sys_funcs->next;
		x_free((byte **) &sys_funcs);
		sys_funcs = sf;
	}
	if(level < 2) return;
	while(included_libs) {	/* release the loaded user libraries */
		included_libs_t *fl = included_libs->next;
		x_free((byte **) &included_libs->code);
		x_free((byte **) &included_libs);
		included_libs = fl;
	}
	while(funcs) {		/* release the user function definitions */
		func_t *fn = funcs->next;
		x_free((byte **) &funcs);
		funcs = fn;
	}
}


/* print error code and perform full exit */
void error(error_t e) {
	if(enable_flags & FLAG_VIDEO) {
        enable_flags &= ~FLAG_NO_SCROLL;    /* unlock screen scrolling */
	}
    #if DISABLE_ERROR_TEXT == 0
	if(e != OK) {
		if(assert_flag) e = ASSERTION_FAILED;
        uint_t t;
        for(t = 0; errors[t].s; t++) {
            if(e == errors[t].e) {
                char *text = prog_begin;
                unsigned long int line = 1;
				while(*prog < ' ' && prog > prog_begin) prog--;
                while(prog_begin < prog) {	/* find the source line of the error */
                    prog_begin++;
                    if(*prog_begin == '\n' || *prog_begin == '\0') {
                        line++;
                        text = (prog_begin + 1);
                    }
                }
                if(e != TERMINATED) printf("\r\n(ERROR %d) line %lu: %s\r\n", e, line, errors[t].s);
                else {
                    printf("\r\n(EXIT %d) line %lu: %s\r\n", e, line, errors[t].s);
                    e = -1; /* can't exit with code 0 */
                }
                while(*text && *text < ' ') {
                    if(*text == '\t') printf("  ");
                    text++;
                }
                while(*text && (*text >= ' ' || *text == '\t')) {
                    if(*text == '\t') printf("  "); else printf("%c", *text);
                    text++;
                }
                printf("\r\n");
                break;
            }
        }
	}
    #endif
	assert_flag = 0;
	release_memory(NULL, NULL, 2);	/* release everything */
	while(kbhit() > 0) getchx();	/* clear the keyboard buffer */
    longjmp(err_env, (int) e);
}


/* Ctrl-C by default */
void wait_break(void) {
    if(settings.brk_code < 0) error(TERMINATED);
    if(kbhit() > 0) {
        uint16_t ee = (enable_flags & FLAG_NO_ECHO);
        enable_flags |= FLAG_NO_ECHO;
        int k = getchx();
        if(!ee) enable_flags &= ~FLAG_NO_ECHO;
        if(k == settings.brk_code) error(TERMINATED);
    }
}


/* convert data container (d) to a specified data type (mod) */
void convert(data_t *d, data_t *mod) {
    if((d->type & DT_MASK) <= DT_ENUM) {
        if(!d->ind && !mod->ind) {	/* conversion is performed only on direct data, not on pointers */
            if(isFPN_t(d->type) && mod->ind) error(IMPOSSIBLE_CONVERSION);
            if(isFPN_t(mod->type) && d->ind) error(IMPOSSIBLE_CONVERSION);
			if(isFPN_t(d->type) && !isFPN_t(mod->type) && (isnan(d->val.f) || isinf(d->val.f))) error(IMPOSSIBLE_CONVERSION);
            switch(mod->type & DT_MASK) {

                case DT_BOOL:
                    if(isFPN_t(d->type)) d->val.i = !!((uint8_t) d->val.f);
                    else d->val.i = !!((uint8_t) d->val.i);
                    break;

                case DT_CHAR:
                    if(mod->type & FT_UNSIGNED) {
                        if(isFPN_t(d->type)) d->val.i = ((uint8_t) d->val.f);
                        else d->val.i = ((uint8_t) d->val.i);
                    }
                    else {
                        if(isFPN_t(d->type)) d->val.i = ((int8_t) d->val.f);
                        else d->val.i = ((int8_t) d->val.i);
                    }
                    break;

                case DT_SHORT:
                    if(mod->type & FT_UNSIGNED) {
                        if(isFPN_t(d->type)) d->val.i = ((uint16_t) d->val.f);
                        else d->val.i = ((uint16_t) d->val.i);
                    }
                    else {
                        if(isFPN_t(d->type)) d->val.i = ((int16_t) d->val.f);
                        else d->val.i = ((int16_t) d->val.i);
                    }
                    break;

                case DT_INT:
                    if(mod->type & FT_UNSIGNED) {
                        if(isFPN_t(d->type)) d->val.i = ((uint_t) d->val.f);
                        else d->val.i = ((uint_t) d->val.i);
                    }
                    else {
                        if(isFPN_t(d->type)) d->val.i = ((int_t) d->val.f);
                        else d->val.i = ((int_t) d->val.i);
                    }
                    break;

                case DT_LONG:
                    if(mod->type & FT_UNSIGNED) {
                        if(isFPN_t(d->type)) d->val.i = ((uint32_t) d->val.f);
                        else d->val.i = ((uint32_t) d->val.i);
                    }
                    else {
                        if(isFPN_t(d->type)) d->val.i = ((int32_t) d->val.f);
                        else d->val.i = ((int32_t) d->val.i);
                    }
                    break;

                case DT_LLONG:
                    if(mod->type & FT_UNSIGNED) {
                        if(isFPN_t(d->type)) d->val.i = (ullong_t) d->val.f;
                        else d->val.i = (ullong_t) d->val.i;
                    }
                    else {
                        if(isFPN_t(d->type)) d->val.i = (llong_t) d->val.f;
                        else d->val.i = (llong_t) d->val.i;
                    }
                    break;

                case DT_FLOAT:
                    if(isFPN_t(d->type)) d->val.f = (float) d->val.f;
                    else d->val.f = (float) d->val.i;
                    break;

                case DT_DOUBLE:
                    if(isFPN_t(d->type)) d->val.f = (double) d->val.f;
                    else d->val.f = (double) d->val.i;
                    break;

                case DT_LDOUBLE:
                    if(isFPN_t(d->type)) d->val.f = (ldouble_t) d->val.f;
                    else d->val.f = (ldouble_t) d->val.i;
                    break;

                case DT_ENUM:
                    if(isFPN_t(d->type)) error(IMPOSSIBLE_CONVERSION);
                    else d->val.i = (uint_t) d->val.i;
                    break;

				case DT_FILE:
					if(isFPN_t(d->type)) error(IMPOSSIBLE_CONVERSION);
					break;

                default:
                    error(IMPOSSIBLE_CONVERSION);

            }
        }
    }
    else if((d->type & DT_MASK) != (mod->type & DT_MASK) || (d->ind != mod->ind)
				|| memcmp(d->dim, mod->dim, sizeof(zero.dim)))
			error(IMPOSSIBLE_CONVERSION);
	d->type = mod->type;
	d->ind = mod->ind;
}


/* skip whitespaces and commentaries */
/* (skip_chr) specifies number of characters to first skip unconditionally */
void skip_spaces(uint8_t skip_chr) {
	static uint_t comment = 0;	/* commentary nesting level */
	while(*prog != ETX && skip_chr--) prog++;
    while(*prog != ETX) {
        while(isspaceC(*prog)) prog++;
        if(*prog == '/') {
			prog++;
            if(*prog == '/' && !comment) {	/* check for '//' */
                while(*prog && *prog != ETX && *prog != '\n') prog++;	/* skip everything until the end of the line */
                continue;
            }
            else if(*prog == '*') {	/* check for commentary opening */
                comment++;
                prog += 2;
            }
			else prog--;	/* not '//' nor stardard opening, so return back */
        }
        else if(*prog == '*' && *(prog + 1) == '/') {	/* check for commentary closing */
            if(comment) comment--;
			else error(SYNTAX);
            prog += 2;
            continue;
        }
        if(comment) {
            if(*prog != ETX) prog++;
        }
        else break;
    }
}


/* install a system library */
/* (*lib) must point to a valid system library structure */
void install_sys_library(const system_library_t *lib) {
    const sys_const_t *c = lib->const_table;
    const sys_func_t *f = lib->func_table;
    sys_func_t *sf;
	var_t *v = vars;
	while(v && (strlen(lib->name) != v->nlen || strncmp(lib->name, v->name, v->nlen))) v = v->next;
	if(v == NULL) {	/* install the library only if it has not been installed already, otherwise just skip */
		memset(&acc[accN], 0, sizeof(data_t));
		ival(accN) = (uintptr_t) lib->name;
		acc[accN].type = (FT_CONST | DT_CHAR);
		idlen = strlen(lib->name);
		token = IDENTIFIER;
		prototype = NULL;
		new_var(NULL, NULL, 1, 0, 0);

		while(c && c->nlen) {	/* add the library constants */
			v = vars;
			while(v && (strlen(c->name) != v->nlen || strncmp(c->name, v->name, v->nlen))) v = v->next;
			if(!v) {	/* will ignore redeclaring constants with the same name */
		        x_malloc((byte **) &v, sizeof(var_t));
		        if(!v) error(MEMORY_ALLOCATION);
		        memset(v, 0, sizeof(var_t));
		        memcpy(&v->data, &c->data, sizeof(data_t));
				memcpy(&v->data.val, &c->data, sizeof(v->data.val));
		        v->nlen = c->nlen;
		        v->name = c->name;
                if(!strcmp(v->name, "NAN") || !strcmp(v->name, "NaN")) {
                    double z1f = 0.0;
                    double z2f = 0.0;
                    v->data.val.f = (z1f/z2f);  /* generate NaN as it is apparently not defined in all compilers */
                }
		        v->next = vars; vars = v;
			}
	        c++;
	    }

	    while(f && f->nlen) {	/* add the library functions */
	        sf = NULL;
	        x_malloc((byte **) &sf, sizeof(sys_func_t));
	        if(!sf) error(MEMORY_ALLOCATION);
	        memcpy(sf, f, sizeof(sys_func_t));
	        sf->next = sys_funcs; sys_funcs = sf;
	        f++;
	    }

	    if(lib->src && *(lib->src)) {	/* execute any provided initialisation source code for the library */
	        char *tp = prog;
	        prog = token_entry = (char *) lib->src;
	        while(*prog != '\0' && *prog != ETX) {	/* execute the code for this library */
				if(*prog == ';') { skip_spaces(1); continue; }
				execute_block(NULL);
			}
	        prog = token_entry = tp;
	    }

        if(lib->init) (lib->init)();
	}
}


/* process a directive */
void process_directive(void) {
	if(!strncmp(prog, "#include ", 9)) {
        skip_spaces(9);	/* skip the directive and and following spaces */
		const system_library_t *sl = system_libs;
        while(sl->name && strncmp(prog, sl->name, strlen(sl->name))) sl++;
        if(sl->name) {  	/* found the library */
            prog += strlen(sl->name);
            install_sys_library(sl);
        }
		#if DISABLE_FILE == 0
		else if(*prog == '\"') {	/* include a user defined library */
			get_param(&d1, (FT_CONST | DT_CHAR), 1);	/* get the file name */
			if(!prog_end) {			/* only act on this during the initial pre-processing stage */
				char *fn = (char *) (uintptr_t) d1.val.i;
				FIL ff;
	            FRESULT fr = f_open(&ff, fn, (FA_OPEN_EXISTING | FA_READ));
	            if(fr == FR_OK) {
					included_libs_t *l = NULL;
					x_malloc((byte **) &l, sizeof(included_libs_t));
					if(!l) { f_close(&ff); error(UNABLE_TO_INCLUDE); }
					l->next = included_libs; included_libs = l;
					uint32_t read, fs = f_size(&ff);
					l->code = NULL;
					x_malloc((byte **) &l->code, (fs + strlen(ETXSTR)));
					if(!l->code) { f_close(&ff); error(UNABLE_TO_INCLUDE); }
					fr = f_read(&ff, l->code, fs, &read);
					if(fr != FR_OK || read != fs) { f_close(&ff); error(UNABLE_TO_INCLUDE); }
					f_close(&ff);
					strcpy((l->code + fs), ETXSTR);
	            }
				else error(UNABLE_TO_INCLUDE);
				char *pp = prog; prog = fn;
				execute_block(NULL);
				prog = token_entry = pp;
			}
		}
		#endif
        else error(UNKNOWN_LIBRARY);
    }

    #if 0
	else if(!strncmp(prog, "#pragma ", 8)) {
		skip_spaces(8);		/* skip the directive and and following spaces */

        /* TODO: other pragmas */

        error(UNKNOWN_PRAGMA);
	}
    #endif

	else error(UNKNOWN_DIRECTIVE);
}


/* get from the source and return a 4-bit (single digit) hexadecimal number */
uint8_t get_hex_digit(char **src) {
    char *s = *src;
    char c = *(s++);
    if(c >= '0' && c <= '9') c = (c - '0');
    else if(c >= 'a' && c <= 'f') c = (c - 'a' + 10);
    else if(c >= 'A' && c <= 'F') c = (c - 'A' + 10);
    else s--;   /* error(INVALID_CHARACTER); */
    *src = s;
    return (uint8_t) c;
}


/* helper function used by get_token() for dealing with suffix in integer numbers */
void int_suffix(uint8_t dest) {
    if(*prog == 'u' || *prog == 'U') { 		/* 'U' suffix for unsigned types */
		prog++;
		acc[dest].type |= FT_UNSIGNED;
	}
    if(*prog == 'l' || *prog == 'L') {		/* 'L' suffix for long int types */
        prog++;
        if(*prog == 'l' || *prog == 'L') {	/* 'LL' suffix for long long int */
            prog++;
            acc[dest].type |= DT_LLONG;
        }
        else acc[dest].type |= DT_LONG;		/* long int */
    }
    else acc[dest].type |= DT_INT;			/* default integers */
}


/* get a single character from a source constant */
/* the leading double quote character must be skipped externally before calling this function */
/* observes the pre-defined character constants and numeric codes */
char get_char(char **src) {
    char c = '\0';
    char *s = *src;
    while(*s == '\\' && *(s + 1) <= ' ') {	/* a single '\' character allows continuing the string on the next line */
        s++;	/* skip the '\' */
        while(*s <= ' ') s++;
    }
    if(*s == '\"') { c = '\0'; s++; }       /* the closing double quote will return 0 */
    else if(*s == '\\') {  /* special character constant */
        s++;	/* skip the '\' character */
        if(*s == '0') c = '\0';
        else if(*s == 'n') c = '\n';
        else if(*s == 'r') c = '\r';
        else if(*s == 't') c = '\t';
        else if(*s == 'b') c = '\b';
        else if(*s == 'a') c = '\a';
        else if(*s == 'f') c = '\f';
        else if(*s == 'e') c = 27;
        else if(*s == '\\') c = '\\';
        else if(*s == '\'') c = '\'';
        else if(*s == '\"') c = '\"';
        else if(*s == '\?') c = '\?';
        else if(*s == 'x') {    /* up to 2-digit hexadecimal ASCII code */
            s++;
            c = (char) ((get_hex_digit(&s) << 4) + get_hex_digit(&s));
            s--;
        }
        else if(isdigitC(*s)) {  /* up to 3-digit decimal ASCII code */
            uint8_t t = (uint8_t) (*(s++) - '0');
            if(isdigitC(*s)) t = (t * 10) + (uint8_t) (*(s++) - '0'); /* else error(INVALID_CHARACTER); */
            if(isdigitC(*s)) t = (t * 10) + (uint8_t) (*(s++) - '0'); /* else error(INVALID_CHARACTER); */
            c = (char) t;
            s--;
        }
        else error(INVALID_CHARACTER);
        s++;
    }
	else if(*s == '\0') c = *s;
	else c = *(s++);
    *src = s;
    return c;
}


/* get parameter of specified type and store it into the *dt structure */
/* the parameters 'data_type' and 'ind' are the same as their counterpart fields in a mod_t structure */
void get_param(data_t *dt, uint8_t data_type, uint8_t ind) {
    data_t newt;
	memset(&newt, 0, sizeof(newt));
	newt.type = data_type;
	newt.ind = ind;
	skip_spaces(0);
	uint8_t strcf = (*prog == '\"');
    get_value(1);
	if(strcf) {	/* a string constant needs taking into memory */
		if(++curr_strbuf >= STR_BUFFERS) curr_strbuf = 0;
		x_free((byte **) &strbuf[curr_strbuf]);
		x_malloc((byte **) &strbuf[curr_strbuf], (str_len + 1));
		if(!strbuf[curr_strbuf]) error(MEMORY_ALLOCATION);
		char *s = prog;
		prog = (char *) (uintptr_t) ival(accN);
		char *d = strbuf[curr_strbuf];
		do {
			while((*(d++) = get_char(&prog)));
			d--;
			skip_spaces(0);
	    } while(*(prog++) == '\"');	/* if another string is following immediately, it will be joined */
		ival(accN) = (uintptr_t) strbuf[curr_strbuf];
		prog = s;
	}
	if(dt != &acc[accN]) memcpy(dt, &acc[accN], sizeof(data_t));
	convert(dt, &newt);
}


/* get identifier pointed by (*prog) and modify (*prog) accordingly */
char *get_id(uint8_t *nlen) {
	char *p = prog;
	*nlen = 0;
	if(isidchr1C(*prog)) {	/* the first character in an identifier is a different set from the rest (no digits) */
		prog++;
		(*nlen)++;
		while(isidchrC(*prog)) {     /* get the length in *len */
	        prog++;
	        if(++(*nlen) > MAX_ID_LENGTH) error(ID_TOO_LONG);
	    }
	}
    if(*nlen == 0) error(IDENTIFIER_EXPECTED);
    return p;
}


/* get token and return its code in (token) and any acquired data in (acc[accN]) */
/* will return pointer to structure relevant to the type of the token */
void *get_token(void) {
	skip_spaces(0);
	token_entry = prog;

	/* end of source */
	if(*prog == ETX) {
		token = UNKNOWN;
		return NULL;
	}

	/* semicolon */
	if(*prog == ';') {
		prog++;
		token = SEMICOLON;
		return NULL;
	}

	/* numeric constants */
    if(isdigitC(*prog)) {
		uint8_t t;
		ullong_t i = 0;
		token = NUMBER;
		acc[accN].type = FT_CONST;

        /* hexadecimal unsigned constants staring with '0x' */
        if(*prog == '0' && (*(prog+1) == 'x' || *(prog+1) == 'X')) {
            t = 16;     /* maximum number of hexadecimal digits */
            prog += 2;
            while(t-- && isXdigitC(*prog)) i = (i << 4) + get_hex_digit(&prog);
            if(t == 0) error(INVALID_NUMBER);
            ival(accN) = i;
            int_suffix(accN);
            acc[accN].type |= FT_UNSIGNED;	/* hexadecimal numbers are always considered unsigned */
        }

        /* octal unsigned constants staring with '0' */
        else if(*prog == '0' && isdigitC(*(prog+1))) {
            t = 21; /* maximum number of octal digits */
            prog++; /* skip the '0' */
            if(*prog > '7') error(INVALID_NUMBER);
            while(t-- && *prog >= '0' && *prog <= '7') i = (i << 3) + (*(prog++) - '0');
            if(t == 0) error(INVALID_NUMBER);
            ival(accN) = i;
            int_suffix(accN);
            acc[accN].type |= FT_UNSIGNED;	/* hexadecimal numbers are always considered unsigned */
        }

        /* binary unsigned constants staring with '0b' */
        else if(*prog == '0' && (*(prog+1) == 'b' || *(prog+1) == 'B')) {
            t = 64; /* maximum number of binary digits */
            prog += 2;
            while(t-- && (*prog == '0' || *prog == '1')) i = (i << 1) + (*(prog++) - '0');
            if(t == 0) error(INVALID_NUMBER);
            ival(accN) = i;
            int_suffix(accN);
            acc[accN].type |= FT_UNSIGNED;	/* binary numbers are always considered unsigned */
        }

        /*
		decimal constants (optionally starting with '0d')
		supported format: [sign] nnn [.nnn [ e/E [sign] nnn [.nnn] ] ]
		trailing 'f' or 'F' forces DT_FLOAT (default)
		trailing 'd' or 'D' forces DT_DOUBLE
        trailing 'l' or 'L' forces DT_LDOUBLE
		default storage type in integers is DT_INT
		trailing 'u' or 'U' in integers forces unsigned (signed by default)
		trailing 'l' or 'L' in integers forces DT_LONG
		trailing 'll' or 'LL' in integers forces DT_LLONG
        */
        else {
            ldouble_t v = 0.0;  /* fully constructed floating point value */
            ldouble_t f = 0.0;  /* factor */
            ldouble_t e = 0.0;  /* exponent */
            int8_t dp = 0;      /* 1:'decimal point found' flag */
            int8_t ef = 0;      /* 1:'E found' flag */
            int8_t es = 1;      /* 'E' sign 1:positive, -1:negative */
            t = 60;             /* maximum number of characters in the number */
            if(*prog == '0' && (*(prog+1) == 'd' || *(prog+1) == 'D')) prog += 2;   /* just skip the '0d' part */
            if(!isdigitC(*prog)) error(INVALID_NUMBER);
            while(t-- && *prog != ETX) {
                if(isdigitC(*prog)) {
                    if(!ef) {
                        if(!dp) i = (10 * i) + (*prog - '0');
                        else { v += (f * (*prog - '0')); f *= 0.1; }
                    }
                    else {
                        if(!dp) e = (10 * e) + (*prog - '0');
                        else { e += (f * (*prog - '0')); f *= 0.1; }
                    }
                    prog++;
                }
                else {
                    if(*prog == '.') {
                        if(!dp) {
                            if(!ef) v = (double) i;
                            dp = 1; f = 0.1;
                            prog++;
                        }
                        else error(INVALID_NUMBER);
                    }
                    else if(*prog == 'e' || *prog == 'E') {
                        if(!ef) {
                            if(!dp) v = (double) i;
                            dp = 0; ef = 1;
                            prog++;
                            if(*prog == '-' || *prog == '+') {
                                if(*(prog++) == '-') { es = -1; f = 0.1; }
                            }
                        }
                        else error(INVALID_NUMBER);
                    }
                    else break;
                }
            }
            if(t == 0) error(INVALID_NUMBER);
            if(dp || ef || *prog == 'f' || *prog == 'F' || *prog == 'd' || *prog == 'D' ||
                	((*prog == 'l' || *prog == 'L') && (dp || ef))) {   /* float or [long] double */
				if(!dp && !ef) v = (ldouble_t) i;		/* it is a floating point number but was given in integer form */
                v *= pow(10, (e * es));
                fval(accN) = v;
                if(*prog == 'l' || *prog == 'L') {      /* forced long double */
                    prog++;     /* skip the suffix */
                    acc[accN].type |= DT_LDOUBLE;
                }
                else if(*prog == 'd' || *prog == 'D') { /* forced double */
                    prog++;     /* skip the suffix */
                    acc[accN].type |= DT_DOUBLE;
                }
                else {  /* forced float (also default) */
                    if(*prog == 'f' || *prog == 'F') prog++;    /* skip the optional suffix */
                    acc[accN].type |= DT_FLOAT;
                }
            }
            else {      /* integer types */
                if(e) i *= (llong_t) pow(10, (e * es));
                ival(accN) = i;
                int_suffix(accN);
            }
        }
    }

	/* single quote characters */
    else if(*prog == '\'') {
		token = NUMBER;
		acc[accN].type = (FT_CONST | DT_CHAR);
		acc[accN].ind = 0;
        prog++; /* skip the opening single quote */
        if(*prog != '\"') ival(accN) = get_char(&prog);
        else {  /* bypassing get_char() here for double quote characters */
            ival(accN) = '\"';
            prog++;
        }
        if(*prog != '\'') error(INVALID_CHARACTER);
        prog++; /* skip the closing single quote */
    }

    /* double quote character string constants */
    else if(*prog == '\"') {
		token = STRING;
		str_len = 0;
		acc[accN].type = (FT_CONST | DT_CHAR);
		acc[accN].ind = 1;
        ival(accN) = (uintptr_t) ++prog;	/* record the start of the string after the opening double quote */
        do {
			while(get_char(&prog)) str_len++;
			skip_spaces(0);
        } while(*(prog++) == '\"');	/* if another string is following immediately, it will be joined */
        prog--;
        if(*prog == ETX || *prog == '\0') error(DQUOTE_EXPECTED);
    }

	/* operators */
	else if(isoprC(*prog)) {
		token = OPERATOR;
		if(*prog == '(') {	/* an opening paren... speculate that it is a cast operator */
			char *p = prog;
			prog++;
			get_token();
			if(token == DATA_TYPE) {
				skip_spaces(0);
				if(*prog == ')') { p = prog; token = CAST; }
				else token = UNKNOWN;
			}
			else token = UNKNOWN;
			prog = p;
		}
		if(token != UNKNOWN) {
			const keyword_t *k = operators;
			while(k->l && strncmp(prog, k->s, k->l)) k++;
			if(k->l) {
				token = k->t;
				prog += k->l;
				return (void *) k;	/* return pointer to the operator structure */
			}
			else error(SYNTAX);
		}
	}

	/* identifiers of any type (labels, data types, keywords, functions, variables) */
	else if(isidchr1C(*prog)) {
		char *id = get_id(&idlen);	/* get the identifier */

		/* check if it is a label (except 'default:' which is a reserved word) */
		if(*prog == ':' && (idlen != 7 || strncmp(id, "default", 7))) {
			token = LABEL;
			prog++;	/* skip the ':' character */
			ival(accN) = (uintptr_t) id;	/* store the label entry in .data and length is in the global (idlen) */
			label_t *l = labels;
			while(l && (idlen != l->nlen || strncmp(id, l->name, l->nlen))) l = l->next;
			return (void *) l;	/* return NULL if the label is unknown or pointer to the label structure */
		}

		/* check in the keywords */
		const keyword_t *k = keywords;
		while(k->l && (idlen != k->l || strncmp(k->s, id, k->l))) k++;
		if(k->l) {
			token = k->t;
			return (void *) k;	/* return pointer to the keyword structure */
		}

		/* check in the data types (information passed in .type and .ind fields) */
		k = basic_types;
		while(k->l && (idlen != k->l || strncmp(k->s, id, k->l))) k++;
		if(k->l) {
			uint8_t void_flag = (k->t == DT_VOID);
			memset(&acc[accN], 0, sizeof(data_t));
			uint8_t long_cnt = 0;

			do {
				if(k->t != DT_LONG) {
					if(k->t == FT_CONST && (acc[accN].type & FT_CONST)) acc[accN].type |= FT_CONSTPTR;
					acc[accN].type |= k->t;
				}
				else long_cnt++;
				skip_spaces(0);
				k = basic_types;
				while(k->l && (strncmp(prog, k->s, k->l) ||
						(*(prog + k->l) != ' ' && *(prog + k->l) != '*' && *(prog + k->l) != ')'))) k++;
				prog += k->l;
			} while(k->l);

			memset(&dtype_dim, 0 ,sizeof(dtype_dim));
			skip_spaces(0);
			if(*prog == '[') {			/* dimensions are provided */
				char *entry_saved = token_entry;
				uint8_t ixt = indexed;
				uint8_t idlen_save = idlen;
				skip_spaces(1);			/* skip the opening bracket */
				uint8_t index = 0;
				do {
					if(index >= MAX_DIMENSIONS) error(TOO_MANY_DIMENSIONS);
					if(*prog == ']') {	/* unspecified dimension [] */
						acc[accN].ind++;
						prog++;
						break;
					}
					incN();
					_get_value(0, NULL);
					vprec = NULL;
					if(!isINT(accN)) error(INVALID_DATA_TYPE);
					accN--;
					dtype_dim[index++] = ival(accN + 1);
					skip_spaces(0);
					if(*prog != ']') error(CL_BRACKET_EXPECTED);
					skip_spaces(1);		/* skip the closing bracket */
					if(*prog != '[') break;
					skip_spaces(1);		/* skip the opening bracket */
				} while(1);
				idlen = idlen_save;
				indexed = ixt;
				token_entry = entry_saved;
			}

			while(*prog == '*') { prog++; acc[accN].ind++; }
			if(long_cnt) {	/* 'long' or 'long long' prefix */
				if(((acc[accN].type & DT_MASK) == DT_INT) || !(acc[accN].type & DT_MASK)) {
					acc[accN].type &= ~ DT_MASK;
					acc[accN].type |= (long_cnt + DT_INT);
				}
				else if((acc[accN].type & DT_MASK) == DT_DOUBLE) acc[accN].type += long_cnt;
				else error(SYNTAX);
			}
			else if(((acc[accN].type & DT_MASK) == 0) && !void_flag) acc[accN].type |= DT_INT;

			token = DATA_TYPE;
			return NULL;
		}

		/* check in the user-defined data types */
		var_t *zz = NULL, *z = types;
		while(z) {
			if(idlen == z->nlen && !strncmp(id, z->name, z->nlen)) {
				if(!zz) zz = z;
				if(z->block == block && z->depth == exec_depth) { zz = z; break; }	/* found the exact match */
				if(z->block >= zz->block && z->depth >= zz->depth) zz = z;	/* ok but keep searching for better */
			}
			z = z->next;
		}

		if(zz) {
			memcpy(&dtype_dim, &zz->data.dim, sizeof(acc[accN].dim));
			acc[accN].type = zz->data.type;
			acc[accN].ind = 0;
			indexed = 0;
			while(indexed < MAX_DIMENSIONS && dtype_dim[indexed]) indexed++;
			skip_spaces(0);
			if(*prog == '[') {			/* dimensions are provided */
				char *entry_saved = token_entry;
				uint8_t ixt = indexed;
				uint8_t idlen_save = idlen;
				skip_spaces(1);			/* skip the opening bracket */
				do {
					if(indexed >= MAX_DIMENSIONS) error(TOO_MANY_DIMENSIONS);
					if(*prog == ']') {	/* unspecified dimension [] */
						acc[accN].ind++;
						prog++;
						break;
					}
					incN();
					_get_value(0, NULL);
					vprec = NULL;
					if(!isINT(accN)) error(INVALID_DATA_TYPE);
					accN--;
					dtype_dim[indexed++] = ival(accN + 1);
					skip_spaces(0);
					if(*prog != ']') error(CL_BRACKET_EXPECTED);
					skip_spaces(1);		/* skip the closing bracket */
					if(*prog != '[') break;
					skip_spaces(1);		/* skip the opening bracket */
				} while(1);
				memcpy(&acc[accN].dim, &dtype_dim, sizeof(dtype_dim));
				idlen = idlen_save;
				indexed = ixt;
				token_entry = entry_saved;
			}
			skip_spaces(0);
			while(*prog == '*') { prog++; acc[accN].ind++; }

			token = DATA_TYPE;
			return (void *) zz;	/* return pointer to the type definition structure */
		}

		/* functions and variables */
		ival(accN) = (uintptr_t) id;	/* the global (idlen) already holds the length */
		skip_spaces(0);

		if(*prog == '(') {		/* opening '(' indicates that the identifier is a function */
			prog++;				/* skip the opening paren */
			sys_func_t *sf = sys_funcs;
			while(sf && (idlen != sf->nlen || strncmp(id, sf->name, sf->nlen))) sf = sf->next;
			if(sf) {
				token = SYS_FUNCTION;
				return (void *) sf;
			}
			token = FUNCTION;
			func_t *f = funcs;
			while(f && (idlen != f->nlen || strncmp(id, f->name, f->nlen))) f = f->next;
			return (void *) f;	/* return NULL if the function is unknown or pointer to the function structure */
		}

		if(dtype_dim[0]) {
			memcpy(&dtype_dim_saved, &dtype_dim, sizeof(dtype_dim));
			memcpy(&acc[accN].dim, &dtype_dim, sizeof(dtype_dim));
			memset(&dtype_dim, 0, sizeof(dtype_dim));
		}
		else memset(&acc[accN].dim, 0, sizeof(acc[accN].dim));

		indexed = 0;
		if(*prog == '[') {	/* dimensions are provided */
			char *entry_saved = token_entry;
			uint8_t idlen_save = idlen;
			skip_spaces(1);			/* skip the opening bracket */
			while(indexed < MAX_DIMENSIONS && acc[accN].dim[indexed]) indexed++;
			do {
				if(indexed >= MAX_DIMENSIONS) error(TOO_MANY_DIMENSIONS);
				if(*prog == ']') {	/* unspecified dimension [] */
					acc[accN].ind++;
					prog++;
					break;
				}
				uint8_t ixt = indexed;
				incN();
				_get_value(0, NULL);
				vprec = NULL;
				indexed = ixt;
				if(!isINT(accN)) error(INVALID_DATA_TYPE);
				accN--;
				acc[accN].dim[indexed++] = ival(accN + 1);
				skip_spaces(0);
				if(*prog != ']') error(CL_BRACKET_EXPECTED);
				skip_spaces(1);		/* skip the closing bracket */
				if(*prog != '[') break;
				skip_spaces(1);		/* skip the opening bracket */
			} while(1);
			idlen = idlen_save;
			token_entry = entry_saved;
		}

		var_t *vv = NULL, *v = vars;
		while(v) {
			if(v->parent == var_parent && idlen == v->nlen && !strncmp(id, v->name, v->nlen)) {
				if(v->block == block && v->depth == exec_depth) { vv = v; break; }	/* found the exact match */
				if(!vv) vv = v;
				else if((v->block == NULL) ||
						(v->block >= vv->block && v->depth >= vv->depth)) vv = v;	/* ok but keep searching for better */
			}
			v = v->next;
		}

		token = IDENTIFIER;
		return (void *) vv;	/* return NULL if the variable is unknown or pointer to the variable structure */

	}

	else if(*prog == ETX) error(UNEXPECTED_END);
    else token = UNKNOWN;
	return NULL;
}


/* evaluate expression(s) and get one or more value(s) */
/* result is returned in acc[accN] */
/* normally the parameter (force_single) should be 1 with the exception of cases such as for(), while(), if() */
void get_value(uint8_t force_single) {
	do {
		memset(&acc[accN], 0, sizeof(data_t));
		_get_value(0, NULL);
		vprec = NULL; var_parent = NULL; parent_addr = NULL;
		if(*prog != ',' || force_single) break;
	} while(prog++);
	execute_postmods();
}


/* INTERNAL; use get_value() instead */
/* evaluate a single expression and return the result in acc[accN] */
/* the parameter must be 0 on entry */
void _get_value(tcode_t topr, void *vpre) {
	if(get_depth >= MAX_TERMS) error(TOO_COMPLEX);
	tcode_t tprev = topr;
	void *v = (vpre? vpre : get_token());
	do {
		skip_spaces(0);
		if(token == UNKNOWN) {
			if(*prog != '(') break;
			prog++;
			get_depth++; paren_depth++;
			vprec = NULL;
			_get_value(0, NULL);
			get_depth--; paren_depth--;
			if(*prog != ')') error(CL_PAREN_EXPECTED);
			prog++; tprev = KEYWORD;
			if((int16_t) paren_depth == exit_depth) break;	/* force exit at specific parenthesis depth */
			incN(); v = get_token(); accN--;
			if(token < OPERATOR && *token_entry != ')') { prog = token_entry; break; }
			continue;
		}
		if(token == SEMICOLON) break;	/* check for exit after deeper levels */
		if((tprev && tprev < OPERATOR) && token < OPERATOR) error(SYNTAX);

		if(token > OPERATOR) {
			uint8_t unary_flag = 0;
			keyword_t *k = (keyword_t *) v;

			if(token == INCREMENT) {
				if(vprec) k++;	/* post type operator */
				else unary_flag = 1;
			}
			else if(token == DECREMENT) {
				if(vprec) k++;	/* post type operator */
				else unary_flag = 1;
			}

			if(tprev > OPERATOR || !tprev) {	/* adjust the code for some unary operators */
				if(token == MULTIPLICATION) { unary_flag = 1; token = POINTER; k++; }
				else if(token == BITWISE_AND) { unary_flag = 1; token = ADDRESS; k++; }
				else if(token == MINUS) { unary_flag = 1; token = UNARY_MINUS; k++; }
				else if(token == PLUS) { unary_flag = 1; token = UNARY_PLUS; k++; }
				else if(token == CAST) unary_flag = 1;
			}

			if(unary_flag || ((token + (k->s[k->l - 1] == '=')) > topr)) {	/* hack to work as '>=' in right-to-left ops */
				get_depth++;
				/* if(k->func) { */

				var_t *vtmp = vprec;
				if(k->t != STRUCT_DOT && k->t != STRUCT_ARROW) {
					indexed_attr = indexed;
					var_t *vpp = var_parent; var_parent = NULL;
					uint8_t *pap = parent_addr; parent_addr = NULL;
					uint8_t ixt = indexed;
					incN(); _get_value(token, NULL);	/* get the following parameter */
					indexed = ixt;
					var_parent = vpp; parent_addr = pap;
					vpost = vprec; vprec = vtmp;
					((void (*)(void)) k->func)();       /* execute the operator handler */
					vprec = vpost;

				}
				else {	/* referring to structure and union members */
					vpost = vprec; vprec = vtmp;
					((void (*)(void)) k->func)();	/* execute the operator handler */
					var_get(vprec);
					token_entry = prog;
				}
				tprev = UNKNOWN;

				/* } */
				get_depth--;
				if(token == SEMICOLON) break;
			}

			else if(get_depth) { prog = token_entry; token = tprev; break; }

			prog = token_entry; token = tprev;		/* unget the token */
			if(token == UNKNOWN) token = NUMBER;	/* at this point it is known that there has been an operand */
		}

		else if(token == NUMBER || token == STRING) {
			if(tprev == NUMBER || tprev == STRING ||
				tprev == IDENTIFIER || tprev == FUNCTION ||
				tprev == SYS_FUNCTION) error(SYNTAX);
			vprec = NULL;
		}

		else if(token == IDENTIFIER) {
			if(tprev == NUMBER || tprev == STRING ||
				tprev == IDENTIFIER || tprev == FUNCTION ||
				tprev == SYS_FUNCTION) error(SYNTAX);
			if(!v) error(UNKNOWN_IDENTIFIER);
			vprec = v;
			var_get(v);
		}

		else if(token == FUNCTION) {
			if(tprev == NUMBER || tprev == STRING ||
				tprev == IDENTIFIER || tprev == FUNCTION ||
				tprev == SYS_FUNCTION) error(SYNTAX);
			if(!v) error(UNKNOWN_IDENTIFIER);
			vprec = NULL;
			execute_function((func_t *) v);
		}

		else if(token == SYS_FUNCTION) {
			if(tprev == NUMBER || tprev == STRING ||
				tprev == IDENTIFIER || tprev == FUNCTION ||
				tprev == SYS_FUNCTION) error(SYNTAX);
			if(!v) error(UNKNOWN_IDENTIFIER);
			vprec = NULL;
			if(!assert_flag) ((sys_func_t *) v)->fn();
			else ival(accN) = 1;	/* only confirm the assertion */
			skip_spaces(0);
			if(*prog != ')') error(CL_PAREN_EXPECTED);
			prog++;
		}

		else error(SYNTAX);
		if(token == SEMICOLON) break;
		tprev = token;
		v = get_token();
	} while(1);
}


/* skip an entire program block */
void skip_block(void) {
	int k = 1;
	skip_spaces(0);
	if(!memcmp(prog, "for", 3) && (isspaceC(*prog + 3) || *(prog + 3) == '(')) {	/* special case for the for() statement */
		skip_spaces(3);	/* skip the 'for' keyword */
		prog++;			/* skip the opening paren */
		skip_till_cl_paren();	/* skip the control block */
		if(*prog == ')') prog++;
		skip_block();		/* skip the execution block */
	}
	else if(*prog == '{') {	/* skip a full {} block */
		prog++;
		while(*prog != ETX) {
			if(*prog == '}') {
				if(--k == 0) break;
				prog++;
			}
			else if(*prog == '{') { prog++; k++; }
			else if(*prog == '\"' || *prog == '\'') { 	/* skipping over string constants */
				indexed = acc[accN].ind = 0;
				incN(); get_token(); accN--;
			}
			else skip_spaces(1);	/* skip this character */
		}
		if(*prog != '}') error(CL_BRACE_EXPECTED);
	}
	else {	/* skip a single statement */
		while(*prog != ETX) {
			if(*prog == ';') break;
			else if(*prog == ':') return;
			else if(*prog == ')') {
				if(--k == 0) break;
				prog++;
			}
			else if(*prog == '(') { prog++; k++; }
			else if(*prog == '\"' || *prog == '\'') { 	/* skipping over string constants */
				indexed = acc[accN].ind = 0;
				incN(); get_token(); accN--;
			}
			else skip_spaces(1);	/* skip this character */
		}
	}
	token_entry = prog;
	if(*prog != ETX) prog++;	/* skip the closing character ';' or '}' */
	token = SEMICOLON;			/* satisfy the syntax check requirement */
}


/* modify the timer counters and call the handlers when needed */
void execute_timers(void) {
	uint32_t ck = clock();
    uint32_t dt = ((ck >= last_clock) ? (ck - last_clock) : (last_clock - ck));
    last_clock = ck;
	unsigned char t;
    for(t = 0; t < MAX_TIMERS; t++) {
        if(timers[t].reload && timers[t].counter && timers[t].handler) {
            timers[t].counter -= ((timers[t].counter >= dt) ? dt : timers[t].counter);
            if(timers[t].counter == 0) {
				timers[t].counter = timers[t].reload;
				tcode_t k = token;
				char *p = prog;
                execute_function(timers[t].handler);
				prog = p; token = k;
            }
        }
    }
    callback_t *c = callbacks;
    while(c != NULL) { c->call(); c = c->next; }
}


/* execute function (f) */
void execute_function(func_t *f) {
	if(exec_depth++ >= MAX_NESTED) error(MAXIMUM_NESTING);
	func_t *cf_saved = current_f;
	current_f = f;
	char *p1, *p2 = prog;	/* (p1) function definition side, (p2) execution side */
	prog = p1 = (char *) f->name + f->nlen;
	skip_spaces(0);
	if(*prog != '(') error(OP_PAREN_EXPECTED);
	skip_spaces(1);
	char *eptr = ellipsis_ptr;
	char *block_saved = block;
	block = prog;
	if(f->pnum) {	/* the function is expecting some parameters */
		struct {
			var_t *v;
			uint8_t nlen;
		} nlent[MAX_PARAMS];	/* this is the only thing that directs a limit on the number of parameters */
		uint8_t param;
		for(param = 0; param < f->pnum; param++) {	/* get all parameters */
			prototype = get_token();
			if(token != DATA_TYPE) error(DATA_TYPE_EXPECTED);
			var_t *v = new_var(NULL, NULL, 0, 0, 0);
			if(param < (f->pnum - 1)) get_comma();
			p1 = prog; prog = p2;
			if(param) get_comma();
			nlent[param].v = v;
			nlent[param].nlen = v->nlen; v->nlen = 0;	/* make this new variable undiscoverable for now */
			var_set(v, &zero, 1);
			p2 = prog; prog = p1;
		}
		if(f->ellipsis_flag) {	/* more parameters may come here */
			ellipsis_ptr = p2;
			get_comma();
			get_token();		/* skip the '...' operator */
			p1 = prog; prog = p2;
			skip_till_cl_paren();
			if(*prog != ')') error(CL_PAREN_EXPECTED);
			p2 = prog; prog = p1;
		}
		for(param = 0; param < f->pnum; param++) (nlent[param].v)->nlen = nlent[param].nlen;
	}
	else {	/* the function is expecting no parameters */
		get_token();
		if(token == DATA_TYPE && isType(accN, DT_VOID)) token_entry = prog;	/* ignore (void) */
		prog = token_entry;
	}
	p2++;
	skip_spaces(0);
	if(*prog != ')') error(CL_PAREN_EXPECTED);
	prog++;
	execute_block(NULL);			/* execute the function body */
	release_memory(block, NULL, 0);	/* release the local variables */
	if(!return_flag) {
		if((current_f->data.type & DT_MASK) != DT_VOID || current_f->data.ind) error(VALUE_EXPECTED);
	}
	else return_flag = 0;
	ellipsis_ptr = eptr;
	block = block_saved;
	current_f = cf_saved;
	--exec_depth;
	prog = p2;
}


/* execute program block pointed by (*prog) */
/* a program block is either a single statement, or multiple statements enclosed in {} braces */
void execute_block(var_t *parent) {
	wait_break();
	execute_timers();
	skip_spaces(0);
	if(*prog == ';') { token = SEMICOLON; prog++; return; }
	if(*prog == '}') {
		if(exec_depth) { token = UNKNOWN; return; }
		else error(UNEXPECTED_TOKEN);
	}
	while(*prog == '#') { process_directive(); skip_spaces(0); }
	if(*prog != ETX) {
		if(*prog == '{') {	/* a block enclosed in braces */
			prog++;			/* skip the opening brace */
			char *bt = block;
			block = prog;
			do {
				if(goto_point.name) {
					if(block == goto_point.block) { prog = (char *) goto_point.name; memset(&goto_point, 0, sizeof(label_t)); }
					else break;
				}
				execute_block(parent);
				skip_spaces(0);
			} while(*prog != ETX && *prog != '}' && brkcont_flag == BRK_NONE && !return_flag);
			if(brkcont_flag == BRK_NONE) {
				if(*prog != '}' && !return_flag) error(CL_BRACE_EXPECTED);
				token_entry = prog++;	/* skip the closing '}' brace */
			}
			release_memory(block, NULL, 0);
			block = bt;	/* restore the previous block pointer */
		}
		else {			/* single statement */
			do {
				if(goto_point.name) {
					if(block == goto_point.block) { prog = (char *) goto_point.name; memset(&goto_point, 0, sizeof(label_t)); }
					else break;
				}
				if(*prog == ',') prog++;
				execute_statement(parent);
				execute_timers();
				skip_spaces(0);
			} while(*prog == ',' && !return_flag);
			if(token != SEMICOLON) error(SEMICOLON_EXPECTED);
			execute_postmods();
		}
	}
}


/* execute a single statement */
void execute_statement(var_t *parent) {
	vprec = vpost = NULL;
	memset(&acc[accN], 0, sizeof(data_t));
	indexed = 0;
	void *v = get_token();
	if(token == SEMICOLON) {
		var_parent = NULL; parent_addr = NULL;
		return;
	}

	if(token == DATA_TYPE) {
		prototype = v;
		v = get_token();
		skip_spaces(0);
		if(token == FUNCTION) {	/* skip function definitions entirely here */
			func_t *f = (func_t *) v;
			if(f->nlen == 4 && !strncmp(f->name, "main", 4)) execute_function(f);	/* execute function main() */
			prog = (char *) f->after;
			token = SEMICOLON;
		}
		else if(token == IDENTIFIER || *prog == '{') new_var(parent, (var_t *) v, 1, 1, 0);	/* new variable */
		else error(IDENTIFIER_EXPECTED);
	}

	else if(token == LABEL) {	/* label */
		if(v) {
			if(((label_t *) v)->block == block) error(DUPLICATED_LABEL);
		}
		else {
			label_t *l = NULL;
			x_malloc((byte **) &l, sizeof(label_t));
			if(!l) error(MEMORY_ALLOCATION);
			l->name = (const char *) (uintptr_t) ival(accN);
			l->nlen = (ival(accN) ? idlen : 0);
			l->block = block;
			l->next = labels;
			labels = l;
		}
	}

	else if(token > KEYWORD && token < OPERATOR) {	/* C language keywords */
		keyword_t *k = (keyword_t *) v;
		if(k->func) ((void (*)(void)) k->func)();	/* execute the keyword handler */
	}

	else if(v) _get_value(0, v);					/* try it as an expression */
	else if(*prog == '}') { token = SEMICOLON; return; }
	else error(SYNTAX);
}


/* execute and clear the list of post-modifier operations */
void execute_postmods(void) {
	incN();
	while(post_ix) {
		post_ix--;
		memcpy(&acc[accN], &post[post_ix].d, sizeof(data_t));
		uint8_t ixt = indexed;
		indexed = abs(post[post_ix].opr);
		if(isINT(accN)) {
			ival(accN) += (post[post_ix].opr / indexed);
		}
		else if(isFPN(accN)) {
			fval(accN) += (post[post_ix].opr / indexed);
		}
		else error(INVALID_DATA_TYPE);
		indexed--;
		if(post[post_ix].v) var_set(post[post_ix].v, &acc[accN], 0);
		indexed = ixt;
	}
	accN--;
	var_parent = NULL;
	parent_addr = NULL;
}


/* new enum {} variable or type */
void new_enum(void) {
	if(*prog == '{') {		/* define the enum set of values */
		skip_spaces(1);
		data_t dt;
		memset(&dt, 0, sizeof(data_t));
		dt.type = DT_INT;	/* members in the enum are integer constant values (the constant flag will be set later) */
		while(*prog != ETX && *prog != '}') {
			memcpy(&acc[accN], &dt, sizeof(data_t));
			var_t *vt = new_var(NULL, NULL, 0, 0, 0);
			if(initialised) {
				memset(&acc[accN].dim, 0, sizeof(acc[accN].dim));
				acc[accN].ind = 0;
				var_get(vt);
				memcpy(&dt, &acc[accN], sizeof(data_t));
			}
			else {
				memcpy(&acc[accN], &dt, sizeof(data_t));
				var_set(vt, &dt, 0);
			}
			vt->data.type |= FT_CONST;	/* set the constant flag for the new member value */
			skip_spaces(0);
			if(*prog != ',') break;
			skip_spaces(1);
			dt.val.i++;
		}
		if(*prog != '}') error(CL_BRACE_EXPECTED);
		skip_spaces(1);
		token_entry = prog;
	}
	token = SEMICOLON;
}


/* new struct {} or union {} variable or type */
/* the (strf) flag specifies the type of definition: 0 for union, 1 for struct */
void new_struct(var_t *proto, var_t *parent, uint8_t strf) {
	do {
		char *p_stored = prog;
		if(proto) {
			prog = (char *) (uintptr_t) proto->alloc;
			var_t *vp = var_parent;
			var_parent = parent;
			incN();
			execute_block(parent);
			accN--;
			var_parent = vp;
			prog = p_stored;
		}
		var_t *p = vars;
		while(p) {	/* define the member variables for this instance */
			if(p->parent == parent) {
				x_free((byte **) &parent->alloc);
				p->alloc = (uint8_t *) (uintptr_t) ((strf) ? parent->size : 0);	/* all unions members start from offset 0 */
				if(strf) parent->size += p->size;				/* structure */
				else parent->size = max(parent->size, p->size);	/* union */
			}
			p = p->next;
		}
		uint8_t t;
		for(t = 0; t < MAX_DIMENSIONS && parent->data.dim[t]; t++) parent->size *= parent->data.dim[t];
		x_malloc((byte **) &parent->alloc, parent->size);	/* expand the structure to its full size */
		if(!parent) error(MEMORY_ALLOCATION);
		skip_spaces(0);
		if(*prog == ';') break;
		else if(*prog == ',') skip_spaces(1);
		else error(SYNTAX);
	} while(1);
	token_entry = prog;
	token = SEMICOLON;
}


/* initialise value(s) to a variable */
/* uses indexes as provided in (ix->dim[]); the (ix) structure is modified during the execution */
/* (ixx) is internal and must be 0 on the first call */
void var_init(var_t *v, data_t *ix, uint8_t ixx) {
	if(v->data.dim[ixx]) {
		skip_spaces(0);
		if(*prog != '{') {
			if(v->alloc) x_free((byte **) &v->alloc);
			get_value(1);
			v->data.val.i = acc[accN].val.i;
			return;
		}
		skip_spaces(1);	/* skip the '{' character */
	}
	uint8_t strf = 0;
	for(ix->dim[ixx] = 0; (!ixx && !ix->dim[ixx]) || (ix->dim[ixx] < v->data.dim[ixx]); ix->dim[ixx]++) {
		if(ixx < (MAX_DIMENSIONS - 1) && v->data.dim[ixx + 1]) {
			uint8_t t;
			for(t = ixx + 1; t < MAX_DIMENSIONS; t++) ix->dim[t] = 0;
			var_init(v, ix, ixx + 1);
			skip_spaces(0);
			if(*prog != ',') break;
			prog++;
			continue;
		}
		skip_spaces(0);
		strf = 0;
		if(*prog != '\"') var_set(v, ix, 1);
		else {	/* initialise with a string constant */
			if(v->data.ind > 1) error(INVALID_ASSIGNMENT);
			if(v->data.ind) {
				char *p = prog;
				get_value(1);	/* this will always be a string */
				prog = p;
				p = NULL;
				x_malloc((byte **) &p, (str_len + 1));	/* allocate memory for the string */
				if(!p) error(MEMORY_ALLOCATION);
				included_libs_t *l = NULL;
				x_malloc((byte **) &l, sizeof(included_libs_t));	/* allocate new list element */
				if(!l) error(MEMORY_ALLOCATION);
				l->next = included_libs; included_libs = l;
				l->code = p;
				v->alloc = (uint8_t *) (uintptr_t) p;
				v->size = (str_len + 1);
				char c = *(prog++);
				do {
					c = get_char(&prog);
					*(p++) = c;
				} while(c);
			}
			else {
				acc[accN].type = DT_CHAR;
				acc[accN].ind = 0;
				char c = *(prog++);
				do {
					c = get_char(&prog);
					ival(accN) = c;
					var_set(v, ix, 0);
					ix->dim[ixx]++;
				} while(c);
				strf = 1;
			}
		}
		skip_spaces(0);
		if(strf || *prog != ',' || !v->data.dim[ixx]) break;
		skip_spaces(1);
	}
	if(v->data.dim[ixx] && !strf) skip_spaces(1);	/* skip the closing '}' */
}


/* declare and initialise new variable(s) */
/* the type is passed in acc[accN] */
var_t *new_var(var_t *parent, var_t *v, uint8_t id_preloaded, uint8_t allow_more, uint8_t make_type) {
	initialised = 0;
	if(v && v->block != block) v = NULL;
	data_t dt;
	if(prototype) {
		memcpy(&dt, &prototype->data, sizeof(data_t));
		uint8_t t, r;
		for(r = 0; r < MAX_DIMENSIONS && dt.dim[r]; r++);
		for(t = 0; t < MAX_DIMENSIONS && acc[accN].dim[t]; t++) dt.dim[r++] = acc[accN].dim[t];
	}
	else memcpy(&dt, &acc[accN], sizeof(data_t));
	do {

		if(!id_preloaded) {
			skip_spaces(0);
			while(*prog == '*') { acc[accN].ind++; skip_spaces(1); }
			var_t *vp = var_parent; var_parent = parent;
			v = (var_t *) get_token();
			var_parent = vp;
		}
		uint8_t is_complex = (isType(accN, DT_ENUM) || isType(accN, DT_STRUCT) || isType(accN, DT_UNION));
		if(token != IDENTIFIER && !is_complex) error(IDENTIFIER_EXPECTED);
		make_type |= (is_complex && *prog == '{');	/* definition of a complex data type follows */
		if(v && v->depth < exec_depth) v = NULL;

		if(!v) {	/* the variable is entirely new (not known before) */
			x_malloc((byte **) &v, sizeof(var_t));
			if(!v) error(MEMORY_ALLOCATION);
			memcpy(&v->data, &acc[accN], sizeof(data_t));
			v->name = (const char *) (uintptr_t) ival(accN);
			v->nlen = (ival(accN) ? idlen : 0);
			v->block = block;
			v->depth = exec_depth;
			v->size = ((v->data.ind) ? dt_size[DT_VOID] : dt_size[v->data.type & DT_MASK]);
			v->parent = parent;
			uint8_t t;
			for(t = 0; t < MAX_DIMENSIONS && v->data.dim[t]; t++) v->size *= v->data.dim[t];
			v->data.val.i = 0;
			v->alloc = NULL;

			if(make_type) {
				v->next = types; types = v;
				if(v->data.type == DT_ENUM) new_enum();
				else if(v->data.type == DT_STRUCT || v->data.type == DT_UNION) {
					skip_spaces(0);
					if(*prog == '{') {	/* define the members of the structure */
						v->alloc = (uint8_t *) (uintptr_t) prog;	/* record the start of the member definitions */
						skip_block();
						if(*(prog - 1) != '}') error(CL_BRACE_EXPECTED);
					}
				}
				var_t *pro = prototype; prototype = v;
				memcpy(&dt, &v->data, sizeof(data_t));
				skip_spaces(0);
				while(*prog != ';' && *prog != ETX) {	/* define instance variables of this new type */
					memcpy(&acc[accN], &dt, sizeof(data_t));
					new_var(parent, v, 0, 0, 0);
					skip_spaces(0);
					if(*prog == ';') break;
					else if(*prog == ',') skip_spaces(1);
					else error(SYNTAX);
				}
				prototype = pro;
			}

			else {	/* normal variables */
				v->next = vars; vars = v;
				if(v->data.type == DT_ENUM) v->data.type = DT_INT;	/* enum values are int */
				else if(v->data.ind == 0 && (v->data.type == DT_STRUCT || v->data.type == DT_UNION)) {
					if(make_type) v->size = 0;	/* prevent allocation of memory as it is already done in new_struct() */
					else {
						memcpy(&v->data, &dt, sizeof(data_t));
						new_struct(prototype, v, (v->data.type == DT_STRUCT));
					}
				}
				if(v->size && v->data.dim[0]) {
					x_malloc((byte **) &v->alloc, v->size);		/* allocate memory block for the actual data */
					if(!v->alloc) error(MEMORY_ALLOCATION);
				}
				skip_spaces(0);
				if(*prog == '=') {	/* setting values during allocation */
					prog++;
					data_t z;
					memset(&z, 0, sizeof(data_t));
					if(prog_end) var_init(v, &z, 0);
					else skip_block();
					initialised = 1;
				}
			}

			if(!allow_more) break;
			memcpy(&dtype_dim, &dtype_dim_saved, sizeof(dtype_dim));
		}

		else if(v->data.type & FT_STATIC) {		/* the variable is already known (static?) */
			skip_spaces(0);
			if(!allow_more) break;
			if(*prog == '=') {	/* skipping the initialisation part */
				prog++;
				skip_block();
			}
		}

		else error(DUPLICATED_VARIABLE);
		skip_spaces(0);
		if(*prog != ',') break;
		prog++;	/* more variables of the same type */
		id_preloaded = 0;
		memcpy(&acc[accN], &dt, sizeof(data_t));
		acc[accN].ind = 0;	/* will get the indirection level for each variable individually */

	} while(1);

	memset(&dtype_dim, 0, sizeof(dtype_dim));
	token = SEMICOLON;	/* satisfy the syntax check requirement */
	return v;
}


/* get value into acc[accN] from a variable */
/* uses indexes as provided in acc[accN] on entry */
void var_get(var_t *v) {
	uint8_t dims, *p = calcAddr(v, &acc[accN], &dims);
	memset(&acc[accN].val, 0, sizeof(acc[accN].val));
	acc[accN].type = v->data.type;
	if((acc[accN].ind + indexed) < dims) {
		if(!v->alloc) p = *((byte **) p);
		ival(accN) = (uintptr_t) p;
		return;
	}
	switch(v->data.type & DT_MASK) {
		case DT_BOOL:
			acc[accN].val.i = *((uint8_t *) p);
			break;
		case DT_CHAR:
			if(v->data.type & FT_UNSIGNED) acc[accN].val.i = *((uint8_t *) p);
			else acc[accN].val.i = *((int8_t *) p);
			break;
		case DT_SHORT:
			if(v->data.type & FT_UNSIGNED) acc[accN].val.i = *((uint16_t *) p);
			else acc[accN].val.i = *((int16_t *) p);
			break;
		case DT_INT:
			if(v->data.type & FT_UNSIGNED) acc[accN].val.i = *((uint_t *) p);
			else acc[accN].val.i = *((int_t *) p);
			break;
		case DT_LONG:
			if(v->data.type & FT_UNSIGNED) acc[accN].val.i = *((uint32_t *) p);
			else acc[accN].val.i = *((int32_t *) p);
			break;
		case DT_LLONG:
			if(v->data.type & FT_UNSIGNED) acc[accN].val.i = *((uint64_t *) p);
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


/* set value acc[accN] to a variable */
/* uses indexes as provided in (ix->dim[]) */
void var_set(var_t *v, data_t *ix, uint8_t getval_flag) {
	if(getval_flag) {
		uint8_t ixt = indexed;
		get_value(1);
		indexed = ixt;
		ix->ind = acc[accN].ind;
		ix->type = acc[accN].type;
	}
	uint8_t dims, *p = calcAddr(v, ix, &dims);
	if(v->data.ind) {
		if(ix->ind) {
			if(acc[accN].type & FT_CONST) error(WRITING_CONST);
		}
		else if(acc[accN].type & FT_CONSTPTR) error(WRITING_CONST_POINTER);
	}
	if((acc[accN].ind + indexed) < dims) {
		*((uint8_t **) p) = (uint8_t *) (uintptr_t) ival(accN);
		return;
	}
	convert(&acc[accN], &v->data);
	switch(v->data.type & DT_MASK) {
		case DT_BOOL:
			*((uint8_t *) p) = (uint8_t) ival(accN);
			break;
		case DT_CHAR:
			if(v->data.type & FT_UNSIGNED) *((uint8_t *) p) = (uint8_t) ival(accN);
			else *((int8_t *) p) = (int8_t) ival(accN);
			break;
		case DT_SHORT:
			if(v->data.type & FT_UNSIGNED) *((uint16_t *) p) = (uint16_t) ival(accN);
			else *((int16_t *) p) = (int16_t) ival(accN);
			break;
		case DT_INT:
			if(v->data.type & FT_UNSIGNED) *((uint_t *) p) = (uint_t) ival(accN);
			else *((int_t *) p) = (int_t) ival(accN);
			break;
		case DT_LONG:
			if(v->data.type & FT_UNSIGNED) *((uint32_t *) p) = (uint32_t) ival(accN);
			else *((int32_t *) p) = (int32_t) ival(accN);
			break;
		case DT_LLONG:
			if(v->data.type & FT_UNSIGNED) *((uint64_t *) p) = (uint64_t) ival(accN);
			else *((int64_t *) p) = (int64_t) ival(accN);
			break;
		case DT_FLOAT:
			*((float *) p) = (float) fval(accN);
			break;
		case DT_DOUBLE:
			*((double *) p) = (double) fval(accN);
			break;
		case DT_LDOUBLE:
			*((ldouble_t *) p) = (ldouble_t) fval(accN);
			break;
		case DT_ENUM:
			*((int_t *) p) = (int_t) ival(accN);
			break;
		case DT_VA_LIST:
		case DT_FILE:
			*((byte **) p) = (byte *) (uintptr_t) ival(accN);
			break;
		default:
			error(INVALID_ASSIGNMENT);
	}
}


/* calculate address to physical data in a variable whose indirection level and index are provided in (*ix) */
uint8_t *calcAddr(var_t *v, data_t *ix, uint8_t *dims) {
	int16_t i = ix->ind;
	if(i > v->data.ind) {
		indexed++;
		if(--i > v->data.ind) error(INVALID_POINTER);
	}
	uint8_t *p;
	if(v->alloc || v->parent) { p = (uint8_t *) (uintptr_t) v->alloc; i--; }
	else if(indexed) p = (uint8_t *) (uintptr_t) v->data.val.i;
	else p = (uint8_t *) (uintptr_t) &v->data.val;
	p += (uintptr_t) parent_addr;
	for( ; i > 0; i--) p = *((byte **) p);
	int32_t xt;
	uint8_t t, r;
	*dims = !!v->data.ind;
	for(t = 0; t < MAX_DIMENSIONS && v->data.dim[t]; t++) (*dims)++;	/* count the variable's dimensions */
	for(t = 0; t < *dims; t++) {
		xt = 1;
		for(r = t + 1; r < *dims; r++) xt *= v->data.dim[r];
		if((ix->ind + indexed) < *dims) p += (dt_size[DT_VOID] * xt * ix->dim[t]);
		else {
			if(dt_size[v->data.type & DT_MASK]) p += (dt_size[v->data.type & DT_MASK] * xt * ix->dim[t]);
			else {		/* calculate the length of a single struct or union */
				uint32_t pl = 1;
				for(r = 0; r < *dims; r++) pl *= v->data.dim[r];
				p += ((v->size / pl) * xt * ix->dim[t]);
			}
		}
	}
	return p;
}

#endif
