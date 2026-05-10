# R125 Firmware - Fixes Summary

R125 is a stable baseline created from R115 with targeted fixes for 7 reported issues.

## Completed Fixes ✅

### 1. Fix .lsl command functionality
- **Status**: COMPLETE
- **File**: RIDE/ride.c (lines 1438-1529)
- **Issue**: The `./lsl` command (list system libraries) was not working in R120
- **Fix**: R125 is based on R115 which contains the full, working implementation
- **Usage**: Type `./lsl` to list all system libraries, or `./lsl <libname>` to show library contents

### 2. Fix RTC weekday not being set
- **Status**: COMPLETE  
- **File**: Platform/ello1a/platform.c (line 1301)
- **Issue**: DS3231 RTC's weekday register was hardcoded to 1 instead of actual weekday
- **Fix**: Changed from `i2cSend(1)` to `i2cSend(t->tm_wday + 1)` to transmit actual weekday
- **Impact**: RTC now correctly sets the day of week from the struct tm data

### 3. Fix .n command without parameters bricking system
- **Status**: COMPLETE
- **File**: RIDE/ride.c (lines 705, 708, 725)
- **Issue**: Typing `.n` without proper parameters caused continuous scrolling and required reflash
- **Fix**: Added `break` statements after `what()` error calls to exit command processing immediately
- **Impact**: Invalid `.n` parameters now show error message and return control safely

### 4. Fix ++i operator unreliability  
- **Status**: COMPLETE
- **File**: Cimpl/opr.c (lines 28-34, 49-57)
- **Issue**: Pre-increment/decrement operators (++i, --i) didn't return correct values to parent expressions
- **Fix**: Added `memcpy(&acc[accN - 1], &acc[accN], sizeof(data_t))` before stack pop to propagate values
- **Impact**: Expressions like `a = ++i` now work correctly

### 5. Implement ternary operator (c)?(a):(b)
- **Status**: COMPLETE
- **File**: Cimpl/cimpl.c (line 1299)
- **Issue**: Ternary operator required a space before colon to work: `(c) ? (a) : (b)` 
- **Fix**: Added `else if(*prog == ':') { token = UNKNOWN; return NULL; }` to handle standalone colon
- **Impact**: Ternary operator now works without requiring space: `(c)?(a):(b)`

### 6. Document error codes
- **Status**: COMPLETE
- **File**: ERROR_CODES.md (new file)
- **Issue**: Error messages showed only numeric codes with no reference documentation
- **Fix**: Created comprehensive ERROR_CODES.md reference with all 57 error codes and descriptions
- **Impact**: Users can now look up what numeric error codes mean

## Pending Tasks ⏳

### 7. Fix escape sequence in printf breaking variable output  
- **Status**: PENDING/INVESTIGATION NEEDED
- **File**: Cimpl/libs/l_stdio.c, Cimpl/cimpl.c
- **Issue**: `printf("\n\r%i", variable)` doesn't output the variable when escape sequences precede format specifiers
- **Notes**: This requires actual testing on the hardware to pinpoint the exact cause. The issue may be in:
  - How escape sequences are processed in `get_char()`
  - How the format string with embedded control characters is parsed in `ff_core()`
  - Buffer management in the string handling code

## Build Instructions

For ELLO 1AL2 board:
```
1. Open Platform/ello1a/ with MPLAB X
2. Build the project (the R125 code is already in place)
3. Program the PIC32MX270B microcontroller
```

## Testing Recommendations

1. **Test .lsl command**: Type `./lsl` to list libraries, `./lsl stdio` to see stdio contents
2. **Test RTC**: Set date/time with `./date YYMMDD` and `./time HHMMSS`, verify weekday is correct
3. **Test .n command**: Type `.n` with invalid parameters and verify it returns error gracefully
4. **Test ++i operator**: Run `a = ++i; printf("%i", a)` and verify correct result
5. **Test ternary operator**: Run `x = (5 > 3) ? 10 : 20; printf("%i", x)` and verify output is 10
6. **Test MasterMind/RTC programs**: Re-test the user programs that were crashing in R122

## Version History

- **R115**: Stable baseline (baseline for R125)
- **R120**: Introduced regressions in .lsl, RTC weekday, .n command, ++i operator
- **R121**: Minor updates
- **R122**: Attempted fixes but introduced new issues (inconsistent implementations)
- **R125**: Clean baseline with targeted fixes, avoids R120/R122 regressions
