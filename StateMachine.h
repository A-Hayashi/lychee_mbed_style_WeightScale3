#include "mbed.h"

typedef void (*func_t)(void);

typedef struct {
	func_t init_func;
	func_t do_func;
	func_t end_func;
} state_func_t;

typedef enum {
	INIT, LOCKED, UNLOCKED, SETTING,
} state_t;

void state_init(state_func_t *func, uint8_t num);
void set_state(state_t state);
state_t get_state(void);
void state_main(void);
