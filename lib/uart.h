#include <stdio.h>
#include <avr/io.h>

#define BAUD 19200
#define MYUBRR F_CPU/16/BAUD-1

void uart_init(unsigned int ubrr);
