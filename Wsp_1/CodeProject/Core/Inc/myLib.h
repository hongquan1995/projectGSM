/*
 * trace.h
 *
 *  Created on: Nov 16, 2020
 *      Author: 0
 */

#ifndef INC_MYLIB_H_
#define INC_MYLIB_H_

#include <stdint.h>

void my_printf(const char *fmt, ...);
void printArray(uint8_t array[],uint16_t len );
uint8_t convertBcdToDec(uint8_t vrBCD);

#endif /* INC_MYLIB_H_ */
