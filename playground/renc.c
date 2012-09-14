#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>

#include <pthread.h>

#include <wiringPi.h>
#include <lcd.h>

typedef struct _encdata {
	int pin_button;
	int pin_left;
	int pin_right;
	void *cb_button;
	void *cb_rotate;
} encdata;

#define BUTTON_LEFT	0
#define BUTTON_RIGHT	1

// lcd_<[registerselect|enable|data0-3]>_pin
#define LCD_RS_PIN	1
#define LCD_EN_PIN	4
#define LCD_D0_PIN	5
#define LCD_D1_PIN	11
#define LCD_D2_PIN	6
#define LCD_D3_PIN	10

// encoder_<[left|right]>_<[button|left|right]>_pin
#define EN_L_B_PIN	3
#define EN_L_R_PIN	2
#define EN_L_L_PIN	0
#define EN_R_B_PIN	14
#define EN_R_R_PIN	13
#define EN_R_L_PIN	12

int lcd = -1;
int enc_val = 0;
int btn_val = 0;
int shutdown = 0;

pthread_t t_encoder;
mutex_t m_lcd, m_enc, m_btn;

void lcdup() {
	pthread_mutex_lock(&m_lcd);
	stdoutup();
	lcdClear(lcd);
	delay(2);
	lcdPrintf(lcd, "v: %i, b: %i", enc_val, btn_val);
	delay(2);
	pthread_mutex_unlock(&m_lcd);
}

void stdoutup() {
	printf("\033[2K\r\tvalue: %i\tbutton: %i", enc_val, btn_val);
}

void cb_button(int btn, int state) {
	pthread_mutex_lock(&m_btn);
	if (state) btn_val |= 1 << btn; else btn_val &= ~(1 << btn);
	pthread_mutex_unlock(&m_btn);
	lcdup();
}

void cb_encoder(int enc, int direction) {
	pthread_mutex_lock(&m_enc);
	
	pthread_mutex_unlock(&m_enc);
}

void t_encoder_func(void *args) {
	int roen[3] = { 12, 13, 14 };
	int roev[3] = { 0, 0, 0 }, u;
	int l, e, s, b = 0;

	for (u = 0; u < 3; ++u) { pinMode(roen[u], INPUT); pullUpDnControl(roen[u], PUD_UP); }

	lcdup();
	while (1) {
		for (u = 0; u < 3; ++u) roev[u] = digitalRead(roen[u]);
		e = (roev[0] << 1) | roev[1];
		s = (l << 2) | e;
		if (roev[2] != b) {
			b = roev[2];
			if (b == 0) btn_down(); else btn_up();
		} else {
			if (s == 1) enc_left(); // || s == 14
			if (s == 2) enc_right(); // || s == 13
		}
		delay(2);
		l = e;
		if (shutdown) break;
	}

	return;
}

void sighandler(int signum) {
	printf("\ncaught signal %i\n", signum);
	if (signum == SIGINT || signum == SIGTERM) {
		if (lcd > -1) {
			printf("clearing lcd...\n");
			delay(20);
			lcdClear(lcd);
		}
		printf("exiting...\n");
		shutdown = 1;
		exit(signum);
	}
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
	lcd = lcdInit(2, 16, 4, LCD_RS_PIN, LCD_EN_PIN, LCD_D0_PIN, LCD_D1_PIN, LCD_D2_PIN, LCD_D3_PIN, 0, 0, 0, 0);
	delay(5);
	if (lcd == -1) {
		printf("lcdinit failed!\nexiting.\n");
		exit(2);
	}

	pthread_create(&t_encoder, NULL, (void *)t_encoder_func, NULL);
	pthread_join(t_encoder, NULL);

	return 0;
}

