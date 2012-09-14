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
} encdata;

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

pthread_t t_encoders[2];
pthread_mutex_t m_lcd, m_enc, m_btn;

void stdoutup() {
	printf("\033[2K\r\tvalue: %i\tbutton: %i", enc_val, btn_val);
}

void lcdup() {
	pthread_mutex_lock(&m_lcd);
	stdoutup();
	lcdClear(lcd);
	delay(4);
	lcdPrintf(lcd, "v: %i, b: %i", enc_val, btn_val);
	delay(4);
	pthread_mutex_unlock(&m_lcd);
}

void cb_button(encdata *enc, int state) {
	btn_val = state;
	lcdup();
}

void cb_rotate(encdata *enc, int direction) {
	enc_val = enc_val + direction;
	lcdup();
}

void t_encoder_func(void *args) {
	int val_pin_button = 0,
	    val_pin_left = 0,
	    val_pin_right = 0;
	int l, e, s, b = 0;

	encdata *enc = (encdata*)args;

	pinMode(enc->pin_button, INPUT); pullUpDnControl(enc->pin_button, PUD_UP);
	pinMode(enc->pin_left  , INPUT); pullUpDnControl(enc->pin_left  , PUD_UP);
	pinMode(enc->pin_right , INPUT); pullUpDnControl(enc->pin_right , PUD_UP);

	while (1) {
		val_pin_button = digitalRead(enc->pin_button);
		val_pin_left = digitalRead(enc->pin_left);
		val_pin_right = digitalRead(enc->pin_right);

		e = (val_pin_left << 1) | val_pin_right;
		s = (l << 2) | e;
		if (val_pin_button != b) {
			b = val_pin_button;
			pthread_mutex_lock(&m_btn);
			cb_button(enc, val_pin_button);
			pthread_mutex_unlock(&m_btn);
		}
		if (s == 1 || s == 2) {
			pthread_mutex_lock(&m_enc);
			cb_rotate(enc, --s ? s : --s);
			pthread_mutex_unlock(&m_enc);
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
		//exit(signum);
	}
}

encdata* mkencdata(encdata *enc, int button, int left, int right) {
	enc->pin_button = button;
	enc->pin_left = left;
	enc->pin_right = right;
	return enc;
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

	encdata enc_left, enc_right;

	pthread_create(&t_encoders[0], NULL, (void *)t_encoder_func, (void*)mkencdata(&enc_left, 3, 0, 2));
	pthread_create(&t_encoders[1], NULL, (void *)t_encoder_func, (void*)mkencdata(&enc_right, 14, 12, 13));
	pthread_join(t_encoders[0], NULL);
	pthread_join(t_encoders[1], NULL);
//	for (size_t i = 0; i < sizeof(t_encoders) * sizeof(encdata); ++i) pthread_join(t_encoders[i], NULL);

	return 0;
}

