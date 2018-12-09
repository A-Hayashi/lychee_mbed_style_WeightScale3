#ifndef PASSWORD_H
#define PASSWORD_H

#include "mbed.h"

#define MAX_PASSWORD_LENGTH 20

#define STRING_TERMINATOR '\0'

typedef uint8_t byte;

class Password {
public:
	Password(char* pass);
	
	void set(char* pass);
	bool is(char* pass);
	bool append(char character);
	void reset();
	bool evaluate();
	
	char* getPassword();
	char* getGuess();
	
	//operators
	Password &operator=(char* pass);
	bool operator==(char* pass);
	bool operator!=(char* pass);
	Password &operator<<(char character);
	
private:
	char* target;
	char guess[ MAX_PASSWORD_LENGTH ];
	byte currentIndex;
};

#endif

/*
|| @changelog
|| | 1.1 2009-06-17 - Alexander Brevig : Added assignment operator =, equality operators == != and insertion operator <<
|| | 1.0 2009-06-17 - Alexander Brevig : Initial Release
|| #
*/
