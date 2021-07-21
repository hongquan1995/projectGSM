/*
 * L506.h
 *
 *  Created on: Jul 4, 2021
 *      Author: 0
 */

#ifndef INC_L506_H_
#define INC_L506_H_

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
	CMD_AT = 0,
	CMD_CPIN,
	CMD_CSQ,
	CMD_CGREG,
	CMD_CREG,
	CMD_CGATT,
	CMD_CIPTIMEOUT,
	CMD_CIPMODE,
	CMD_NETOPEN,
	CMD_IPADDR,
	CMD_CIPOPEN,
	CMD_CIPOPQUERY,
	CMD_CIPSEND,
	CMD_CIPRXGET,
	CMD_REVPROCESS,
	CMD_TRANSRTC
}cmd_sim4G;


void Sim_PWON();
void Sim_PWOFF();
void Pow_ON_OFF();
void GSM_Init();

uint8_t fnParseOKPacket(char* arrRes);
uint8_t fnParseCPINPacket(char* arrRes);
uint8_t fnParseCSQPacket(char* arrRes);
uint8_t fnParseCGREGPacket(char* arrRes);
uint8_t fnParseCREGPacket(char* arrRes);
uint8_t fnParseCGATTPacket(char* arrRes);
uint8_t fnParseIPADDRPacket(char* arrRes);
uint8_t fnParseCIPOPQUERYPacket(char* arrRes);
uint8_t fnParseSendSVPacket(char* arrRes);
uint8_t fnParseReceiveSVPacket(char* arrRes);

uint8_t fnCheckPacket(uint8_t* packet, uint16_t len, fncProcess fnParse);
void GPRS_Ask(uint8_t cmd[], uint8_t len);

#endif /* INC_L506_H_ */
