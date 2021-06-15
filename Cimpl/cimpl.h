/*
 * --------------------------------------------------------------------------------------
 *
 * C.impl Implantable C interpreter
 * written by Konstantin Dimitrov, (C) 2020-2021
 *
 * --------------------------------------------------------------------------------------
 *  LICENSE INFORMATION
 *   1. This code is "linkware". It is free for use, modification, and distribution
 *      without any limitations. The only requirement is to link with the author on
 *      LinkedIn: https://www.linkedin.com/in/knivd/ or follow on Twitter: @knivd
 *   2. The author assumes no responsibility for any loss or damage caused by this code
 * --------------------------------------------------------------------------------------
*/

/* ======================================================================================

Written and debugged with Dev-C++ v5.11
NOTE: for best source readability, please use a monospace font (eg. Consolas)

The aim of this code is to achieve a close C90 specification functional implementation
of an interpreter for the C language with minimum required system resources and without
references to specific external layers such as OS.

Data types:

'bool'                          - defined as 1-bit integer
'char'                          - defined as 8-bit integer
'short' or 'short int'          - defined as 16-bit integer
'int'                           - defined as per the 'typedef int_t' statement
'long' or 'long int'            - defined as 32-bit integer
'long long' or 'long long int'  - defined as 64-bit integer
'float'                         - single precision floating point number (32-bit)
'double'                        - double precision floating point number (64-bit)
'long double'                   - quadruple precision floating point number (80-bit or more)

In addition to the basic types above, several other data types are also supported:

'size_t'                        - natively represented as 'unsigned int'
'enum'                          - standard C-type enum list
'struct'                        - standard C-type data structure
'union'                         - standard C-type data union
'va_list'                       - used in conjunction with the <stdarg.h> library for variable argument lists
'FILE'                          - used with file functions in the <stdio.h> library

'signed' or 'unsigned' optionally preceding the integer types, except 'bool'
All integer types are signed by default (i.e. as if preceded by a 'signed' declaration)

Constant data types:

'char'      - single 8-bit character
"string"    - string of zero-terminated 8-bit characters
up to 64-bit decimal integer numbers (optionally preceded by '0d' or '0D')
up to 64-bit hexadecimal unsigned numbers; preceded by '0x' or '0X'
up to 64-bit binary unsigned numbers; preceded by '0b' or '0B'
up to 64-bit octal unsigned numbers; preceded by '0'
floating point numbers (always decimal) in format:
    [sign] [ iii [ .fff ['E/e' [sign]  [ eee [ .xxx ] ] ] ] ]

Decimal numbers optionally support suffix specifiers for sign and/or size:
'u' or 'U' for unsigned
'l' or 'L' for long integer or long double
'll' or 'LL' for long long integer
'f' or 'F' for float
'd' or 'D' for double

The default type of all integer constants, unless explicitly specified, is 'signed int'
The default type of floating point number constants, unless explicitly specified, is 'float'

Pre-defined character literals:

'\0'	- ASCII code 0 (character NUL)
'\a'    - ASCII code 7 (alarm)
'\b'    - ASCII code 8 (backspace)
'\e'    - ASCII code 27 (escape)
'\f'    - ASCII code 12 (form feed)
'\n'    - ASCII code 10 (new line)
'\r'    - ASCII code 13 (carriage return)
'\t'    - ASCII code 9 (horizontal tabulation)
'\\'    - the character 'backslash'
'\''    - the character 'single quote'
'\"'    - the character 'double quote'
'\?'    - the character 'question mark'

Other notes and features:

1. True execution in place. The source code is not compiled or modified and can run directly from ROM
2. Fully dynamic memory model with no limitation on the number of functions or variables, or their size
3. Arrays and pointers are fully supported
4. Inline local variable definitions are allowed, eg. for(int t=0; .....
5. String literals can be split on to multiple lines
6. Pointer to constant (eg. const char *) and constant pointer (eg. char const *), are both supported
7. Standard C and C++ style commentaries are both supported

Built-in libraries:

<stdlib.h>      // extra: unsigned long BIT(unsigned char n)
<stdio.h>       // extra: void run(const char *filename)
<stdint.h>
<stdbool.h>
<stdarg.h>
<string.h>
<math.h>
<limits.h>
<ctype.h>
<time.h>
<assert.h>		// can be used also to assert the existence of library functions, eg. assert(printf())
<conio.h>       // console functions
<fatfs.h>		// wrapper functions for most of the FatFs library
<graphics.h>    // hardware-independent higher-level graphical primitives
<platform.h>	// support library for specific hardware platform implementations

Known issues and limitations:

1. Function pointers and function address dereferencing are not supported (no feasible way in an interpreter!)
2. Support for preprocessor directives is reduced only to #include
3. In for(), while(), and do...while() loops, 'break' and 'continue' can only work from within a {} block
4. The 'goto' instruction only allows jumping to a location at the same or upper {} depth level
5. A goto label must have unique name among all labels globally.
6. The 'extern' keyword is not supported since the source can be only a single file
7. In the ternary operator ?: there must be a space before the colon ':' so it is not mistaken for a label
8. The main() function must not have parameters or a return value
9. In composite data types (enum, struct, union), the name of the type must be before the {} block, but can't be after
10. Structures, unions, and enums are all supported but have high runtime memory cost and should be used with care
11. Keywords 'struct', 'union', and 'enum' must be used only for the initial type declaration, but not for instances
12. Casting to structure or union pointer will not work. The cost of its implementation in an interpreter
    is massive in terms of both runtime memory and execution speed
13. Indexing by pointers (eg. int a[10]; int b = *(a+1); ) will not work. Indexing needs to be done in the brackets way

Other notes:

The entire code is deeply recursive so it requires sufficient stack for its operation. With the
default settings, at least 4kB stack size is recommended.

=========================================================================================== */

