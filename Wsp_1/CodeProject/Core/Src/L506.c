/*
 * L506.c
 *
 *  Created on: Jul 4, 2021
 *      Author: 0
 */
#include "stm32l4xx_hal.h"
#include "L506.h"
#include "time.h"
#include "string.h"
#include "stdlib.h"
#include "myLib.h"
#include "stdbool.h"
#include "UartRingbuffer.h"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;

extern sTimer sTimer_10s ;
extern sTimer sTimer_100ms;
extern sTimer sTimer_1000ms;

extern uint8_t vr_count;
extern uint16_t num_check;
extern uint32_t Time_send_sv;

extern RTC_TimeTypeDef sTimedif;
extern RTC_DateTypeDef sDatedif;

uint8_t vr_flag = 0;

int8_t gsm_state = -1;
uint8_t result;
uint8_t answer;
uint8_t count_time = 0;

uint8_t flag_rev = 0;

char arrRevProcess[256]={0};

uint8_t buffer[256];
uint8_t indexBuffer;

uint8_t Rx_index = 0;  		// index cho nhận từ server
uint8_t Rx_buffer[256];		// buf cho nhận từ server

int vr_check = 0;

uint32_t time_conv = 0;

//uint8_t bufferTest[50];

uint8_t vr_test = 0;

static uint8_t arr_cmd_send[] = "AT+CIPSEND=1,";

static uint8_t serverText1[] = "gui thanh cong ban tin len server\r\n";
static uint8_t serverText2[] = "gui khong thanh cong ban tin len server\r\n";
static uint8_t cmdReceiveData[] = "gui lenh nhan data tu server\r\n";

t_uartAt arrInitialSim[]={{CHECK_CMD_AT, 				{(uint8_t*)"AT\r\n",4},												fnParseOKPacket},
					  	  {CHECK_STATUS_SIM, 			{(uint8_t*)"AT+CPIN?\r\n",10},										fnParseCPINPacket},
						  {CHECK_CMD_CSQ,	 			{(uint8_t*)"AT+CSQ\r\n",8},											fnParseCSQPacket},
						  {CHECK_STATUS_NETWORK, 		{(uint8_t*)"AT+CGREG?\r\n",11},										fnParseCGREGPacket},
						  {CMD_REPORT_NETWORK,			{(uint8_t*)"AT+CREG?\r\n",10},										fnParseCREGPacket},
						  {CHECK_ATTACHED_STATUS, 		{(uint8_t*)"AT+CGATT?\r\n",11},										fnParseCGATTPacket},
						  {CMD_CIPTIMEOUT, 				{(uint8_t*)"AT+CIPTIMEOUT=30000,20000,40000,50000\r\n",39},			fnParseOKPacket},
						  {CHECK_MODE_TCP, 				{(uint8_t*)"AT+CIPMODE=0\r\n",14},									fnParseOKPacket},
						  {CHECK_CMD_NETOPEN, 			{(uint8_t*)"AT+NETOPEN\r\n",12},									fnParseNETOPENPacket},
						  {CMD_GET_IPADDR, 				{(uint8_t*)"AT+IPADDR\r\n",11},										fnParseIPADDRPacket},
						  {CMD_CREATE_TCP, 				{(uint8_t*)"AT+CIPOPEN=1,\"TCP\",\"113.190.240.47\",7580\r\n",42},	fnCheckCreateTCPPacket},
						  {CHECK_CMD_CIPOPQUERY, 		{(uint8_t*)"AT+CIPOPQUERY=1\r\n",17},								fnParseCIPOPQUERYPacket},
						  {CMD_SEND_DATA, 				{arr_cmd_send, 13},													fnParseSendSVPacket},
						  {CMD_RECEIVE_DATA, 			{(uint8_t*)"\r\nAT+CIPRXGET=0,1\r\n",19},							fnParseOKPacket}};


void Sim_PWON(){
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
}

void Sim_PWOFF(){
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
}

