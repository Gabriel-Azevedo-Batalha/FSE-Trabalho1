#include "modbus.h"

#define ADDRESS 0x01
#define REQUEST 0x23
#define SEND 0x16

int errorCount = 0;

int receiveInteger(int device){
	// Recebe os códigos
	unsigned char msg[7];
	receive(device, 3, msg);
	// Recebe Inteiro
	int buffer;
	receive(device, sizeof(int), &buffer);	
	// Recebe CRC
	short crc;
	receive(device, sizeof(short), &crc);	
	memcpy(&msg[3], &buffer, sizeof(int));

	if (crc != calcula_CRC(msg, 7)){
		printf("Erro de CRC\n");
		errorCount++;
		if (errorCount > 3){
			printf("Erro na UART\n");
			raise(SIGINT);
		}
		return -1;
	}
	errorCount = 0;
	return buffer;
}

float receiveFloat(int device){
	// Recebe os códigos
	unsigned char msg[7];
	receive(device, 3, msg);
	// Recebe Float
	float buffer;
	receive(device, sizeof(float), &buffer);	
	memcpy(&msg[3], &buffer, sizeof(float));
	// Recebe CRC
	short crc;
	receive(device, sizeof(short), &crc);	

	if (crc != calcula_CRC(msg, 7)){
		printf("Erro de CRC\n");
		errorCount++;
		if (errorCount > 3){
			printf("Erro na UART\n");
			raise(SIGINT);
		}
		return -1;
	}
	errorCount = 0;
	return buffer;

}

float requestIntTemp(int device){
	// Mensagem
	unsigned char msg[9] = {ADDRESS, REQUEST, 0xC1 , 0, 8, 4, 0, 0, 0};
	short crc = calcula_CRC(msg, 7);
	memcpy(&msg[7], &crc, sizeof(short));

	send(device, msg, 9);

	float response = receiveFloat(device);

	if (response == -1)
		response = requestIntTemp(device);
	return response;
}

float requestPotTemp(int device){
	// Mensagem
	unsigned char msg[9] = {ADDRESS, REQUEST, 0xC2 , 0, 8, 4, 0, 0, 0};
	short crc = calcula_CRC(msg, 7);
	memcpy(&msg[7], &crc, sizeof(short));

	send(device, msg, 9);

	float response = receiveFloat(device);

	if (response == -1)
		response = requestPotTemp(device);
	return response;
}

int requestUserComm(int device){
	// Mensagem
	unsigned char msg[9] = {ADDRESS, REQUEST, 0xC3, 0, 8, 4, 0, 0, 0};
	short crc = calcula_CRC(msg, 7);
	memcpy(&msg[7], &crc, sizeof(short));

	send(device, msg, 9);

	int response = receiveInteger(device);

	if (response == -1)
		response = requestUserComm(device);
	return response;
}

void sendControlInt(int device, int controlInt){
	// Mensagem
	unsigned char msg[13] = {ADDRESS, SEND, 0xD1, 0, 8, 4, 0, 0, 0, 0, 0, 0, 0};
	int tControlInt = controlInt;
	memcpy(&msg[7], &tControlInt, sizeof(int));
	short crc = calcula_CRC(msg, 11);
	memcpy(&msg[11], &crc, sizeof(short));

	send(device, msg, 13);
}

void sendRefFloat(int device, float refFloat){
	// Mensagem
	unsigned char msg[13] = {ADDRESS, SEND, 0xD2, 0, 8, 4, 0, 0, 0, 0, 0, 0, 0};
	float tRefFloat = refFloat;
	memcpy(&msg[7], &tRefFloat, sizeof(float));
	short crc = calcula_CRC(msg, 11);
	memcpy(&msg[11], &crc, sizeof(short));

	send(device, msg, 13);
}

int sendSisState(int device, char state){
	// Mensagem
	unsigned char msg[10] = {ADDRESS, SEND, 0xD3, 0, 8, 4, 0, 0, 0, 0};
	char tState = state;
	memcpy(&msg[7], &tState, sizeof(char));
	short crc = calcula_CRC(msg, 8);
	memcpy(&msg[8], &crc, sizeof(short));

	send(device, msg, 10);

	int response = receiveInteger(device);

	if (response == -1)
		response = sendSisState(device, state);
	return response;
}

int sendControlMode(int device, char mode){
	// Mensagem
	unsigned char msg[10] = {ADDRESS, SEND, 0xD4, 0, 8, 4, 0, 0, 0, 0};
	char tMode = mode;
	memcpy(&msg[7], &tMode, sizeof(char));
	short crc = calcula_CRC(msg, 8);
	memcpy(&msg[8], &crc, sizeof(short));

	send(device, msg, 10);

	int response = receiveInteger(device);

	if (response == -1)
		response = sendControlMode(device, mode);
	return response;
}
