#include "mbed.h"
#include "Password.h"
#include "I2C_Comm.h"
#include "StateMachine.h"
#include "ThingSpeak.h"
#include "PrintfWrapper.h"

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

static state_func_t state_function[4] = {
	{INIT_init, INIT_do, INIT_end },
	{LOCKED_init, LOCKED_do, LOCKED_end },
	{UNLOCKED_init, UNLOCKED_do, UNLOCKED_end },
	{SETTING_init, SETTING_do, SETTING_end },
};

Password password = Password("1234");
EventQueue queue;
float target_weight = 100;

int main() {
	printf2("start\n");
	Thread com_main_task(osPriorityNormal, 500 * 1024);

	com_main_task.start(&com_main);
	Thread eventThread;
	eventThread.start(callback(&queue, &EventQueue::dispatch_forever));
	wait(osWaitForever);
	return 0;
}

static void com_main() {
	state_init(state_function, 4);
	wifi_init();
	char str[50];
	DigitalIn lead_sw(D10);

	bool door_open = lead_sw;
	if (door_open == false) {
		set_state(LOCKED);
	} else {
		set_state(UNLOCKED);
	}

	cmd_set_line(2);
	cmd_set_text_color(0, 15, 0);
	sprintf(str, "PASS:\n");
	cmd_print_text(str);

	while (1) {
		door_open = lead_sw;
		weight_t weights = get_weight();
		keymat_t key_data = get_key();
		{
			static weight_t weights_old;
			if (weights.stable != weights_old.stable) {
				cmd_set_line(1);
				cmd_set_text_color(0, 15, 0);
				if (weights.stable == initial) {
					printf2("stable: %d, weight: ---\n", weights.stable);
					sprintf(str, "NOW:---kg\n");
					cmd_print_text(str);
				} else {
					printf2("stable: %d, weight: %f\n", weights.stable,
							weights.weight);
					sprintf(str, "NOW:%3.0fkg\n", weights.weight);
					cmd_print_text(str);

					if (weights.stable == determined) {
						ChannelUpdate(weights.weight, target_weight);
						if (weights.weight < target_weight) {
							NotifyDietAchieved(weights.weight, target_weight);
						} else {
							NotifyDietNotAchieved(weights.weight, target_weight, weights.weight - target_weight);
						}
					}
				}
			}
			weights_old = weights;
		}

		{
			static bool door_open_old;
			if (door_open_old != door_open) {
				if (door_open == false) {
					cmd_lamp(false);
					printf2("door close\n");
					set_state(LOCKED);
				} else {
					cmd_lamp(true);
					printf2("door open\n");
				}
			}
			door_open_old = door_open;
		}

		if (get_state() == LOCKED) {
			keypadEvent(&key_data, &weights);
		}

		{
			int target_weight_int = target_weight;
			static int target_weight_old = 0;

			if (target_weight_int != target_weight_old) {
				cmd_set_line(0);
				cmd_set_text_color(0, 15, 0);
				sprintf(str, "GOAL:%3.0fkg\n", target_weight);
				cmd_print_text(str);

				if (target_weight_old != 0) {
					NotifyChangeTarget(target_weight_old, target_weight_int);
				}
			}
			target_weight_old = target_weight_int;
		}

		state_main();
		Thread::wait(300);
	}
}

static void keypadEvent(keymat_t *key_data, weight_t *weights) {
	char str[50];
	char *guess2;
	switch (key_data->state) {
	case push:
		printf2("Pressed: ");
		printf2("%c\n", key_data->key);

		switch (key_data->key) {
		case '*':
			guess2 = password.getGuess();
			if (guess2[0] == '0' && guess2[1] == '0') {
				target_weight = atoi(&guess2[2]);
				printf2("Weight Set: %3.1f\n", target_weight);
			} else if (guess2[0] == '0') {
				password.set(&guess2[1]);
				printf2("Password Set: %s\n", password.getPassword());
			} else {
				printf2("Guess Get: %s\n", password.getGuess());
				printf2("Password Get: %s\n", password.getPassword());
				checkPassword(weights);
			}
			password.reset();
			break;
		case '#':
			password.reset();
			break;
		default:
			password.append(key_data->key);
			break;
		}

		char *guess = password.getGuess();
		cmd_set_line(2);
		cmd_set_text_color(0, 15, 0);
		sprintf(str, "PASS:%s\n", guess);
		cmd_print_text(str);
	}

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

	cmd_set_line(3);
	cmd_set_text_color(0, 15, 0);

	switch (pattern) {
	case 0:
		printf2("Password is incorrect\n");

		sprintf(str, "INCORRECT\n");
		cmd_print_text(str);
		NotifyUnlockFailedInvalidPass();
		break;
	case 1:
		printf2("Weight is initial\n");

		sprintf(str, "INITIAL\n");
		cmd_print_text(str);
		break;
	case 2:
		printf2("Target weight unachieved\n");

		sprintf(str, "DO DIET\n");
		cmd_print_text(str);
		NotifyUnlockFailedDoDiet();
		break;
	}
	printf2("Unlock failed\n");
}

static void kaiten_stop() {
	char str[50];
	printf2("kaiten_stop\n");

	cmd_kaiten_lamp(0, false);

	cmd_set_line(3);
	cmd_set_text_color(0, 15, 0);
	sprintf(str, "LOCKED\n");
	cmd_print_text(str);
}

static void INIT_init() {
	printf2("INIT_init\n");
}

static void INIT_do() {
//	printf2("INIT_do\n");
}

static void INIT_end() {
	printf2("INIT_end\n");
}

static void LOCKED_init() {
	char str[50];
	printf2("LOCKED_init\n");

	cmd_kaiten_lamp(0, false);
	cmd_servo(150, 10);

	cmd_set_line(3);
	cmd_set_text_color(0, 15, 0);
	sprintf(str, "LOCKED\n");
	cmd_print_text(str);
	NotifyLocked();
}

static void LOCKED_do() {
//	printf2("LOCKED_do\n");
}

static void LOCKED_end() {
	printf2("LOCKED_end\n");
}

static void UNLOCKED_init() {
	char str[50];
	printf2("UNLOCKED_init\n");

	cmd_servo(70, 10);

	printf2("Target weight achieved\n");

	cmd_set_line(3);
	cmd_set_text_color(0, 15, 0);
	sprintf(str, "UNLOCKED\n");
	cmd_print_text(str);
	NotifyUnlocked();
}

static void UNLOCKED_do() {
//	printf2("UNLOCKED_do\n");
}

static void UNLOCKED_end() {
	printf2("UNLOCKED_end\n");
}

static void SETTING_init() {
	printf2("SETTING_init\n");
}

static void SETTING_do() {
//	printf2("SETTING_do\n");
}

static void SETTING_end() {
	printf2("SETTING_end\n");
}

//I2C master(I2C_SDA, I2C_SCL);
//int main()
//{
//    printf2("RUN\r\n");
//    for(int i = 0; i < 128 ; i++) {
//    	master.start();
//        if(master.write(i << 1)) printf2("0x%x ACK \r\n",i); // Send command string
//        master.stop();
//    }
//}