void Pow_ON_OFF(){
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
	HAL_Delay(1000);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
	HAL_Delay(1000);
}

void getBuffRx(bool needWaitAtFirstTime) {
	if(needWaitAtFirstTime) {
		while(!IsDataAvailable());
	}
	while(IsDataAvailable()){
		buffer[indexBuffer] = Uart_read();
		indexBuffer ++;
		HAL_Delay(1);
	}
	// xử lý THA
}
void GSM_Init(){
	  Pow_ON_OFF();;
	  Sim_PWON();
	  HAL_Delay(2000);
	  Sim_PWOFF();
	  HAL_Delay(25000);
	  indexBuffer = 0;
	  memset(buffer, '\0', 256);
	  getBuffRx(true);
	  HAL_UART_Transmit(&huart1, buffer, strlen((char*)buffer), 1000);
}

uint8_t fnParseOKPacket(char* arrRes) { 	//kiem tra Sim
	if(memcmp((const char*)arrRes, (const char*)"\r\nOK", 4) == 0){
		return 1;
	}
    return 0;
}

uint8_t fnParseCPINPacket(char* arrRes) {	// kiem tra trang thai cua Sim
  if (memcmp(arrRes, "\r\n+CPIN: READY", 14) == 0) {
    return 1;
  }
  return 0;
}

uint8_t fnParseCSQPacket(char* arrRes) {	// kiem tra cuong do song
  if (memcmp(arrRes, "\r\n+CSQ:", 7) == 0) {
    return 1;
  }
  return 0;
}

uint8_t fnParseCGREGPacket(char* arrRes) {	// dang ki mạng
  if (memcmp(arrRes, "\r\n+CGREG:", 9) == 0) {
    return 1;
  }
  return 0;
}

uint8_t fnParseCREGPacket(char* arrRes) {	// dang ki mang
  if (memcmp(arrRes, "\r\n+CREG:", 8) == 0) {
    return 1;
  }
  return 0;
}

uint8_t fnParseCGATTPacket(char* arrRes) { // kiem tra trang thai GPRS
  if (memcmp(arrRes, "\r\n+CGATT:", 9) == 0) {
    return 1;
  }
  return 0;
}

uint8_t fnParseNETOPENPacket(char* arrRes) {
	char *ptr;
	ptr = strstr(arrRes, "NETOPEN:SUCCESS");
	if(ptr != 0){
		return 1;
	}
	return 0;
}

uint8_t fnParseIPADDRPacket(char* arrRes) {	// nhan dia chi IP
	char *ptr;
	ptr = strstr(arrRes, "IPADDR:SUCCESS");
	if(ptr != 0){
		return 1;
	}
	return 0;
}

uint8_t fnCheckCreateTCPPacket(char * arrRes) {
	char *ptr;
	ptr = strstr(arrRes, "CIPOPEN:SUCCESS");
	if(ptr != 0){
		return 1;
	}
	return 0;
}

uint8_t fnParseCIPOPQUERYPacket(char* arrRes) {	// quet link ket noi
  if (memcmp(arrRes, "\r\n+CIPOPQUERY:0", 15) == 0) {
    return 0;
  }
  return 1;
}

uint8_t fnParseSendSVPacket(char* arrRes) {	// kiem tra phan hoi gui
  if (memcmp(arrRes, "\r\n>", 3) == 0) {
    return 1;
  }
  return 0;
}

uint8_t fnCheckSendSVPacket(char* arrRes) {
	char *ptr;
	ptr = strstr(arrRes, "CIPSEND:SUCCESS");
	if(ptr != 0){
		return 1;
	}
	return 0;
}


uint8_t fnParseReceiveSVPacket(char* arrRes) {	// kiem tra phan hoi nhan
	char *ptr;
	ptr = strstr(arrRes, "CIPRXGET:SUCCESS");
	if(ptr != 0){
		return 1;
	}
	return 0;
}

