#include "StateMachine.h"

static state_func_t state_function[4];
static state_t g_state = INIT;

static void state_trans(state_t state);

void state_init(state_func_t *func, uint8_t num) {
	memcpy(state_function, func, sizeof(state_func_t) * num);
}

void set_state(state_t state) {
	g_state = state;
}

state_t get_state(void) {
	return g_state;
}

void state_main(void) {
	state_trans(g_state);
}

static void state_trans(state_t state) {
	static state_t old_state;
	static state_t next_state;

	if (old_state != state) {
		switch (old_state) {
		case INIT:
			if (state == LOCKED) {
				next_state = LOCKED;
			} else if (state == UNLOCKED) {
				next_state = UNLOCKED;
			} else {
				next_state = INIT;
			}
			break;
		case LOCKED:
			if (state == UNLOCKED) {
				next_state = UNLOCKED;
			} else {
				next_state = LOCKED;
			}
			break;
		case UNLOCKED:
			if (state == LOCKED) {
				next_state = LOCKED;
			} else if (state == SETTING) {
				next_state = SETTING;
			} else {
				next_state = UNLOCKED;
			}
			break;
		case SETTING:
			if (state == INIT) {
				next_state = INIT;
			} else {
				next_state = SETTING;
			}
			break;

		}
		if (state_function[old_state].end_func != NULL) {
			state_function[old_state].end_func();
		}
		if (state_function[next_state].init_func != NULL) {
			state_function[next_state].init_func();
		}
	}
	old_state = next_state;
	if (state_function[next_state].do_func != NULL) {
		state_function[next_state].do_func();
	}
}

