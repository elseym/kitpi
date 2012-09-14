#include <wiringPi.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>

int pins[] = { 0, 1 };

void sighandler(int signum) {
	printf("\ncaught signal %i\n", signum);
	if (signum == SIGINT || signum == SIGTERM) {
		int i, pincount = sizeof(pins) / sizeof(int);
		printf("switching off %i pins...\n", pincount);
		for (i = 0; i < pincount; ++i) {
			pinMode(i, OUTPUT);
			digitalWrite(i, LOW);
		}
		printf("exiting.\n");
		exit(signum);
	}
}

int main(int argc, char* argv[]) {
	setbuf(stdout, NULL);
	printf("Hallo\n");

	signal(SIGINT, sighandler);

	if (wiringPiSetup() == -1) {
		printf("wiringPiSetup failed!\nexiting.\n");
		exit(1);
	}

	pinMode(0, OUTPUT);
	pinMode(1, OUTPUT);

	int state = 0;

	for (;;) {
		digitalWrite(0, state);
		printf("0 / 17 / 11: %i\t1 / 18 / 12: %i\n", state, (state = (++state % 2)));
		digitalWrite(1, state);
		delay(1000);
	}

	return 0;
}
