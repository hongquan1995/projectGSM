/*
 * trace.c
 *
 *  Created on: June 21, 2021
 *      Author: quandh
 */
#include "stm32l4xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

extern UART_HandleTypeDef huart1;

void vprint(const char *fmt, va_list argp)
{
    char string[200];
    if(0 < vsprintf(string,fmt,argp)) // build string
    {
        HAL_UART_Transmit(&huart1, (uint8_t*)string, strlen(string), 0xffffff); // send message via UART
    }
}

void my_printf(const char *fmt, ...) // custom printf() function
{
    va_list argp;
    va_start(argp, fmt);
    vprint(fmt, argp);
    va_end(argp);
}

void printArray(uint8_t array[],uint16_t len ) {
	if (0 == len) {
		my_printf("");
		return;
	}

	uint8_t loop = 0;
	for (loop = 0; loop < len; loop++) {
		my_printf("%02X ", array[loop]);
	}
	my_printf("\r\n");
}

uint8_t convertBcdToDec(uint8_t vrBCD){
		uint8_t lowByte = (vrBCD & 0x0F);
		uint8_t highByte = ((vrBCD >> 4) & 0x0F);
		return 	(10*highByte + lowByte);
}