#ifdef CIMPL
#ifndef CIMPL_H
#define CIMPL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <conio.h>

#ifndef BIT
#define BIT(b) (1ul << (b))
#endif

/* =========================================================================================== */
/* configuration options */

/* int_t and uint_t definitions */
typedef int32_t int_t;
typedef uint32_t uint_t;

#define DISABLE_ERROR_TEXT  0   /* non zero disables the text error messages */
#define DISABLE_LONG_LONG   0   /* control the use of 'long long int' data type */
                                /* value 0 enables it
                                   value >0 disables it and generates an error when used
                                   value <0 disables it without an error but equal to 'long int' */
#define DISABLE_LONG_DOUBLE 1   /* control the use of 'long double' data type */
                                /* value 0 enables it
                                   value >0 disables it and generates an error when used
                                   value <0 disables it without an error but equal to 'double' */
#define DISABLE_FILE        0   /* disable data type FILE and inclusion of file operations in <stdio.h> */
                                /* value 0 enables files; any other value disables them */
#define MAX_FILES           5   /* maximum number of open file handlers (meaningful only if DISABLE_FILE == 0 */
#define MAX_ID_LENGTH       25  /* maximum allowed length for program identifiers (must be less than 255) */
#define MAX_DIMENSIONS      5   /* maximum number of dimensions for variable */
#define MAX_PARAMS			20	/* maximum number of parameters in a function */
#define MAX_TERMS          	35	/* maximum number of nested terms (stack size must be large with greater depths) */
#define MAX_NESTED			10	/* maximum level of nesting in loops */
#define MAX_TIMERS          5   /* maximum number of simultaneous working internal timers */

#if DISABLE_LONG_LONG == 0
    typedef int64_t llong_t;
    typedef uint64_t ullong_t;
#else
    typedef int32_t llong_t;
    typedef uint32_t ullong_t;
#endif

#if DISABLE_LONG_DOUBLE == 0
    typedef long double ldouble_t;
#else
    typedef double ldouble_t;
#endif

#define ETX         ((char) 3)          /* ASCII code of the End-Of-Text character */
#define ETXSTR		"\x03\x03\x03\x03"	/* the same ETX character but duplicated 4 times and as string (for the libraries) */

/* main exported function: process program source and return error code (0 for successful execution) */
int Cimpl(const char *source);

/* =========================================================================================== */

