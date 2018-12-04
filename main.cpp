#include "mbed.h"

void draw_main();

Thread draw_main_task(osPriorityNormal, 500 * 1024);
Serial pc(USBTX, USBRX);
I2C master(I2C_SDA, I2C_SCL);

//int main()
//{
//    pc.printf("RUN\r\n");
//    for(int i = 0; i < 128 ; i++) {
//    	master.start();
//        if(master.write(i << 1)) pc.printf("0x%x ACK \r\n",i); // Send command string
//        master.stop();
//    }
//}

int main() {
	draw_main_task.start(&draw_main);
	while (true)
		;
	return 0;
}

enum {
	cmd_print = 0x01,
	cmd_setCursor,
	cmd_fillRect,
	cmd_setTextSize,
	cmd_setTextColor
};

typedef struct {
	uint32_t stable;
	float weight;
} weight_t;

typedef enum {
	release = 0, push, none,
} key_state;

typedef struct {
	char key;
	key_state state;
} keymat_t;

const int kaiten_addr = 0x15 << 1;
const int led_addr = 0x20 << 1;
const int servo_addr = 0x25 << 1;

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

void draw_main() {

	keymat_t key_data;
	weight_t weights;
	static uint8_t cnt = 0;

	while (1) {
		weights = get_weight();
		key_data = get_key();
		pc.printf("%d, %d\n", key_data.key, key_data.state);

		if (key_data.state == push) {
			switch (key_data.key) {
			case '0':
				cmd_servo(0, 90);
				break;
			case '1':
				cmd_servo(90, 90);
				break;
			case '2':
				cmd_servo(180, 90);
				break;
			case '*':
				cmd_servo(0, 10);
				break;
			case '#':
				cmd_servo(180, 90);
				break;
			}
		}

		if (key_data.key == '3') {
			if (key_data.state == push) {
				cmd_kaiten(50);
			} else {
				cmd_kaiten(0);
			}
		}

		if (key_data.key == '4') {
			if (key_data.state == push) {
				cmd_lamp(true);
			} else {
				cmd_lamp(false);
			}
		}

		if (cnt == 5) {
			cmd_clear_all();
			cmd_set_cursor(0, 0);
			cmd_set_text_color(0, 15, 0);

			char str[50] = "0123456789\nabcdefg\nhigklmn\nABCDEFG\n";
			cmd_print_text(str);
		}
		if (cnt == 9) {
			cmd_clear_all();
			cmd_set_cursor(0, 0);
			cmd_set_text_color(15, 0, 0);

			char str[50];
			sprintf(str, "weight:\n%f kg", weights.weight);
			cmd_print_text(str);
		}

		cnt++;
		if (cnt > 10) {
			cnt = 0;
		}

		Thread::wait(500);
	}
}
