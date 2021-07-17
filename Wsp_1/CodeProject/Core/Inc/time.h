/*
 * time.h
 *
 *  Created on: Jul 10, 2021
 *      Author: 0
 */

#ifndef INC_TIME_H_
#define INC_TIME_H_

#include <stdint.h>

void wait_to_reinitial(uint8_t time);
void wait_to_reinitialTCP(uint8_t time);
void processChar(char *strInput, uint8_t chr, char strRes[]);
void takeTime(uint8_t *sv_time);
void convertTime(uint8_t *p_time, uint16_t index);

#endif /* INC_TIME_H_ */