#define DT_VOID     0x00    /* void (NOTE: must be the 0 - the lowest id of all valid data types) */
#define DT_BOOL     0x01    /* bool */
#define DT_CHAR     0x02    /* char */
#define DT_SHORT    0x03    /* short int */
#define DT_INT      0x04    /* int (the built-in type 'size_t' is equivalent to 'unsigned int' */
#define DT_LONG     0x05    /* long int */
#define DT_LLONG    0x06    /* long long int */
#define DT_FLOAT    0x07    /* float */
#define DT_DOUBLE   0x08    /* double */
#define DT_LDOUBLE  0x09    /* long double */
#define DT_RESERVED	0x0A
#define DT_FILE     0x0B    /* FILE */
#define DT_ENUM     0x0C    /* enum */
#define DT_STRUCT   0x0D    /* struct */
#define DT_UNION    0x0E    /* union */
#define DT_VA_LIST  0x0F    /* va_list (NOTE: must be the highest id of all valid data types) */

#define FT_SIGNED 	0x100   /* bitmask flag for signed value (unchanged mask since this is the dafault mode) */
#define FT_UNSIGNED 0x10    /* bitmask flag for unsigned value */
#define FT_CONST    0x20    /* bitmask flag for data constant */
#define FT_CONSTPTR 0x40    /* bitmask flag for constant (unmutable) pointer */
#define FT_STATIC   0x80    /* bitmask flag for static variable or 'weak' function */

#define DT_MASK     0x0F    /* bitmask for extracting the data type only */
#define FT_MASK     0xF0    /* bitmask for extracting the modifier flags */

#define DT_SIZE_T   (FT_UNSIGNED | DT_INT)

/* token codes used by the parser */
typedef enum {

	UNKNOWN = 0, SEMICOLON, NUMBER, STRING, LABEL, FUNCTION, SYS_FUNCTION, IDENTIFIER, DATA_TYPE,

	KEYWORD,	/* only used to mark the start of keywords */
    TYPEDEF, IF, ELSE, FOR, DO, WHILE, BREAK, CONTINUE, SWITCH, CASE, DEFAULT, RETURN, GOTO,

	OPERATOR,	/* only used to mark the start of operators */
	/* the operators below are sorted by increasing priority according to the standard C conventions */
    COMMA, ELLIPSIS, ATTRIBUTION,							/*  ,  ...  =  */
    BITWISE_OR_ATTR, BITWISE_XOR_ATTR, BITWISE_AND_ATTR,	/*  |=  ^=  &=  */
    BITWISE_SHIFTR_ATTR, BITWISE_SHIFTL_ATTR,				/*  >>=  <<=  */
	BITWISE_NOT_ATTR,										/*  ~= */
    PLUS_ATTR, MINUS_ATTR, MULTIPLICATION_ATTR,				/*  +=  -=  *=  */
    MODULO_ATTR, DIVISION_ATTR, COLON, QUESTION,			/*  %=  /=  :  ?  */
    LOGICAL_OR, LOGICAL_AND,								/*  ||  &&  */
    BITWISE_OR, BITWISE_XOR, BITWISE_AND,					/*  |  ^  &  */
    NOT_EQUAL, EQUAL,										/*  !=  ==  */
    GREATER_OR_EQUAL, SMALLER_OR_EQUAL, GREATER, SMALLER,	/*  >=  <=  >  <  */
    BITWISE_SHIFTR, BITWISE_SHIFTL,							/*  >>  <<  */
    PLUS, MINUS, MULTIPLICATION, MODULO, DIVISION,			/*  +  -  *  %  /  */
    CAST, SIZEOF, POINTER, ADDRESS,							/*  (type)  sizeof  *  &  */
    UNARY_MINUS, UNARY_PLUS,								/*  -  +  */
    LOGICAL_NOT, BITWISE_NOT,								/*  !  ~  */
    DECREMENT, INCREMENT,									/*  --  ++  */
    STRUCT_ARROW, STRUCT_DOT								/*  ->  .  */

} tcode_t;

