#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <wiringPi.h>
#include <lcd.h>

#define MAX_LINES 2

void die(const char* msg, int exitcode) {
	printf("%s", msg);
	exit(exitcode);
}

int main(int argc, char *argv[]) {
	int lcd, i;

	if (wiringPiSetup() == -1) die("wiringPiSetup failed!\nexiting.\n", 1);

	lcd = lcdInit(MAX_LINES, 16, 4, 1, 4, 5, 11, 6, 10, 0, 0, 0, 0);
	if (lcd == -1) die("lcdinit failed!\nexiting.\n", 2);

	delay(2);
	lcdClear(lcd);

	for (i = 1; i < argc; ++i) {
		delay(2);
		lcdPosition(lcd, 0, (i - 1) % MAX_LINES);
		delay(2);
		lcdPrintf(lcd, "%s",  argv[i]);
	}
	return 0;
}
