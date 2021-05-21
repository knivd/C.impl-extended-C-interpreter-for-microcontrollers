#include "platform.h"
#include "../../xmem.h"
#include "../../ride/fos.h"
#include "../../RIDE/ride.h"   /* for the settings{} structure */


void initPlatform(void) {
	SysMem = MEMORY;
	SysMemSize = x_meminit();
    f_mount(&FatFs, "IFS:", 0);
	enable_flags |= FLAG_RTC;
	time(&ss_time);
}


void resetPlatform(void) {
	printf("Not supported\r\n\n");
}


void mSec(clock_t ms) {
	unsigned long ce = clock() + (ms * (CLOCKS_PER_SEC / 1000));
	while((unsigned long) clock() > ce) continue;
	while((unsigned long) clock() < ce) continue;
}


/* primitive function stubs */

int dummy;	/* used only to avoid the annoying "unused variable" messages from the compiler */
void initVideo(uint8_t mode) { dummy = mode; }
void clearScreen(int c) { dummy = c; }
void scrollUp(int vl, int c) { dummy = vl = c; }
void setPixel(int x, int y, int c) { dummy = x = y = c; }
int getPixel(int x, int y) { dummy = x = y; return 0; }
void beep(void) {}