/* error codes */
typedef enum {
	TERMINATED = -1,
	OK = 0,
	SYNTAX,
	UNEXPECTED_END,
	INVALID_NUMBER,
	INVALID_CHARACTER,
	ID_TOO_LONG,
	DATA_TYPE_EXPECTED,
	VALUE_EXPECTED,
	IDENTIFIER_EXPECTED,
	SEMICOLON_EXPECTED,
	COLON_EXPECTED,
	DQUOTE_EXPECTED,
	COMMA_EXPECTED,
	OP_PAREN_EXPECTED,
	CL_PAREN_EXPECTED,
	OP_BRACE_EXPECTED,
	CL_BRACE_EXPECTED,
	OP_BRACKET_EXPECTED,
	CL_BRACKET_EXPECTED,
	OP_BRACE_OR_QUOTE_EXPECTED,
	CL_BRACE_OR_QUOTE_EXPECTED,
	CASE_OT_DEFAULT_EXPECTED,
	WHILE_EXPECTED,
	UNEXPECTED_ELSE,
	UNEXPECTED_CASE,
	UNEXPECTED_DEFAULT,
	UNEXPECTED_BREAK,
	UNEXPECTED_CONTINUE,
	UNEXPECTED_TOKEN,
	MEMORY_ALLOCATION,
	DUPLICATED_FUNCTION,
	DUPLICATED_VARIABLE,
	DUPLICATED_LABEL,
	DUPLICATED_DEFAULT,
	DUPLICATED_ENUM,
	TOO_COMPLEX,
	MAXIMUM_NESTING,
	DIVISION_BY_ZERO,
	IMPOSSIBLE_CONVERSION,
	INVALID_DATA_TYPE,
	NOT_ALLOWED_WITH_PTR,
	TOO_MANY_DIMENSIONS,
	TOO_MANY_PARAMETERS,
	TOO_MANY_POSTMODS,
	UNKNOWN_IDENTIFIER,
	INVALID_ASSIGNMENT,
	INVALID_POINTER,
	WRITING_CONST_POINTER,
	WRITING_CONST,
	UNKNOWN_DIRECTIVE,
	UNKNOWN_LIBRARY,
	UNKNOWN_PRAGMA,
	UNKNOWN_LABEL,
	INVALID_ELLIPSIS,
	UNABLE_TO_INCLUDE,
	ASSERTION_FAILED,
    INSUFFICIENT_RESOURCE
} error_t;

/* print error code and perform full exit */
void error(error_t e);

typedef struct {
	error_t e;	/* associated error code */
	char *s;	/* error message as text */
} error_msg_t;

typedef struct {
	uint8_t p;	/* priority level (higher number = greater priority) */
	tcode_t t;	/* associated token code */
    char *s;	/* keyword as text */
    char l;		/* keyword length in number of characters; needed for optimising the execution speed */
    void (*func)(void);	/* function handler; parameter is the operator's own token code */
} keyword_t;

/* universal data type and value container */
typedef struct {
	union {			/* data value container */
		llong_t i;	/* this field is also used as pointer or offset (in structures) to actual data */
		double f;
	} val;
	uint8_t type;	/* the low 4 bits store data type as defined in the DT_xxx constants */
                    /* the high four bits are reserved for modifier flags as defined in the FT_xxx constants */
	uint8_t ind;	/* indirection level for pointers */
	int_t dim[MAX_DIMENSIONS];	/* array dimensions */
} data_t;

/* token structure */
typedef struct {
    data_t data;
    tcode_t code;
} token_t;

/* goto label structure */
typedef struct _label_t {
    const char *name;	/* entry point of the label (start of the name) */
    uint8_t nlen;	/* length of the text name in characters (including the trailing ':' character) */
	char *block;	/* pointer to the opening block in which this variable is located */
    struct _label_t *next;  /* pointer to next label in the list */
} label_t;

/* variable structure; also used for new data types */
typedef struct _var_t {
    data_t data;	/* data type, allocation pointer and dimensions */
					/* when used as data prototype, (.val.i) points to the type structure */
					/* in type structures, (.val.i) points to the start of the definition block */
    const char *name;		/* text name of the function */
    uint8_t nlen;	/* length of the text name in characters */
	uint32_t size;	/* total data size in bytes */
	uint8_t *alloc;	/* allocated memory block or offset from the start of a structure */
	char *block;	/* pointer to the opening block in which this variable is located */
	uint16_t depth;	/* nesting block depth of the variable */
    struct _var_t *parent; 	/* parental variable in structures */
    struct _var_t *next;	/* pointer to next variable in the definitions */
} var_t;

