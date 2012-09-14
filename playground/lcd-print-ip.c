#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>

#include <wiringPi.h>
#include <lcd.h>

int pins[] = { 7, 0, 2, 3, 1, 4 };
int lcd = -1;
int reval = 0;
int btns = 0;

void lcdup(int val, int sum, int btn) {
	printf("updating lcd with value %i, sum was %i, button pressed %i times\n", val, sum, btn);
	lcdClear(lcd);
	delay(2);
	lcdPrintf(lcd, "value: %i, btn: %i", val, btn);
}

int main(int argc, char* argv[]) {
	setbuf(stdout, NULL);
	printf("\n\tkitpi LCD!\n");

	signal(SIGINT, sighandler);

	if (wiringPiSetup() == -1) {
		printf("wiringPiSetup failed!\nexiting.\n");
		exit(1);
	}

	lcd = lcdInit(2, 16, 4, 1, 4, 7, 0, 2, 3, 0, 0, 0, 0);
	delay(5);
	if (lcd == -1) {
		printf("lcdinit failed!\nexiting.\n");
		exit(2);
	}

	pinMode(10, INPUT); pullUpDnControl(10, PUD_UP);
	pinMode(11, INPUT); pullUpDnControl(11, PUD_UP);
	pinMode(14, INPUT); pullUpDnControl(14, PUD_UP);

	lcdup(reval, 0, 0);
	int l, e, s, b, o;
	int p1, p2;
	for (;;) {
		p1 = digitalRead(10);
		p2 = digitalRead(11);
		if (p1 != o) {
			b = digitalRead(14);
			if (p2 == 0) {
				lcdup(++reval, 0, b);
			} else {
				lcdup(--reval, 0, b);
			}
		}
		o = p1;
		delay(1);
/*
		e = (digitalRead(10) << 1) | digitalRead(11);
		s = (l << 2) | e;
		b = digitalRead(14);
		// if (b == 1) lcdup(reval, b, ++btns);
		if (s == 2 || s == 13) lcdup(++reval, s, btns);
		if (s == 1 || s == 14) lcdup(--reval, s, btns);

		delayMicroseconds(10);
		l = e;
*/
	}

	return 0;
}

