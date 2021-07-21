/*
 * time.c
 *
 *  Created on: Jul 10, 2021
 *      Author: 0
 */

#include "stm32l4xx_hal.h"
#include "L506.h"
#include "time.h"

extern uint8_t count;
extern int8_t gsm_state;

extern RTC_TimeTypeDef sTime;
extern RTC_DateTypeDef sDate;

extern RTC_TimeTypeDef sTime2;
extern RTC_DateTypeDef sDate2;

// ham cho de khoi tao lai
void wait_to_reinitial(uint8_t time){
		count++;
		 if(count == time){
		  	gsm_state = CMD_PWNON;
		  	count = 0;
		  }
}

// ham quay lai khoi tao TCP
void wait_to_reinitialTCP(uint8_t time){
		count++;
		 if(count == time){
		  	gsm_state = CMD_CREATE_TCP;
		  	count = 0;
		  }
}

void processChar(char *strInput, uint8_t chr, char strRes[]){		// ham xu ly chuoi nhan ve
	char *ptr1, *ptr2;
	ptr1 = strchr(strInput, chr);
	ptr1 = strchr(ptr1+1,chr);
	ptr1 = strchr(ptr1+1,chr);
	ptr1 = strchr(ptr1+1,chr);
	ptr2 = strchr(ptr1+1,chr);
	memcpy(strRes, ptr1+1, ptr2-ptr1-2);
}

void takeTime(uint8_t *sv_time){
	 sTime2.Seconds = *(sv_time+0);
	 sTime2.Minutes = *(sv_time+1);
	 sTime2.Hours = *(sv_time+2);
	 sDate2.Date = *(sv_time+3);
	 sDate2.Month = *(sv_time+4);
	 sDate2.Year = *(sv_time+5);
}

void convertTime(uint8_t *p_time, uint16_t index){
  		*(p_time+0) = (sTime.Hours/16)+48;
  		*(p_time+1) = (sTime.Hours%16)+48;
  		*(p_time+2) = '-';
  		*(p_time+3) = (sTime.Minutes/16)+48;
  		*(p_time+4) = (sTime.Minutes%16)+48;
  		*(p_time+5) = '-';
	  	*(p_time+6) = (sTime.Seconds/16)+48;
	  	*(p_time+7) = (sTime.Seconds%16)+48;
	  	*(p_time+8) = ' ';
	  	*(p_time+9) = (sDate.Date/16)+48;
	  	*(p_time+10) = (sDate.Date%16)+48;
	  	*(p_time+11) = '/';
	  	*(p_time+12) = (sDate.Month/16)+48;
	  	*(p_time+13) = (sDate.Month%16)+48;
	  	*(p_time+14) = '/';
	  	*(p_time+15) = (sDate.Year/16)+48;
	  	*(p_time+16) = (sDate.Year%16)+48;
	  	*(p_time+17) = ' ';
	  	*(p_time+18) = (index/1000)+48;
		*(p_time+19) = ((index%1000)/100)+48;
		*(p_time+20) = (((index%1000)%100)/10)+48;
		*(p_time+21) = (((index%1000)%100)%10)+48;
	  	*(p_time+22) = '\r';
		*(p_time+23) = '\n';
}

uint8_t convertTimeSendSV(uint8_t vrBCD){
		uint8_t lowByte = (vrBCD & 0x0F);
		uint8_t highByte = ((vrBCD >> 4) & 0x0F);
		return 	(10*highByte + lowByte);
}