uint8_t fnCheckPacket(uint8_t* packet, uint16_t len, fncProcess fnParse, uint16_t delayWaitRxMs) { // ham gui du lieu co kiem tra phan hoi
	 uint16_t countTimeout = 0;
	 indexBuffer = 0;
	 bool isFirstCall = true;
	 memset(buffer, '\0', 256);
//	 HAL_UART_Transmit(&huart3, packet, len, 1000);
	 Uart_sendArray ((char*)packet,(int)len);// Send dữ liệu
	 HAL_UART_Transmit(&huart1, packet, len, 1000);

//	 HAL_Delay(200);
	 answer = 0;
	 while(countTimeout++ <= delayWaitRxMs && answer == 0) {
		 getBuffRx(isFirstCall);
		 isFirstCall = false;
		 if (fnParse) {
			answer = fnParse(buffer);
		   }
		 HAL_Delay(1);
	 }
	HAL_UART_Transmit(&huart1, buffer, strlen((char*)buffer), 1000);

	return answer;
}


void fncSend_CommandAT(uint8_t curr_cmd, uint8_t next_cmd){
		result = fnCheckPacket(arrInitialSim[curr_cmd].strSend.packetAt, arrInitialSim[curr_cmd].strSend.length, arrInitialSim[curr_cmd].fncType, 1000);
		if(result != 0){
			gsm_state = next_cmd;
		}
		else {
			if(CHECK_CMD_CIPOPQUERY == curr_cmd){
				wait_to_reinitialTCP(10);
			}
			else {
				wait_to_reinitial(10);
			}
		}
}

void Sim_work(void){
  switch (gsm_state){
  	 case CMD_PWNON:
  		  GSM_Init();
  		  gsm_state = CHECK_CMD_AT;
  		  break;
  	 case CHECK_CMD_AT:
  		  fncSend_CommandAT(CHECK_CMD_AT, CHECK_STATUS_SIM);
  		  break;
  	 case CHECK_STATUS_SIM:
  		  fncSend_CommandAT(CHECK_STATUS_SIM, CHECK_CMD_CSQ);
  		  break;
  	 case CHECK_CMD_CSQ:
  		  fncSend_CommandAT(CHECK_CMD_CSQ, CHECK_STATUS_NETWORK);
  		  break;
  	 case CHECK_STATUS_NETWORK:
  		  fncSend_CommandAT(CHECK_STATUS_NETWORK, CMD_REPORT_NETWORK);
   		  break;
  	 case CMD_REPORT_NETWORK:
  		  fncSend_CommandAT(CMD_REPORT_NETWORK, CHECK_ATTACHED_STATUS);
    	  break;
  	 case CHECK_ATTACHED_STATUS:
  		  fncSend_CommandAT(CHECK_ATTACHED_STATUS, CMD_CIPTIMEOUT);
     	  break;
  	 case CMD_CIPTIMEOUT:
  		  fncSend_CommandAT(CMD_CIPTIMEOUT, CHECK_MODE_TCP);
      	  break;
  	 case CHECK_MODE_TCP:
  		  fncSend_CommandAT(CHECK_MODE_TCP, CHECK_CMD_NETOPEN);
       	  break;
 	 case CHECK_CMD_NETOPEN:
 		  fncSend_CommandAT(CHECK_CMD_NETOPEN, CMD_GET_IPADDR);
          break;
 	 case CMD_GET_IPADDR:
 		  fncSend_CommandAT(CMD_GET_IPADDR, CMD_CREATE_TCP);
          break;
 	 case CMD_CREATE_TCP:
 		  fncSend_CommandAT(CMD_CREATE_TCP, CHECK_CMD_CIPOPQUERY);
          break;
 	 case CHECK_CMD_CIPOPQUERY:
 		  fncSend_CommandAT(CHECK_CMD_CIPOPQUERY, CMD_SENDREC_DATA);
          break;
  	  }
}


