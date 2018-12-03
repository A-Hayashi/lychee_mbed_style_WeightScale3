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


const int key_addr = 0x10 << 1;
const int servo_addr = 0x25 << 1;
const int kaiten_addr = 0x15 << 1;
const int led_addr = 0x20 << 1;
const int weight_addr = 0x30 << 1;

void draw_main() {
	char key_data[2] = { 0xff, 0xff };
	char servo_cmd[4] = { 0x01, 0x01, 0, 90 };
	char kaiten_cmd[2];
	char led_cmd[100];
	weight_t weights;
	static uint8_t cnt = 0;

	while (1) {
		char *p = (char *)&weights;
		master.read(weight_addr, p, 8);

		master.read(key_addr, key_data, 2);
		pc.printf("%d, %d\n", key_data[0], key_data[1]);

		if (key_data[1] == true) {
			switch (key_data[0]) {
			case '0':
				servo_cmd[2] = 0;
				break;
			case '1':
				servo_cmd[2] = 90;
				break;
			case '2':
				servo_cmd[2] = 180;
				break;
			case '*':
				servo_cmd[3] = 10;
				break;
			case '#':
				servo_cmd[3] = 90;
				break;
			}
		}

		if (key_data[0] == '3') {
			if (key_data[1] == true) {
				kaiten_cmd[0] = 0x01;
				kaiten_cmd[1] = 50;
			} else {
				kaiten_cmd[0] = 0x01;
				kaiten_cmd[1] = 0;
			}
		}

		if (key_data[0] == '4') {
			if (key_data[1] == true) {
				kaiten_cmd[0] = 0x02;
				kaiten_cmd[1] = 1;
			} else {
				kaiten_cmd[0] = 0x02;
				kaiten_cmd[1] = 0;
			}
		}

		if (cnt == 5) {
			led_cmd[0] = cmd_fillRect;
			led_cmd[1] = 0;
			led_cmd[2] = 0;
			led_cmd[3] = 64;
			led_cmd[4] = 32;
			led_cmd[5] = 0;
			led_cmd[6] = 0;
			led_cmd[7] = 0;
			master.write(led_addr, led_cmd, 8);

			led_cmd[0] = cmd_setCursor;
			led_cmd[1] = 0;
			led_cmd[2] = 0;
			master.write(led_addr, led_cmd, 3);

			led_cmd[0] = cmd_setTextColor;
			led_cmd[1] = 0;
			led_cmd[2] = 15;
			led_cmd[3] = 0;
			master.write(led_addr, led_cmd, 4);

			led_cmd[0] = cmd_print;
			char str[50] = "0123456789\nabcdefg\nhigklmn\nABCDEFG\n";
			for (int i = 0; i < 50; i++) {
				led_cmd[1 + i] = str[i];
			}
			master.write(led_addr, led_cmd, 51);
		}
		if (cnt == 9) {


			led_cmd[0] = cmd_fillRect;
			led_cmd[1] = 0;
			led_cmd[2] = 0;
			led_cmd[3] = 64;
			led_cmd[4] = 32;
			led_cmd[5] = 0;
			led_cmd[6] = 0;
			led_cmd[7] = 0;
			master.write(led_addr, led_cmd, 8);

			led_cmd[0] = cmd_setCursor;
			led_cmd[1] = 0;
			led_cmd[2] = 0;
			master.write(led_addr, led_cmd, 3);

			led_cmd[0] = cmd_setTextColor;
			led_cmd[1] = 15;
			led_cmd[2] = 0;
			led_cmd[3] = 0;
			master.write(led_addr, led_cmd, 4);

			led_cmd[0] = cmd_print;
			char str2[50];
			sprintf(str2, "weight:\n%f kg", weights.weight);
			for (int i = 0; i < 50; i++) {
				led_cmd[1 + i] = str2[i];
			}
			master.write(led_addr, led_cmd, 51);
		}
		cnt++;
		if (cnt > 10) {
			cnt = 0;
		}

		master.write(servo_addr, servo_cmd, 4);
		master.write(kaiten_addr, kaiten_cmd, 2);
		pc.printf("%d, %d, %d, %d\n", servo_cmd[0], servo_cmd[1], servo_cmd[2],
				servo_cmd[3]);
		Thread::wait(500);
	}
}
