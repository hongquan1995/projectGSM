/*
 * L506.h
 *
 *  Created on: Jul 4, 2021
 *      Author: 0
 */

#ifndef INC_L506_H_
#define INC_L506_H_

#include <stdbool.h>

#define 	CMD_PWNON					-1

typedef uint8_t (*fncProcess)(char*);

typedef struct{
	uint8_t* packetAt;
	uint16_t length;
}message_AT;

typedef struct{
	uint8_t num;
	message_AT strSend;
	fncProcess fncType;
}t_uartAt;


typedef enum
{
	CHECK_CMD_AT = 0,
	CHECK_STATUS_SIM,
	CHECK_CMD_CSQ,
	CHECK_STATUS_NETWORK,
	CMD_REPORT_NETWORK,
	CHECK_ATTACHED_STATUS,
	CMD_CIPTIMEOUT,
	CHECK_MODE_TCP,
	CHECK_CMD_NETOPEN,
	CMD_GET_IPADDR,
	CMD_CREATE_TCP,
	CHECK_CMD_CIPOPQUERY,
	CMD_SEND_DATA,
	CMD_RECEIVE_DATA,
	CMD_SENDREC_DATA,
	CMD_REVPROCESS,
	CMD_TRANSRTC
}cmd_sim4G;


void Sim_PWON();
void Sim_PWOFF();
void Pow_ON_OFF();
void getBuffRx(bool needWaitAtFirstTime);
void GSM_Init();


uint8_t fnParseOKPacket(char* arrRes);
uint8_t fnParseCPINPacket(char* arrRes);
uint8_t fnParseCSQPacket(char* arrRes);
uint8_t fnParseCGREGPacket(char* arrRes);
uint8_t fnParseCREGPacket(char* arrRes);
uint8_t fnParseCGATTPacket(char* arrRes);
uint8_t fnParseNETOPENPacket(char* arrRes);
uint8_t fnParseIPADDRPacket(char* arrRes);
uint8_t fnCheckCreateTCPPacket(char * arrRes);
uint8_t fnParseCIPOPQUERYPacket(char* arrRes);
uint8_t fnParseSendSVPacket(char* arrRes);
uint8_t fnCheckSendSVPacket(char* arrRes);
uint8_t fnParseReceiveSVPacket(char* arrRes);

uint8_t fnCheckPacket(uint8_t* packet, uint16_t len, fncProcess fnParse, uint16_t delayWaitRxMs);
void fncSend_CommandAT(uint8_t curr_cmd, uint8_t next_cmd);
void Sim_work(void);
void fncReceive_DataServer();
void fncCmdReceiveData(uint8_t curr_cmd);
void fncSend_DataServer(uint8_t curr_cmd, uint8_t *arrSend, uint32_t len);
void Sim_SendToServer(uint8_t curr_cmd, uint8_t *data, uint8_t len, uint32_t time_send_data);
void Packet_Rtc_SendToServer(uint8_t *data, uint8_t len, uint8_t time_send_data);
void wait_to_reinitial(uint8_t time);
void wait_to_reinitialTCP(uint8_t time);

#endif /* INC_L506_H_ */
