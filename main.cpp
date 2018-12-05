#include "mbed.h"
#include "Password.h"
#include "I2C_Comm.h"

void com_main();

Thread com_main_task(osPriorityNormal, 500 * 1024);
Serial pc(USBTX, USBRX);

Password password = Password("1234");

DigitalIn lead_sw(D10);

Timeout timeout1;
EventQueue queue;

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
	com_main_task.start(&com_main);
	Thread eventThread;
	eventThread.start(callback(&queue, &EventQueue::dispatch_forever));

	wait(osWaitForever);
	return 0;
}

void keypadEvent(keymat_t *key_data);

void com_main() {
	keymat_t key_data;
	weight_t weights;
	bool door_open;
	static bool door_open_old;

	static uint8_t cnt = 0;

	while (1) {
		weights = get_weight();
		key_data = get_key();
		door_open = lead_sw;

		if (door_open_old != door_open) {
			if (door_open == false) {
				cmd_servo(0, 90);
				cmd_lamp(false);
				pc.printf("door close\n");
			} else {
				cmd_lamp(true);
				pc.printf("door open\n");
			}
		}
		door_open_old = door_open;

		keypadEvent(&key_data);

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

void checkPassword();
void kaiten_stop();

void keypadEvent(keymat_t *key_data) {
	switch (key_data->state) {
	case push:
		pc.printf("Pressed: ");
		pc.printf("%c\n", key_data->key);

		switch (key_data->key) {
		case '*':
			checkPassword();
			password.reset();
			break;
		case '#':
			password.reset();
			break;
		default:
			password.append(key_data->key);
		}
	}
}

void checkPassword() {
	if (password.evaluate()) {
		cmd_servo(180, 90);
		pc.printf("Password is correct\n");
	} else {
		cmd_kaiten_lamp(50, true);
		timeout1.attach(queue.event(&kaiten_stop), 5);
		pc.printf("Password is incorrect\n");
	}
}

void kaiten_stop() {
	cmd_kaiten_lamp(0, false);
}
