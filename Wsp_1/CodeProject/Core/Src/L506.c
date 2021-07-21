/*
 * L506.c
 *
 *  Created on: Jul 4, 2021
 *      Author: 0
 */
#include "stm32l4xx_hal.h"
#include "L506.h"
#include "time.h"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;
extern uint8_t data;
extern uint8_t indexBuffer;

extern int8_t gsm_state;
extern int result;
extern uint16_t num;
extern uint8_t test;
extern uint8_t vr_test;

int answer;

extern char arrRevProcess[256];

extern uint8_t buffer[256];
extern t_uartAt arrInitialSim[];


uint8_t serverText1[] = "gui thanh cong ban tin len server\r\n";
uint8_t serverText2[] = "gui khong thanh cong ban tin len server\r\n";

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

void GSM_Init(){
	  Pow_ON_OFF();;
	  Sim_PWON();
	  HAL_Delay(2000);
	  Sim_PWOFF();
	  HAL_Delay(25000);
	  HAL_UART_Transmit(&huart1, buffer, strlen((char*)buffer), 1000);
}

uint8_t fnParseOKPacket(char* arrRes) { 	//kiem tra Sim
  if (!memcmp(arrRes, "\r\nOK", 4)) {
    return 1;
  }
  return 0;
}

uint8_t fnParseCPINPacket(char* arrRes) {	// kiem tra trang thai cua Sim
  if (!memcmp(arrRes, "\r\n+CPIN: READY", 14)) {
    return 1;
  }
  return 0;
}

uint8_t fnParseCSQPacket(char* arrRes) {	// kiem tra cuong do song
  if (!memcmp(arrRes, "\r\n+CSQ:", 7)) {
    return 1;
  }
  return 0;
}

uint8_t fnParseCGREGPacket(char* arrRes) {	// dang ki mạng
  if (!memcmp(arrRes, "\r\n+CGREG:", 9)) {
    return 1;
  }
  return 0;
}

uint8_t fnParseCREGPacket(char* arrRes) {	// dang ki mang
  if (!memcmp(arrRes, "\r\n+CREG:", 8)) {
    return 1;
  }
  return 0;
}

uint8_t fnParseCGATTPacket(char* arrRes) { // kiem tra trang thai GPRS
  if (!memcmp(arrRes, "\r\n+CGATT:", 9)) {
    return 1;
  }
  return 0;
}

uint8_t fnParseIPADDRPacket(char* arrRes) {	// nhan dia chi IP
  if (!memcmp(arrRes, "\r\n+IPADDR:", 10)) {
    return 1;
  }
  return 0;
}

uint8_t fnParseCIPOPQUERYPacket(char* arrRes) {	// quet link ket noi
  if (!memcmp(arrRes, "\r\n+CIPOPQUERY:0", 15)) {
    return 0;
  }
  return 1;
}

uint8_t fnParseSendSVPacket(char* arrRes) {	// kiem tra phan hoi gui
  if (!memcmp(arrRes, "\r\n>", 3)) {
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

uint8_t fnCheckPacket(uint8_t* packet, uint16_t len, fncProcess fnParse) { // ham gui du lieu co kiem tra phan hoi
	 indexBuffer = 0;
	 memset(buffer, '\0', 256);
	 HAL_UART_Transmit(&huart3, packet, len, 1000);
	 HAL_UART_Transmit(&huart1, packet, len, 1000);
	 HAL_Delay(300);
	 if (fnParse) {
	 	  answer = fnParse(buffer);
	   }
	 HAL_UART_Transmit(&huart1, buffer, strlen((char*)buffer), 1000);
	 return answer;
}

void GPRS_Ask(uint8_t cmd[], uint8_t len){		// ham gui du lieu
	 indexBuffer = 0;
	 memset(buffer, '\0', 256);
	 HAL_UART_Transmit(&huart3, cmd, len, 1000);
	 HAL_UART_Transmit(&huart1, cmd, len, 1000);
	 HAL_Delay(300);
	 HAL_UART_Transmit(&huart1, buffer, strlen((char*)buffer), 1000);
}

void fncSend_CommandAT(uint8_t curr_cmd, uint8_t next_cmd){
		result = fnCheckPacket(arrInitialSim[curr_cmd].strSend.packetAt, arrInitialSim[curr_cmd].strSend.length, arrInitialSim[curr_cmd].fncType);
		if(result != 0){
			gsm_state = next_cmd;
		}
		else
			if(CHECK_CMD_CIPOPQUERY == curr_cmd)
				wait_to_reinitialTCP(10);
			else
				wait_to_reinitial(10);
}

uint8_t fncSend_DataServer(uint8_t curr_cmd, uint8_t *arrSend, uint32_t len){
		result = fnCheckPacket(arrInitialSim[curr_cmd].strSend.packetAt,arrInitialSim[curr_cmd].strSend.length, arrInitialSim[curr_cmd].fncType);
		if(result != 0){
	 		num ++;
	 		result = fnCheckPacket(arrSend, len, fnCheckSendSVPacket);
	 			if(result != 0){
	 				HAL_UART_Transmit(&huart1,serverText1, strlen((char*)serverText1), 1000);
	 				}
	 			else
	 				HAL_UART_Transmit(&huart1,serverText2, strlen((char*)serverText2), 1000);
	 	return 1;
	 	}
	 	else{
	 		wait_to_reinitialTCP(3);
	 		return 0;
	 	}

}

void  fncReceive_DataServer(uint8_t curr_cmd){
	 char revRtc[256];
	 result = fnCheckPacket(arrInitialSim[curr_cmd].strSend.packetAt,arrInitialSim[curr_cmd].strSend.length, arrInitialSim[curr_cmd].fncType);
	 if(result == 1){
	 	memcpy(revRtc, buffer, sizeof(buffer));
	 	processChar(revRtc, '\n', arrRevProcess);
	 	vr_test = strlen(arrRevProcess);
	 	//HAL_UART_Transmit(&huart1, &vr_test, 1, 1000);
	 	result = 0;
	 	}
}
