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

float target_weight = 100;

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

void keypadEvent(keymat_t *key_data, weight_t *weights);

void com_main() {
	keymat_t key_data;

	weight_t weights;
	static weight_t weights_old;

	bool door_open;
	static bool door_open_old;

	static uint8_t cnt = 0;

	while (1) {
		weights = get_weight();
		key_data = get_key();
		door_open = lead_sw;

		if (weights.stable != weights_old.stable) {
			if (weights.stable == initial) {
				pc.printf("stable: %d, weight: ---\n", weights.stable);
			} else {
				pc.printf("stable: %d, weight: %f\n", weights.stable, weights.weight);
			}
		}
		weights_old = weights;

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

		keypadEvent(&key_data, &weights);

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

void checkPassword(weight_t *weights);
void unlock_success(void);
void unlock_failed(void);
void kaiten_stop();

void keypadEvent(keymat_t *key_data, weight_t *weights) {
	switch (key_data->state) {
	case push:
		pc.printf("Pressed: ");
		pc.printf("%c\n", key_data->key);

		switch (key_data->key) {
		case '*':
			checkPassword(weights);
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

void checkPassword(weight_t *weights) {
	if(!password.evaluate()){
		pc.printf("Password is incorrect\n");
		unlock_failed();
		return;
	}

	if (weights->stable == initial) {
		pc.printf("Weight is initial\n");
		unlock_failed();
		return;
	}

	if (weights->weight < target_weight) {
		pc.printf("Target weight achieved\n");
		unlock_success();
	} else {
		pc.printf("Target weight unachieved\n");
		unlock_failed();
	}
}

void unlock_success(void)
{
	cmd_servo(180, 90);
	pc.printf("Unlock success\n");
}

void unlock_failed(void)
{
	cmd_kaiten_lamp(50, true);
	timeout1.attach(queue.event(&kaiten_stop), 5);
	pc.printf("Unlock failed\n");
}

void kaiten_stop() {
	cmd_kaiten_lamp(0, false);
}
