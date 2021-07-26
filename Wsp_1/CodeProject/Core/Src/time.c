/*
 * time.c
 *
 *  Created on: Jul 10, 2021
 *      Author: 0
 */

#include "stm32l4xx_hal.h"
#include "L506.h"
#include "time.h"
#include "string.h"

sTimer sTimer_10s ;
sTimer sTimer_1000ms;
sTimer sTimer_500ms;
sTimer sTimer_7000ms;

uint32_t time_sendServer = 9999;
uint16_t num_check = 0;

extern RTC_TimeTypeDef sTime;
extern RTC_DateTypeDef sDate;


extern RTC_TimeTypeDef sTime2;
extern RTC_DateTypeDef sDate2;
extern RTC_HandleTypeDef hrtc;

RTC_AlarmTypeDef nAlarm;

static uint8_t alarm_hour;
static uint8_t alarm_minute;
static uint8_t alarm_second;

sRtc_data time_data;
volatile uint8_t vr_count = 0;

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
  		*(p_time+0) = (sTime.Hours/10)+48;
  		*(p_time+1) = (sTime.Hours%10)+48;
  		*(p_time+2) = '-';
  		*(p_time+3) = (sTime.Minutes/10)+48;
  		*(p_time+4) = (sTime.Minutes%10)+48;
  		*(p_time+5) = '-';
	  	*(p_time+6) = (sTime.Seconds/10)+48;
	  	*(p_time+7) = (sTime.Seconds%10)+48;
	  	*(p_time+8) = ' ';
	  	*(p_time+9) = (sDate.Date/10)+48;
	  	*(p_time+10) = (sDate.Date%10)+48;
	  	*(p_time+11) = '/';
	  	*(p_time+12) = (sDate.Month/10)+48;
	  	*(p_time+13) = (sDate.Month%10)+48;
	  	*(p_time+14) = '/';
	  	*(p_time+15) = (sDate.Year/10)+48;
	  	*(p_time+16) = (sDate.Year%10)+48;
	  	*(p_time+17) = ' ';
	  	*(p_time+18) = (index/1000)+48;
		*(p_time+19) = ((index%1000)/100)+48;
		*(p_time+20) = (((index%1000)%100)/10)+48;
		*(p_time+21) = (((index%1000)%100)%10)+48;
	  	*(p_time+22) = '\r';
		*(p_time+23) = '\n';
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM2)
	{
		  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_3);
		  sTimer_10s.vr_count ++;
		  sTimer_1000ms.vr_count ++;
		  sTimer_500ms.vr_count ++;
		  sTimer_7000ms.vr_count ++;

		  if(sTimer_10s.vr_count == time_sendServer){
			  sTimer_10s.flag_timer = 1; // timer du 10s thi gui du lieu len server
			  sTimer_10s.vr_count = 0;
		  }
		  if(1000 == sTimer_1000ms.vr_count){
			  sTimer_1000ms.flag_timer = 1; // timer du 1000ms
			  sTimer_1000ms.vr_count = 0;
		  }
		  if(500 == sTimer_500ms.vr_count){
			  sTimer_500ms.flag_timer = 1; // timer du 500ms
			  sTimer_500ms.vr_count = 0;
		  }
		  if(7000 ==  sTimer_7000ms.vr_count){ // timer du 7s
			  sTimer_7000ms.flag_timer = 1;
			  sTimer_7000ms.vr_count = 0;
		  }
	}
}


void set_alarm_A(uint8_t hour,uint8_t minute,uint8_t second)
{
	nAlarm.AlarmTime.Hours = hour;;
	nAlarm.AlarmTime.Minutes = minute;
	nAlarm.AlarmTime.Seconds = second;
	nAlarm.AlarmTime.SubSeconds = 0;
	nAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	nAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
	nAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY;
	nAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
	nAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
	nAlarm.AlarmDateWeekDay = 1;
	nAlarm.Alarm = RTC_ALARM_A;
  if (HAL_RTC_SetAlarm_IT(&hrtc, &nAlarm, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
}

void get_rtc_data(void)
{
	HAL_RTC_GetTime(&hrtc,&sTime,RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc,&sDate,RTC_FORMAT_BIN);
}

void packet_rtc_data(uint8_t* output)
{
		get_rtc_data();
		convertTime(output, num_check);
}

void Set_Frequency_Send_Data(uint8_t frequency)
{
	uint32_t now_time;
	uint32_t alarm_time;
	get_rtc_data();
    time_data.second = sTime.Seconds;
    time_data.minute = sTime.Minutes;
    time_data.hour = sTime.Hours;
	now_time = time_data.hour*3600 + time_data.minute*60 + time_data.second;
	alarm_time = now_time + frequency;
	if(alarm_time > TIME_24_HOUR)
	{
		alarm_time=alarm_time - TIME_24_HOUR;
	}
	alarm_hour = alarm_time/3600;
	alarm_minute = (alarm_time - alarm_hour*3600)/60;
	alarm_second = alarm_time - alarm_hour*3600 - alarm_minute*60;
	set_alarm_A(alarm_hour, alarm_minute, alarm_second);
}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc){
	vr_count = 1;
}

