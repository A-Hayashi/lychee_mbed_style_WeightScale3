#include "mbed.h"

typedef enum {
	start = 0, ongoing = 1, determined = 3, waiting = 4, initial = 0xff
} weight_stable;

typedef struct {
	weight_stable stable;
	float weight;
} weight_t;

typedef enum {
	release = 0, push, none,
} key_state;

typedef struct {
	char key;
	key_state state;
} keymat_t;

void cmd_servo(uint8_t angle, uint8_t speed);

void cmd_kaiten(uint8_t duty);
void cmd_lamp(bool lamp);
void cmd_kaiten_lamp(uint8_t duty, bool lamp);

weight_t get_weight(void);

keymat_t get_key(void);

void cmd_clear_line(uint8_t line);
void cmd_clear_all(void);
void cmd_set_cursor(uint8_t x, uint8_t y);
void cmd_set_line(uint8_t line);
void cmd_set_text_color(uint8_t r, uint8_t g, uint8_t b);
void cmd_print_text(char *str);
