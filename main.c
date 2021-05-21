#include "main.h"

// main entry point
int main(void) {

    /* initialise default settings */
    settings.stored_password = 0;
    settings.kbd_layout = LAYOUT_UK;
    settings.brk_code = KEY_BREAK;
    settings.page_width = PAGE_WIDTH;
    settings.page_height = PAGE_HEIGHT;
    settings.tab_width = TAB_WIDTH;
    settings.checksum = hash((const char *) (&settings + sizeof(settings.checksum)), (sizeof(settings) - sizeof(settings.checksum)));

	initPlatform();
    beep();

    int ch;
    FRESULT fr = FR_OK;

    if(enable_flags & FLAG_VIDEO) {
        posX = posY = 0;
        enable_flags |= FLAG_NO_ECHO;
        fontScale = 3;
        drawRect(0, 0, (Hres - 1),
                    fontScale * (font->header.height + font->header.blankT + font->header.blankB) - 2,
                    COL_GREY25);
        fontFcol = COL_SOLID;
        fontBcol = COL_TRANSP;
        posX = (Hres - (6 + strlen(PLATFORM_NAME) * fontScale *
                            (font->header.width + font->header.blankL + font->header.blankR))) / 2;
    }
	else {
        printf("\r");
        for(ch = 0; ch < 100; ch++) printf("\n");
    }
    printf("%s\r\n", PLATFORM_NAME);

    if(enable_flags & FLAG_VIDEO) {
        fontScale = 1; posX += 4;
        fontFcol = COL_SOLID;
        fontBcol = COL_NONE;
	}

    mSec(1000);
    if(enable_flags & FLAG_VIDEO) mSec(3000);   /* allow time for the monitor to adjust to the video mode */
    if(enable_flags & FLAG_USB_PRES) {
        printf("\r                    \rAttaching USB... ");
        mSec(500);
        ch = 50;    // 5 seconds
        while((enable_flags & FLAG_USB_PRES) && (enable_flags && FLAG_USB) == 0 && ch--) mSec(100);
        printf("\r                    \r");
    }

    if(enable_flags & FLAG_VIDEO) {
        posX = (Hres - (17 + strlen(AUTHOR) + strlen(SW_VERSION)) * fontScale *
                            (font->header.width + font->header.blankL + font->header.blankR)) / 2;
    }
    printf("R%s (C) %c%c%c %s, %s\r", SW_VERSION,
                                        __DATE__[strlen(__DATE__) - 11],
										__DATE__[strlen(__DATE__) - 10],
										__DATE__[strlen(__DATE__) - 9],
										&__DATE__[strlen(__DATE__) - 4],
                                        AUTHOR);

    if((enable_flags & FLAG_VIDEO) == 0 || (enable_flags & FLAG_USB)) printf("\n");
    if(enable_flags & FLAG_USB) printf("[usb]");
    if(enable_flags & FLAG_SERIAL) printf("[ser]");
    if(enable_flags & FLAG_PS2) printf("[kbd]");
    if(enable_flags & FLAG_VIDEO) printf("[vid]");
    if(enable_flags & FLAG_RTC) printf("[rtc]");
    printf("\r\n\n");

    #if IFS_DRV_KB > 0
        fr = FR_OK;
        f_chdrive("IFS:");
        fr = f_mount(&FatFs, "IFS:", 1);
        if(fr == FR_NO_FILESYSTEM) execute_cmd_fos("init IFS:");
    #endif

	/* read the stored settings */
    fr = FR_OK;
	f_chdrive(PWD_FILE);
	f_mount(&FatFs, PWD_FILE, 0);
	f_chdir(PWD_FILE);
    UINT rdwr;
	fr = f_open(&File, PWD_FILE, (FA_OPEN_EXISTING | FA_READ));
	if(fr == FR_OK) {
        ride_settings_t st;
        f_read(&File, &st, sizeof(ride_settings_t), &rdwr);
        if(fr == FR_OK && rdwr == sizeof(settings)) {
			memcpy(&settings, &st, sizeof(ride_settings_t));
        }
    }
	f_close(&File);

    if(enable_flags & FLAG_VIDEO) printf("Video  RAM: %5lu bytes\r\n", VidMemSize);
    printf("System RAM: %5lu bytes\r\n", SysMemSize);
    //if(enable_flags & FLAG_PS2) {
        printf("Kbd layout: ");
        switch(settings.kbd_layout) {
            case LAYOUT_US: printf("US"); break;
			case LAYOUT_UK: printf("UK"); break;
			case LAYOUT_DE: printf("DE"); break;
            case LAYOUT_FR: printf("FR"); break;
            default: printf("unknown"); break;
        }
        printf("\r\n");
    //}
    printf("\n");

    char autorun = 1;   /* auto-run flag */
    mSec(1000);
    printf("<Esc> to cancel autorun... ");
    ch = 10;    // 1000 milliseconds
    while(ch--) {
        mSec(100);
        if(kbhit() && getchx() == KEY_ESC) autorun = 0; /* <Esc> will cancel automated run */
    }
    printf("\r                             \r\n\n");

	/* auto-execute file */
    if(autorun) {
        FILINFO *finfo = NULL;
        x_malloc((byte **) &finfo, sizeof(FILINFO));
		char xt = 2, *xfn;
        while(xt-- && finfo) {
			if(xt == 1) xfn = AUTORUN_FILE;	/* first check the IFS: autorun file */
			else xfn = AUTORUN_FILE_SD;		/* alternatively check in SD1: */
			if(xfn && *xfn) {
				f_chdrive(xfn);
				f_mount(&FatFs, xfn, 0);
				f_chdir(xfn);
	            fr = f_stat(xfn, finfo);
	            if(fr == FR_OK) {
                    x_free((byte **) &finfo);
	                strcpy(buffer, xfn);
	                run_file(buffer);
	                RIDE_release_memory();
	                printf("\r\n\n");
					break;
	            }
			}
        }
        if(finfo) x_free((byte **) &finfo);
    }

	/* login with password */
    passw_mode = 1;
    while(1) {
        ch = edit_line(NULL, 0, 0);
        if(ch == KEY_ENT) {
            printf("\r");
            #ifdef BOOTLOADER_APP
            if(!strcmp(buffer, "./update")) bootloader();	/* run the bootloader */
            #endif
            if((settings.stored_password == 0 && *buffer == '\0') ||
                (*buffer && hash(buffer, strlen(buffer)) == settings.stored_password)) break;
            printf("\r");	/* wrong password at this point */
            for(ch = 0; ch < (int) strlen("password: "); ch++) {
                mSec(150);
                printf(" ");
            }
        }
	}
    passw_mode = 0;

	/* try to switch to SD1: */
	fr = f_chdrive("SD1:");
	if(fr == FR_OK) fr = f_mount(&FatFs, "SD1:", 1);
	if(fr == FR_OK) fr = f_chdir("/");
	if(fr != FR_OK) {
		f_chdrive(PWD_FILE);
		f_mount(&FatFs, PWD_FILE, 1);
		f_chdir(PWD_FILE);
	}

    while(1) RIDE();
    return 0;
}
