# C.impl

C.impl is a small portable C interpreter integrated with a line text editor.

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
<assert.h>      // can be used also to assert the existence of library functions, eg. assert(printf())
<conio.h>       // console functions
<fatfs.h>		    // wrapper functions for most of the FatFs library
<graphics.h>    // hardware-independent higher-level graphical primitives
<platform.h>	  // support library for specific hardware platform implementations

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