/* function structure */
typedef struct _func_t {
    data_t data;	/* return data type (in .type and .ind) */
    const char *name;		/* text name of the function */
    uint8_t nlen;	/* length of the text name in characters */
    uint8_t pnum;	/* number of expected parameters */
	uint8_t ellipsis_flag;	/* indicates that the function will expect an unknown number of parameters */
	const char *after;		/* points to the first character after the function body */
    struct _func_t *next;  	/* next function in the definitions */
} func_t;

/* system library function structure */
typedef struct _sys_func_t {
    void (*fn)(void);   /* pointer to the function handler */
    char *name;         /* text name of the function */
    uint8_t nlen;       /* length of the text name in characters */
	char *help;         /* additional help information in the form of an encoded string */
                        /* v (void), B (BOOL), c (char), s (short), i (integer), l (long), L (long long) */
                        /* f (float), d (double), D (long double), F (FILE), z (size_t) */
                        /* C (const), u (unsigned), * (*), . (...), t (time_t), m (struct tm) */
                        /* individual descriptions are separated by ','; the first description is the function result */
                        /* no other characters (even spaces) are allowed in the description string */
                        /* if the string starts with ':' the rest of the string is free format and not interpreted */
                        /* example: "uL*,d*,Ci" describes a function (let's assume its name is F): */
                        /*                                   unsigned long long *F(double *, const int) */
    struct _sys_func_t *next;   /* next system function in list of definitions (default NULL, updated in execution) */
} sys_func_t;

/* system library constant structure */
typedef struct _const_t {
    data_t data;	/* data container */
    char *name;		/* text name of the constant */
    uint8_t nlen;	/* length of the text name in characters */
} sys_const_t;

/* system library definition */
typedef struct {
    const char *name;   /* library name, eg. "<stdlib.h>" */
    const sys_const_t *const_table; /* library constants */
    const sys_func_t *func_table;   /* library functions */
    const char *src;    /* source to be executed during the process of installation */
    void (*init)(void);	/* optional initialisation code */
} system_library_t;

/* system libraries */
extern const system_library_t system_libs[];

/* system callback drivers */
typedef struct _callback_t {
    void (*call)(void);         /* function handler */
    struct _callback_t *next;   /* pointer to the next handler */
} callback_t;

void wait_break(void);
void convert(data_t *d, data_t *mod);
void get_value(uint8_t force_single);	/* parameter normally (0) except cases when a following comma is not allowed */
void _get_value(tcode_t topr, void *vpre);	/* DO NOT USE IN EXTERNAL FUNCTIONS! use get_value() instead */
void *get_token(void);
char get_char(char **src);
void get_param(data_t *dt, uint8_t data_type, uint8_t ind);
var_t *new_var(var_t *parent, var_t *v, uint8_t id_preloaded, uint8_t allow_more, uint8_t make_type);
void new_enum(void);
void new_struct(var_t *proto, var_t *parent, uint8_t strf);
uint8_t *calcAddr(var_t *v, data_t *ix, uint8_t *dims);
void var_init(var_t *v, data_t *ix, uint8_t ixx);
void var_get(var_t *v);
void var_set(var_t *v, data_t *ix, uint8_t getval_flag);
void release_memory(char *blk, var_t *pnt, uint8_t level);
void execute_statement(var_t *parent);
void execute_postmods(void);
void execute_timers(void);
void execute_block(var_t *parent);
void skip_block(void);
void execute_function(func_t *f);
void skip_spaces(uint8_t skip_chr);
void install_sys_library(const system_library_t *lib);
void process_directive(void);

extern char *file_to_run;   /* file name to be executed */

