#include "mbed.h"
#include "Password.h"
#include "I2C_Comm.h"
#include "StateMachine.h"

static void INIT_init();
static void INIT_do();
static void INIT_end();
static void LOCKED_init();
static void LOCKED_do();
static void LOCKED_end();
static void UNLOCKED_init();
static void UNLOCKED_do();
static void UNLOCKED_end();
static void SETTING_init();
static void SETTING_do();
static void SETTING_end();

static void com_main();
static void keypadEvent(keymat_t *key_data, weight_t *weights);
static void checkPassword(weight_t *weights);
static void unlock_failed(uint8_t pattern);
static void kaiten_stop();

static state_func_t state_function[4] = { { INIT_init, INIT_do, INIT_end }, {
		LOCKED_init, LOCKED_do, LOCKED_end }, { UNLOCKED_init, UNLOCKED_do,
		UNLOCKED_end }, { SETTING_init, SETTING_do, SETTING_end }, };

Serial pc(USBTX, USBRX);

Password password = Password("1234");
EventQueue queue;

float target_weight = 100;

int main() {
	Thread com_main_task(osPriorityNormal, 500 * 1024);

	com_main_task.start(&com_main);
	Thread eventThread;
	eventThread.start(callback(&queue, &EventQueue::dispatch_forever));
	wait(osWaitForever);
	return 0;
}

static void com_main() {
	state_init(state_function, 4);

	DigitalIn lead_sw(D10);
	bool door_open = lead_sw;

	if (door_open == false) {
		set_state(LOCKED);
	} else {
		set_state(UNLOCKED);
	}

	while (1) {
		door_open = lead_sw;
		weight_t weights = get_weight();
		keymat_t key_data = get_key();
		char str[50];
		{
			static weight_t weights_old;
			if (weights.stable != weights_old.stable) {
				cmd_clear_line(1);
				cmd_set_cursor(0, 8);
				cmd_set_text_color(0, 15, 0);
				if (weights.stable == initial) {
					pc.printf("stable: %d, weight: ---\n", weights.stable);

					sprintf(str, "NOW:---kg\n");
					cmd_print_text(str);

				} else {
					pc.printf("stable: %d, weight: %f\n", weights.stable,
							weights.weight);

					sprintf(str, "NOW:%3.0fkg\n", weights.weight);
					cmd_print_text(str);
				}
			}
			weights_old = weights;
		}

		{
			static bool door_open_old;
			if (door_open_old != door_open) {
				if (door_open == false) {
					cmd_lamp(false);
					pc.printf("door close\n");
					set_state(LOCKED);
				} else {
					cmd_lamp(true);
					pc.printf("door open\n");
				}
			}
			door_open_old = door_open;
		}

		if (get_state() == LOCKED) {
			keypadEvent(&key_data, &weights);
		}

		cmd_clear_line(0);
		cmd_set_cursor(0, 0);
		cmd_set_text_color(0, 15, 0);
		sprintf(str, "GOAL:%3.0fkg\n", target_weight);
		cmd_print_text(str);

		state_main();

		Thread::wait(300);
	}
}

static void keypadEvent(keymat_t *key_data, weight_t *weights) {
	char str[50];

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
	char *guess = password.getGuess();

	cmd_clear_line(2);
	cmd_set_cursor(0, 16);
	cmd_set_text_color(0, 15, 0);
	sprintf(str, "PASS:%s\n", guess);
	cmd_print_text(str);
}

static void checkPassword(weight_t *weights) {

	if (!password.evaluate()) {
		unlock_failed(0);
		return;
	}
	if (weights->stable == initial) {
		unlock_failed(1);
		return;
	}
	if (weights->weight < target_weight) {
		set_state(UNLOCKED);
	} else {
		unlock_failed(2);
	}
}

static void unlock_failed(uint8_t pattern) {
	static Timeout timeout1;
	char str[50];

	cmd_kaiten_lamp(50, true);
	timeout1.attach(queue.event(&kaiten_stop), 5);

	cmd_clear_line(3);
	cmd_set_cursor(0, 24);
	cmd_set_text_color(0, 15, 0);

	switch (pattern) {
	case 0:
		pc.printf("Password is incorrect\n");

		sprintf(str, "INCORRECT\n");
		cmd_print_text(str);
		break;
	case 1:
		pc.printf("Weight is initial\n");

		sprintf(str, "INITIAL\n");
		cmd_print_text(str);
		break;
	case 2:
		pc.printf("Target weight unachieved\n");

		sprintf(str, "DO DIET\n");
		cmd_print_text(str);
		break;
	}
	pc.printf("Unlock failed\n");
}

static void kaiten_stop() {
	char str[50];
	pc.printf("LOCKED_init\n");

	cmd_kaiten_lamp(0, false);

	cmd_clear_line(3);
	cmd_set_cursor(0, 24);
	cmd_set_text_color(0, 15, 0);
	sprintf(str, "LOCKED\n");
	cmd_print_text(str);
}

static void INIT_init() {
	pc.printf("INIT_init\n");
}

static void INIT_do() {
	pc.printf("INIT_do\n");
}

static void INIT_end() {
	pc.printf("INIT_end\n");
}

static void LOCKED_init() {
	char str[50];
	pc.printf("LOCKED_init\n");

	cmd_kaiten_lamp(0, false);
	cmd_servo(0, 90);

	cmd_clear_line(3);
	cmd_set_cursor(0, 24);
	cmd_set_text_color(0, 15, 0);
	sprintf(str, "LOCKED\n");
	cmd_print_text(str);
}

static void LOCKED_do() {
	pc.printf("LOCKED_do\n");
}

static void LOCKED_end() {
	pc.printf("LOCKED_end\n");
}

static void UNLOCKED_init() {
	char str[50];
	pc.printf("UNLOCKED_init\n");

	cmd_servo(180, 90);

	pc.printf("Target weight achieved\n");
	cmd_clear_line(3);
	cmd_set_cursor(0, 24);
	cmd_set_text_color(0, 15, 0);
	sprintf(str, "UNLOCKED\n");
	cmd_print_text(str);
}

static void UNLOCKED_do() {
	pc.printf("UNLOCKED_do\n");
}

static void UNLOCKED_end() {
	pc.printf("UNLOCKED_end\n");
}

static void SETTING_init() {
	pc.printf("SETTING_init\n");
}

static void SETTING_do() {
	pc.printf("SETTING_do\n");
}

static void SETTING_end() {
	pc.printf("SETTING_end\n");
}

//int main()
//{
//    pc.printf("RUN\r\n");
//    for(int i = 0; i < 128 ; i++) {
//    	master.start();
//        if(master.write(i << 1)) pc.printf("0x%x ACK \r\n",i); // Send command string
//        master.stop();
//    }
//}
