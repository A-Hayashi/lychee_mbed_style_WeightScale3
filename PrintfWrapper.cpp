#include "PrintfWrapper.h"

#define MAX_PAGE 8
#define MAXITEM MAX_PAGE

static Serial pc(USBTX, USBRX);
static I2C master(I2C_SDA, I2C_SCL);

// OLED制御クラスのインスタンス化
static uint8_t i2cAddress = SSD_I2C_ADDRESS << 1;
static uint8_t rawHeight = 64;
static uint8_t rawWidth = 128;
static Adafruit_SSD1306_I2c oled(master, D10, i2cAddress, rawHeight, rawWidth);

static int split(char *str, const char *delim, char *outlist[]);
static void aaa(char *buff);

static int split(char *str, const char *delim, char *outlist[]) {
	char *tk;
	int cnt = 0;

	tk = strtok(str, delim);
	while (tk != NULL && cnt < MAXITEM) {
		outlist[cnt++] = tk;
		tk = strtok( NULL, delim);
	}
	return cnt;
}

static void aaa(char *buff) {
	static int j = 0;
	static char str[MAX_PAGE][100];
	static char *tmp[MAX_PAGE];

	pc.printf(buff);
	int cnt = split(buff, "\n", tmp);
	for (int i = 0; i < cnt; i++) {
		strcpy(str[j], tmp[i]);
		j++;
		if (j >= MAX_PAGE) {
			j = 0;
		}
	}

	oled.clearDisplay();
	oled.setTextCursor(0, 0);
	for (int k = j; k < MAX_PAGE; k++) {
//		pc.printf("%s\n", str[k]);
		oled.printf("%s\n", str[k]);
	}
	for (int k = 0; k < j; k++) {
//		pc.printf("%s\n", str[k]);
		oled.printf("%s\n", str[k]);
	}
	oled.display();
//	pc.printf("========================\n");

	wait(1);
}

void printf2(char *fmt, ...) {
	char buf[100];
	va_list va;
	va_start(va, fmt);
	vsprintf(buf, fmt, va);
	va_end(va);
	aaa(buf);
}
