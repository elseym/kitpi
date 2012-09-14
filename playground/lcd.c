#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>

#include <wiringPi.h>
#include <lcd.h>

int lcd = -1;
int reval = 0;
int btns = 0;


PI_THREAD (thrButton) {
	
}


void sighandler(int signum) {
	printf("\ncaught signal %i\n", signum);
	if (signum == SIGINT || signum == SIGTERM) {
		if (lcd > -1) {
			printf("clearing lcd...\n");
			delay(20);
			lcdClear(lcd);
		}
		printf("exiting.\n");
		exit(signum);
	}
}

void lcdup(int val, int btn, int roev[3]) {
	lcdClear(lcd);
	delay(2);
	printf("\033[2K\r\tvalue: %i\tbutton: %i\t\tenc1: %i\tenc2: %i\tencb: %i", val, btn, roev[0], roev[1], roev[2]);
	lcdPrintf(lcd, "v: %i, b: %i", val, btn);
	delay(2);
}

int main(int argc, char* argv[]) {
	setbuf(stdout, NULL);
	printf("\n\tkitpi LCD!\n\n");

	signal(SIGINT, sighandler);

	if (wiringPiSetup() == -1) {
		printf("wiringPiSetup failed!\nexiting.\n");
		exit(1);
	}
	// rows, cols, bits, rs, en, d0-d7
	lcd = lcdInit(2, 16, 4, 15, 16, 7, 0, 2, 3, 0, 0, 0, 0);
	delay(5);
	if (lcd == -1) {
		printf("lcdinit failed!\nexiting.\n");
		exit(2);
	}

	int roen[3] = { 12, 13, 14 };

	pinMode(roen[0], INPUT); pullUpDnControl(roen[0], PUD_UP);
	pinMode(roen[1], INPUT); pullUpDnControl(roen[1], PUD_UP);
	pinMode(roen[2], INPUT); pullUpDnControl(roen[2], PUD_UP);

	int l, e, s, b, o;
	int roev[3] = { 0, 0, 0 }, u;
	lcdup(reval, 0, roev);
	for (;;) {
		for (u = 0; u < 3; ++u) {
//			delayMicroseconds(1000);
			roev[u] = digitalRead(roen[u]);
		}
		e = (roev[0] << 1) | roev[1];
		s = (l << 2) | e;
		b = roev[2];
		if (b == 0) {
			lcdup(reval, ++btns, roev);
		} else {
			if (s == 2 || s == 13) lcdup(++reval, s, roev);
			if (s == 1 || s == 14) lcdup(--reval, s, roev);
		}
		l = e;
		delay(2);
	}

	return 0;
}

