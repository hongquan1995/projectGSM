/*
 * L506.c
 *
 *  Created on: Jul 4, 2021
 *      Author: 0
 */
#include "stm32l4xx_hal.h"
#include "L506.h"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;
extern uint8_t data;
extern uint8_t indexBuffer;

int answer;

extern uint8_t buffer[256];
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

uint8_t fnParseReceiveSVPacket(char* arrRes) {	// kiem tra phan hoi nhan
  if (!memcmp(arrRes, "\r\nOK\r\n\r\n+CIPRXGET:", 18)) {
    return 1;
  }
  return 0;
}

uint8_t fnCheckPacket(uint8_t* packet, uint16_t len, fncProcess fnParse) { // ham gui du lieu co kiem tra phan hoi
	 indexBuffer = 0;
	 memset(buffer, '\0', 256);
	 HAL_UART_Transmit(&huart3, packet, len, 1000);
	 HAL_UART_Transmit(&huart1, packet, len, 1000);
	 HAL_Delay(500);
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
	 HAL_Delay(500);
	 HAL_UART_Transmit(&huart1, buffer, strlen((char*)buffer), 1000);
}

