# C.impl

C.impl is a small portable C interpreter integrated with a line text editor, originally developed for the ELLO 1A computer: http://ello.cc<br>
<br>
The aim of this code is to achieve a close C90 specification functional implementation<br>
of an interpreter for the C language with minimum required system resources and without<br>
references to specific external layers such as OS.<br>
<br>
Data types:<br>
<br>
'bool'                          - defined as 1-bit integer<br>
'char'                          - defined as 8-bit integer<br>
'short' or 'short int'          - defined as 16-bit integer<br>
'int'                           - defined as per the 'typedef int_t' statement<br>
'long' or 'long int'            - defined as 32-bit integer<br>
'long long' or 'long long int'  - defined as 64-bit integer<br>
'float'                         - single precision floating point number (32-bit)<br>
'double'                        - double precision floating point number (64-bit)<br>
'long double'                   - quadruple precision floating point number (80-bit or more)<br>
<br>
In addition to the basic types above, several other data types are also supported:<br>
<br>
'size_t'                        - natively represented as 'unsigned int'<br>
'enum'                          - standard C-type enum list<br>
'struct'                        - standard C-type data structure<br>
'union'                         - standard C-type data union<br>
'va_list'                       - used in conjunction with the <stdarg.h> library for variable argument lists<br>
'FILE'                          - used with file functions in the <stdio.h> library<br>
<br>
'signed' or 'unsigned' optionally preceding the integer types, except 'bool'<br>
All integer types are signed by default (i.e. as if preceded by a 'signed' declaration)<br>
<br>
Constant data types:<br>
<br>
'char'      - single 8-bit character<br>
"string"    - string of zero-terminated 8-bit characters<br>
up to 64-bit decimal integer numbers (optionally preceded by '0d' or '0D')<br>
up to 64-bit hexadecimal unsigned numbers; preceded by '0x' or '0X'<br>
up to 64-bit binary unsigned numbers; preceded by '0b' or '0B'<br>
up to 64-bit octal unsigned numbers; preceded by '0'<br>
floating point numbers (always decimal) in format:<br>
    [sign] [ iii [ .fff ['E/e' [sign]  [ eee [ .xxx ] ] ] ] ]<br>
<br>
Decimal numbers optionally support suffix specifiers for sign and/or size:<br>
'u' or 'U' for unsigned<br>
'l' or 'L' for long integer or long double<br>
'll' or 'LL' for long long integer<br>
'f' or 'F' for float<br>
'd' or 'D' for double<br>
<br>
The default type of all integer constants, unless explicitly specified, is 'signed int'<br>
The default type of floating point number constants, unless explicitly specified, is 'float'<br>
<br>
Pre-defined character literals:<br>
<br>
'\0'	- ASCII code 0 (character NUL)<br>
'\a'    - ASCII code 7 (alarm)<br>
'\b'    - ASCII code 8 (backspace)<br>
'\e'    - ASCII code 27 (escape)<br>
'\f'    - ASCII code 12 (form feed)<br>
'\n'    - ASCII code 10 (new line)<br>
'\r'    - ASCII code 13 (carriage return)<br>
'\t'    - ASCII code 9 (horizontal tabulation)<br>
'\\'    - the character 'backslash'<br>
'\''    - the character 'single quote'<br>
'\"'    - the character 'double quote'<br>
'\?'    - the character 'question mark'<br>
<br>
Other notes and features:<br>
<br>
1. True execution in place. The source code is not compiled or modified and can run directly from ROM<br>
2. Fully dynamic memory model with no limitation on the number of functions or variables, or their size<br>
3. Arrays and pointers are fully supported<br>
4. Inline local variable definitions are allowed, eg. for(int t=0; .....<br>
5. String literals can be split on to multiple lines<br>
6. Pointer to constant (eg. const char *) and constant pointer (eg. char const *), are both supported<br>
7. Standard C and C++ style commentaries are both supported<br>
<br>
Built-in libraries:<br>
<br>
< stdlib.h >     // extra: unsigned long BIT(unsigned char n)<br>
< stdio.h >      // extra: void run(const char *filename)<br>
< stdint.h ><br>
< stdbool.h ><br>
< stdarg.h ><br>
< string.h ><br>
< math.h ><br>
< limits.h ><br>
< ctype.h ><br>
< time.h ><br>
< assert.h >     // can be used also to assert the existence of library functions, eg. assert(printf())<br>
< conio.h >      // console functions<br>
< fatfs.h >		 // wrapper functions for most of the FatFs library<br>
< graphics.h >   // hardware-independent higher-level graphical primitives<br>
< platform.h >   // support library for specific hardware platform implementations<br>
<br>
Known issues and limitations:<br>
<br>
1. Function pointers and function address dereferencing are not supported (no feasible way in an interpreter!)<br>
2. Support for preprocessor directives is reduced only to #include<br>
3. In for(), while(), and do...while() loops, 'break' and 'continue' can only work from within a {} block<br>
4. The 'goto' instruction only allows jumping to a location at the same or upper {} depth level<br>
5. A goto label must have unique name among all labels globally.<br>
6. The 'extern' keyword is not supported since the source can be only a single file<br>
7. In the ternary operator ?: there must be a space before the colon ':' so it is not mistaken for a label<br>
8. The main() function must not have parameters or a return value<br>
9. In composite data types (enum, struct, union), the name of the type must be before the {} block, but can't be after<br>
10. Structures, unions, and enums are all supported but have high runtime memory cost and should be used with care<br>
11. Keywords 'struct', 'union', and 'enum' must be used only for the initial type declaration, but not for instances<br>
12. Casting to structure or union pointer will not work. The cost of its implementation in an interpreter<br>
    is massive in terms of both runtime memory and execution speed<br>
13. Indexing by pointers (eg. int a[10]; int b = *(a+1); ) will not work. Indexing needs to be done in the brackets way<br>
<br>
Other notes:<br>
<br>
The entire code is deeply recursive so it requires sufficient stack for its operation. With the<br>
default settings, at least 4kB stack size is recommended.<br>
<br>
