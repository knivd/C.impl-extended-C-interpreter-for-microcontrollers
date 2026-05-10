# C.impl Interpreter Error Codes Reference

This document describes all error codes returned by the C.impl interpreter.

## Error Code Listing

| Code | Name | Description |
|------|------|-------------|
| -1 | TERMINATED | Program terminated (not an error) |
| 0 | OK | Successful execution |
| 1 | SYNTAX | General syntax error |
| 2 | UNEXPECTED_END | Unexpected end of input/file |
| 3 | INVALID_NUMBER | Invalid numeric constant format |
| 4 | INVALID_CHARACTER | Invalid character in source code |
| 5 | ID_TOO_LONG | Identifier exceeds maximum length (25 characters) |
| 6 | DATA_TYPE_EXPECTED | Data type keyword expected but not found |
| 7 | VALUE_EXPECTED | Value/expression expected but not found |
| 8 | IDENTIFIER_EXPECTED | Identifier expected but not found |
| 9 | SEMICOLON_EXPECTED | Semicolon ';' expected but not found |
| 10 | COLON_EXPECTED | Colon ':' expected but not found |
| 11 | DQUOTE_EXPECTED | Double quote '"' expected but not found |
| 12 | COMMA_EXPECTED | Comma ',' expected but not found |
| 13 | OP_PAREN_EXPECTED | Opening parenthesis '(' expected but not found |
| 14 | CL_PAREN_EXPECTED | Closing parenthesis ')' expected but not found |
| 15 | OP_BRACE_EXPECTED | Opening brace '{' expected but not found |
| 16 | CL_BRACE_EXPECTED | Closing brace '}' expected but not found |
| 17 | OP_BRACKET_EXPECTED | Opening bracket '[' expected but not found |
| 18 | CL_BRACKET_EXPECTED | Closing bracket ']' expected but not found |
| 19 | OP_BRACE_OR_QUOTE_EXPECTED | Opening brace '{' or double quote '"' expected |
| 20 | CL_BRACE_OR_QUOTE_EXPECTED | Closing brace '}' or double quote '"' expected |
| 21 | CASE_OT_DEFAULT_EXPECTED | 'case' or 'default' keyword expected in switch |
| 22 | WHILE_EXPECTED | 'while' keyword expected (do-while syntax) |
| 23 | UNEXPECTED_ELSE | 'else' keyword found without matching 'if' |
| 24 | UNEXPECTED_CASE | 'case' keyword found outside switch statement |
| 25 | UNEXPECTED_DEFAULT | 'default' keyword found outside switch statement |
| 26 | UNEXPECTED_BREAK | 'break' statement found outside loop or switch |
| 27 | UNEXPECTED_CONTINUE | 'continue' statement found outside loop |
| 28 | UNEXPECTED_TOKEN | Unexpected token/keyword in this context |
| 29 | MEMORY_ALLOCATION | Failed to allocate memory |
| 30 | DUPLICATED_FUNCTION | Function with same name already defined |
| 31 | DUPLICATED_VARIABLE | Variable with same name already defined in same scope |
| 32 | DUPLICATED_LABEL | Label with same name already defined |
| 33 | DUPLICATED_DEFAULT | Multiple 'default' cases in switch statement |
| 34 | DUPLICATED_ENUM | Enumeration with same name already defined |
| 35 | TOO_COMPLEX | Expression is too complex (exceeds MAX_TERMS limit) |
| 36 | MAXIMUM_NESTING | Maximum nesting depth exceeded |
| 37 | DIVISION_BY_ZERO | Division by zero attempted |
| 38 | IMPOSSIBLE_CONVERSION | Cannot convert between data types |
| 39 | INVALID_DATA_TYPE | Invalid or unsupported data type |
| 40 | NOT_ALLOWED_WITH_PTR | Operation not allowed with pointer types |
| 41 | TOO_MANY_DIMENSIONS | Array has too many dimensions (max: 5) |
| 42 | TOO_MANY_PARAMETERS | Function has too many parameters (max: 20) |
| 43 | TOO_MANY_POSTMODS | Too many post-increment/decrement operations |
| 44 | UNKNOWN_IDENTIFIER | Identifier not defined |
| 45 | INVALID_ASSIGNMENT | Invalid assignment or type mismatch |
| 46 | INVALID_POINTER | Invalid pointer operation or dereferencing |
| 47 | WRITING_CONST_POINTER | Attempt to write through const pointer |
| 48 | WRITING_CONST | Attempt to write to const variable |
| 49 | UNKNOWN_DIRECTIVE | Unknown preprocessor directive |
| 50 | UNKNOWN_LIBRARY | Unknown library function called |
| 51 | UNKNOWN_PRAGMA | Unknown pragma directive |
| 52 | UNKNOWN_LABEL | Referenced label not defined |
| 53 | INVALID_ELLIPSIS | Invalid use of ellipsis '...' operator |
| 54 | UNABLE_TO_INCLUDE | Cannot include specified file |
| 55 | ASSERTION_FAILED | Assertion failed |
| 56 | INSUFFICIENT_RESOURCE | Insufficient system resources (stack, memory, etc.) |

## Common Error Scenarios

### Syntax Errors (1-28)
These errors occur when the source code violates C language syntax rules:
- Missing or mismatched delimiters (parentheses, braces, brackets)
- Missing semicolons or expected keywords
- Invalid token sequences

### Resource Errors (29, 35-36)
These indicate system resource limitations:
- Out of memory (error 29)
- Expression too complex for the interpreter's stack (error 35)
- Exceeds nesting depth limits (error 36)

### Runtime Errors (37-38)
These occur during program execution:
- Division by zero (error 37)
- Type conversion impossible (error 38)

### Identifier Errors (44-48)
These relate to variable, function, or pointer operations:
- Undefined variable or function (error 44)
- Type mismatch in assignment (error 45)
- Invalid pointer operation (error 46)

### Feature/Library Errors (49-55)
These indicate unsupported features or missing libraries:
- Unknown function call (error 50)
- File include error (error 54)
- Assertion failure (error 55)

## Debugging Tips

1. **Error Code -1 (TERMINATED)**: Program exited with `exit()` function
2. **Error Code 0 (OK)**: Successful execution
3. For other error codes, check the error message text first
4. If only a number appears, use this table to identify the issue
5. Review the source code at the location where the error was reported
6. Check variable and function names for typos or case sensitivity
7. Verify all brackets, braces, and parentheses are properly matched
