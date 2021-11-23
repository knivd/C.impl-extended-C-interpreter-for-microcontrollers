#include <stdlib.h>
#include <stdio.h>
#include <platform.h>
#include <time.h>
#include <string.h>
#include <math.h>

void main(void) {
	printf("C.impl benchmark (C) KnivD 2016, 2019, 2020\r\n");
	int t, i = 0;
	double x[1001], f = 0.0;
	char s1[5], s[50];
	printf("Calculating... ");
	int TIMER = clock();
	while((clock() - TIMER) < 30000) {
		i += 2; f += 2.0002;
		if((i % 2) == 0) {
			i *= 2; i /= 2;
			f *= 2.0002; f /= 2.0002;
		}
		i--;
		t = 100; while(t--) {
			f -= 1.0001;
			if((f - floor(f)) >= 0.5) {
				f = sin(f * log10(i));
				sprintf(s, "%6.6f", f);
			}
			f = (f - tan(i)) * ((rand() / RAND_MAX) / i);
			sprintf(s, "%d", i);
			strncpy(s1, s, 2);
			s1[2] = 0;
			if(strstr(s1, s)) strcat(s, "0");
		}
		x[1 + (i%1000)] = f;
	}
	printf("\rPerformance: %d grains\r\n", (i * 1024) / 286);
}