char *prog;				/* main program pointer during execution */
char *token_entry;		/* entry point in source of the current token */
char *block;			/* pointer to the start of the currently executed block */
uint8_t dt_size[16];	/* pre-loaded table with data size for the single types (only) */
jmp_buf err_env;		/* stores the environment return point in case of abnormal exit due to errors */
tcode_t token;			/* current token code */
data_t acc[MAX_TERMS];	/* data accumulators */
uint8_t accN;			/* currently used data accumulator */
uint8_t idlen;			/* used to return the length of the last identifier */
uint8_t paren_depth;	/* parenthesis depth */
int16_t exit_depth;		/* force exit at specific parenthesis depth; used in while() */
uint32_t str_len;		/* get_token() returns here the length of a recognised string constant */
label_t *labels;		/* pointer to the entry of the labels list */
var_t *vars;			/* pointer to the entry of the variables list */
var_t *types;			/* pointer to the entry of the data types list */
func_t *funcs;			/* pointer to the entry of the functions list */
sys_func_t *sys_funcs;	/* pointer to the entry of the system functions list (loaded from libraries) */
func_t *current_f;		/* currently executed function */
uint8_t return_flag;	/* flag indicating return from function */
uint8_t exec_depth;		/* execution stack depth */
data_t zero;			/* just a data structure which is entirely zeroed */
data_t d1, d2, d3, d4;	/* common data accumulators used in the library functions */
uint8_t indexed;		/* flag set by get_token() to indicate when a variable has been [] indexed */
uint8_t indexed_attr;	/* flag set by get_token() to indicate when a variable has been [] indexed */
char *ellipsis_ptr;		/* pointer to the first additional parameter */
var_t *var_parent;		/* used by get_token() to properly address variables with matching names from different structures */
uint8_t *parent_addr;	/* address correction for the parent variable */
var_t *prototype;		/* data type pointer for user-defined types */
uint8_t assert_flag;	/* flag indicating that if an error happens it will be assertion failure */
callback_t *callbacks;  /* additionally installed callback functions */

#define STR_BUFFERS	4
char *strbuf[STR_BUFFERS];	/* common string buffers */
uint8_t curr_strbuf;		/* currently used string buffer */

/* post-modifier structures */
#define MAX_POST	10
struct {
	data_t d;
	var_t *v;
	int8_t opr;		/* +1 or -1 multiplied by (indexed+1) */
} post[MAX_POST];
uint8_t post_ix;	/* number of active elements in the list */

#define isspaceC(c) (((c) != ETX && (c) <= ' ') || (c) == 0x7F)
#define isoprC(c) (strchr("*+-/%=<>&|^~!?(.", (c)))
#define isdigitC(c) ((c) >= '0' && (c) <= '9')
#define isXdigitC(c) (isdigitC(c) || ((c) >= 'a' && (c) <= 'f') || ((c) >= 'A' && (c) <= 'F'))
#define isidchr1C(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z') || (c) == '_')
#define isidchrC(c) (isidchr1C(c) || isdigitC(c))

#define isINT_t(c) (((c) & DT_MASK) >= DT_BOOL && ((c) & DT_MASK) <= DT_LLONG)
#define isFPN_t(c) (((c) & DT_MASK) >= DT_FLOAT && ((c) & DT_MASK) <= DT_LDOUBLE)
#define istype_t(c, type) (((c) & DT_MASK) == (type))

#define isINT(ix) (((acc[ix].type & DT_MASK) >= DT_BOOL && (acc[ix].type & DT_MASK) <= DT_LLONG) || acc[ix].ind)
#define isFPN(ix) ((acc[ix].type & DT_MASK) >= DT_FLOAT && (acc[ix].type & DT_MASK) <= DT_LDOUBLE)
#define isNumeric(ix) (((acc[ix].type & DT_MASK) >= DT_BOOL && (acc[ix].type & DT_MASK) <= DT_LDOUBLE) || acc[ix].ind)
#define isType(ix, dt_type) ((acc[ix].type & DT_MASK) == (dt_type))

#define ival(ix) acc[ix].val.i
#define fval(ix) acc[ix].val.f

#define incN() { if(++accN >= MAX_TERMS) error(TOO_COMPLEX); indexed = acc[accN].ind = 0; }

#define get_comma() { skip_spaces(0); if(*prog == ',') prog++; else error(COMMA_EXPECTED); }

/* call timers structure */
struct {
    uint32_t counter;   /* this counter gets decreased every millisecond */
    uint32_t reload;    /* reload value for the counter */
    func_t *handler;    /* function name to be called when the counter hits 0 */
} timers[MAX_TIMERS];

#ifdef __cplusplus
}
#endif

#endif  /* CIMPL_H */
#endif