void fncReceive_DataServer(){
	char revRtc[256];
	Rx_index = 0;
	while(IsDataAvailable()){

		Rx_buffer[Rx_index] = Uart_read();
		Rx_index ++;
		HAL_Delay(1);
	}
	if(Rx_index > 0) {
		memcpy(revRtc, Rx_buffer, sizeof(Rx_buffer));
		if(strstr(revRtc, "CIPRXGET:SUCCESS")){
			fncStringhandler(revRtc, arrRevProcess);
			vr_test = strlen(arrRevProcess);
			if(arrRevProcess[vr_test-1] == 1){
				delete_rtc();
				takeTime((uint8_t*)arrRevProcess);
				set_rtc_data();
				memset(arrRevProcess, '\0', 256);
			}
			if(arrRevProcess[vr_test-1] == 2){
				time_conv = convertBcdToDec(arrRevProcess[vr_test-2]);
				Time_send_sv  = time_conv;
				memset(arrRevProcess, '\0', 256);
			 }
		}
		if(strstr(revRtc, "SERVER DISCONNECTED")){
			gsm_state = CMD_CREATE_TCP;
		}
	}
}

void fncCmdReceiveData(uint8_t curr_cmd){
	 result = fnCheckPacket(arrInitialSim[curr_cmd].strSend.packetAt,arrInitialSim[curr_cmd].strSend.length, arrInitialSim[curr_cmd].fncType, 0);
	 if(result == 1){
		 HAL_UART_Transmit(&huart1,cmdReceiveData, strlen((char*)cmdReceiveData), 1000);
	 }
}

void fncSend_DataServer(uint8_t curr_cmd, uint8_t *arrSend, uint32_t len){
		char numArr[20]= {0};
		char fullCmdSend[20] = {0};
		char endChar[] = "\r\n";
		uint32_t lengh = 0;
		itoa(len, numArr, 10);
		strcat(numArr, endChar);
		strcpy(fullCmdSend, (const char*)arrInitialSim[curr_cmd].strSend.packetAt);
		strcat(fullCmdSend, numArr);
		lengh = arrInitialSim[curr_cmd].strSend.length + strlen(numArr);
		result = fnCheckPacket((uint8_t*)fullCmdSend, lengh, arrInitialSim[curr_cmd].fncType, 1000);
		if(result != 0){
	 		num_check ++;
	 		result = fnCheckPacket(arrSend, len, fnCheckSendSVPacket, 1000);
	 			if(result != 0){
	 				HAL_UART_Transmit(&huart1,serverText1, strlen((char*)serverText1), 1000);
	 				}
	 			else
	 				HAL_UART_Transmit(&huart1,serverText2, strlen((char*)serverText2), 1000);
	 	}

}

void Sim_SendToServer(uint8_t curr_cmd, uint8_t *data, uint8_t len, uint32_t time_send_data){
			if(CMD_SENDREC_DATA == gsm_state){
				if(vr_flag == 0){
					Set_Frequency_Send_Data(time_send_data);
					vr_flag = 1;
				}
				if(vr_count == 1){
					Set_Frequency_Send_Data(time_send_data);
					vr_count = 0;
					count_time = 0;
					fncSend_DataServer(curr_cmd, data, len);
				}
	}
}

void Packet_Rtc_SendToServer(uint8_t *data, uint8_t len, uint8_t time_send_data){
	if(1 == sTimer_1000ms.flag_timer){
		packet_rtc_data(data);
		Sim_SendToServer(CMD_SEND_DATA, data, len, time_send_data);
		sTimer_1000ms.flag_timer = 0;
	}
}


// ham cho de khoi tao lai
void wait_to_reinitial(uint8_t time){
	count_time++;
		 if(count_time == time){
		  	gsm_state = CMD_PWNON;
		  	count_time = 0;
		  }
}

// ham quay lai khoi tao TCP
void wait_to_reinitialTCP(uint8_t time){
	count_time++;
		 if(count_time == time){
		  	gsm_state = CMD_CREATE_TCP;
		  	count_time = 0;
		  }
}

int8_t getCurCmdState(){
	return gsm_state;
}
