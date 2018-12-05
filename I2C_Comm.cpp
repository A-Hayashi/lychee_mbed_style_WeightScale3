#include "mbed.h"
#include "I2C_Comm.h"

enum {
	cmd_print = 0x01,
	cmd_setCursor,
	cmd_fillRect,
	cmd_setTextSize,
	cmd_setTextColor
};


static I2C master(I2C_SDA, I2C_SCL);

static const int kaiten_addr = 0x15 << 1;
static const int led_addr = 0x20 << 1;
static const int servo_addr = 0x25 << 1;

void cmd_servo(uint8_t angle, uint8_t speed) {
	char servo_cmd[4] = { 0x01, 0x01, 0, 90 };
	servo_cmd[0] = 0x01;
	servo_cmd[1] = 0x01;
	servo_cmd[2] = angle;
	servo_cmd[3] = speed;
	master.write(servo_addr, servo_cmd, 4);
}

void cmd_kaiten(uint8_t duty) {
	char kaiten_cmd[2];

	kaiten_cmd[0] = 0x01;
	kaiten_cmd[1] = duty;

	master.write(kaiten_addr, kaiten_cmd, 2);
}

void cmd_lamp(bool lamp) {
	char kaiten_cmd[2];

	kaiten_cmd[0] = 0x02;
	kaiten_cmd[1] = lamp;

	master.write(kaiten_addr, kaiten_cmd, 2);
}


void cmd_kaiten_lamp(uint8_t duty, bool lamp) {
	cmd_kaiten(duty);
	cmd_lamp(lamp);
}

weight_t get_weight(void) {
	const int weight_addr = 0x30 << 1;
	weight_t weights;
	char *p = (char *) &weights;
	master.read(weight_addr, p, 8);

	return weights;
}

keymat_t get_key(void) {
	const int key_addr = 0x10 << 1;
	keymat_t key_data;
	char buff[2];

	master.read(key_addr, buff, 2);

	key_data.key = buff[0];
	switch (buff[1]) {
	case 0:
		key_data.state = release;
		break;
	case 1:
		key_data.state = push;
		break;
	case 0xff:
	default:
		key_data.state = none;
		break;
	}

	return key_data;
}

void cmd_clear_line(uint8_t line) {
	char led_cmd[8];

	led_cmd[0] = cmd_fillRect;
	led_cmd[1] = 0;
	led_cmd[2] = line * 8;
	led_cmd[3] = 64;
	led_cmd[4] = 8;
	led_cmd[5] = 0;
	led_cmd[6] = 0;
	led_cmd[7] = 0;
	master.write(led_addr, led_cmd, 8);
}

void cmd_clear_all(void) {
	char led_cmd[8];

	led_cmd[0] = cmd_fillRect;
	led_cmd[1] = 0;
	led_cmd[2] = 0;
	led_cmd[3] = 64;
	led_cmd[4] = 32;
	led_cmd[5] = 0;
	led_cmd[6] = 0;
	led_cmd[7] = 0;
	master.write(led_addr, led_cmd, 8);
}

void cmd_set_cursor(uint8_t x, uint8_t y) {
	char led_cmd[3];

	led_cmd[0] = cmd_setCursor;
	led_cmd[1] = x;
	led_cmd[2] = y;
	master.write(led_addr, led_cmd, 3);
}

void cmd_set_text_color(uint8_t r, uint8_t g, uint8_t b) {
	char led_cmd[4];

	led_cmd[0] = cmd_setTextColor;
	led_cmd[1] = r;
	led_cmd[2] = g;
	led_cmd[3] = b;
	master.write(led_addr, led_cmd, 4);
}

void cmd_print_text(char *str) {
	char led_cmd[200];

	led_cmd[0] = cmd_print;

	uint8_t size = strlen(str) + 1;

	for (int i = 0; i < size; i++) {
		led_cmd[1 + i] = str[i];
	}

	master.write(led_addr, led_cmd, size + 1);
}
