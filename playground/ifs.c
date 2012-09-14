#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <wiringPi.h>
#include <lcd.h>

int main(void) {
	char buf[1024];
	int i;

	int sck = socket(AF_INET, SOCK_DGRAM, 0);
	if(sck < 0) {
		perror("socket");
		return 1;
	}

	struct ifconf ifc;
	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;
	if(ioctl(sck, SIOCGIFCONF, &ifc) < 0) {
		perror("ioctl(SIOCGIFCONF)");
		return 1;
	}

	if (wiringPiSetup() == -1) {
		printf("wiringPiSetup failed!\nexiting.\n");
		return 1;
	}

	int lcd = lcdInit(2, 16, 4, 15, 16, 7, 0, 2, 3, 0, 0, 0, 0);
	delay(5);
	if (lcd == -1) {
		printf("lcdinit failed!\nexiting.\n");
		return 2;
	}

	lcdClear(lcd);
	delay(2);

	struct ifreq *ifr = ifc.ifc_req;
	int nInterfaces = ifc.ifc_len / sizeof(struct ifreq), y = 0;
	for(i = 0; i < nInterfaces; i++) {
		struct ifreq *item = &ifr[i];
		if (strcmp(item->ifr_name, "lo") == 0) continue;
		lcdPosition(lcd, 0, y++);
		delay(2);
		lcdPrintf(lcd, "%s: %s", item->ifr_name, inet_ntoa(((struct sockaddr_in *)&item->ifr_addr)->sin_addr));
		delay(5);
	}

  return 0;
}
