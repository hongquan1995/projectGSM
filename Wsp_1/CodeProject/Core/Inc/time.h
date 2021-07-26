/*
 * time.h
 *
 *  Created on: Jul 10, 2021
 *      Author: 0
 */

#ifndef INC_TIME_H_
#define INC_TIME_H_

#include <stdint.h>

#define TIME_24_HOUR 	86400

typedef struct{
	volatile uint8_t flag_timer;
	uint32_t vr_count;
}sTimer;

typedef struct{
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
}sRtc_data;

void processChar(char *strInput, uint8_t chr, char strRes[]);
void takeTime(uint8_t *sv_time);
void convertTime(uint8_t *p_time, uint16_t index);
uint8_t convertTimeSendSV(uint8_t vrBCD);
void set_alarm_A(uint8_t hour,uint8_t minute,uint8_t second);
void get_rtc_data(void);
void packet_rtc_data(uint8_t* output);
void Set_Frequency_Send_Data(uint8_t frequency);


#endif /* INC_TIME_H_ */
